#include "Network/LiminalSessionManager.h"

#include "Engine/NetConnection.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "MEG_ReclamationPlayerController.h"
#include "Player/ScavengerCharacter.h"

void ULiminalSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveStasisRecords.Empty();
	AuthenticatedControllerIds.Empty();
}

void ULiminalSessionManager::Deinitialize()
{
	ActiveStasisRecords.Empty();
	AuthenticatedControllerIds.Empty();
	Super::Deinitialize();
}

void ULiminalSessionManager::RegisterAuthenticatedPlayerId(const AController* Controller, const FString& InPlayerId)
{
	if (!Controller || InPlayerId.IsEmpty())
	{
		return;
	}

	AuthenticatedControllerIds.Add(Controller, InPlayerId);
}

FString ULiminalSessionManager::GetAuthenticatedPlayerId(const AController* Controller) const
{
	if (!Controller)
	{
		return FString();
	}

	// 1. Identite persistante authentifiee de l'Online Subsystem (SteamID, EOS Product User ID, etc.)
	if (const APlayerState* PS = Controller->PlayerState)
	{
		const FUniqueNetIdRepl& UniqueId = PS->GetUniqueId();
		if (UniqueId.IsValid() && UniqueId.GetUniqueNetId().IsValid())
		{
			const FString NetIdStr = UniqueId.ToString();
			if (!NetIdStr.IsEmpty())
			{
				return FString::Printf(TEXT("NetId_%s"), *NetIdStr);
			}
		}
	}

	// 2. Identifiant reseau authentifie de la connexion active
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (const UNetConnection* NetConn = PC->GetNetConnection())
		{
			if (NetConn->PlayerId.IsValid() && !NetConn->PlayerId.ToString().IsEmpty())
			{
				return FString::Printf(TEXT("NetId_%s"), *NetConn->PlayerId.ToString());
			}
		}

		// 3. Jeton de session authentifie stocke sur le PlayerController MEG
		if (const AMEG_ReclamationPlayerController* MegPC = Cast<AMEG_ReclamationPlayerController>(PC))
		{
			if (!MegPC->GetPersistentPlayerId().IsEmpty())
			{
				return FString::Printf(TEXT("AuthSession_%s"), *MegPC->GetPersistentPlayerId());
			}
		}
	}

	// 4. Mappage explicite enregistre (mock de test ou service d'authentification dedie)
	if (const FString* RegisteredId = AuthenticatedControllerIds.Find(Controller))
	{
		if (!RegisteredId->IsEmpty())
		{
			return *RegisteredId;
		}
	}

	// Rejet ferme : interdiction formelle de s'appuyer sur PlayerState.GetPlayerId()
	// (compteur de session volatil) ou PlayerName (pseudo non unique, vulnérable aux homonymes).
	return FString();
}

bool ULiminalSessionManager::IsStasisExpired(const FString& PlayerUniqueId) const
{
	if (const FPlayerStasisRecord* Record = ActiveStasisRecords.Find(PlayerUniqueId))
	{
		const double Elapsed = FPlatformTime::Seconds() - Record->DisconnectTimestamp;
		return (StasisExpirationSeconds > 0.0f && Elapsed >= static_cast<double>(StasisExpirationSeconds));
	}
	return false;
}

void ULiminalSessionManager::HandleStasisExpiration(const FPlayerStasisRecord& Record)
{
	if (AScavengerCharacter* StasisPawn = Record.StasisPawn.Get())
	{
		if (!StasisPawn->IsDead())
		{
			// Delai de stase expire sans retour de l'agent :
			// Sortie de stase, depot physique du butin pour l'escouade et declaration de deces.
			StasisPawn->ExitStasis();
			StasisPawn->DropCarriedLootOnGround();
			StasisPawn->Die(nullptr);
		}
	}
}

bool ULiminalSessionManager::RegisterPlayerDisconnect(AController* ExitingController)
{
	if (!ExitingController)
	{
		return false;
	}

	const FString PlayerId = GetAuthenticatedPlayerId(ExitingController);
	if (PlayerId.IsEmpty())
	{
		return false;
	}

	AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(ExitingController->GetPawn());
	if (!Scavenger || Scavenger->IsDead())
	{
		return false;
	}

	// La stase gele l'agent dans le monde sans ejecter ses ressources au sol.
	// Ejecter les ressources lors de la mise en stase creerait une faille de duplication majeure.
	Scavenger->EnterStasis();

	FPlayerStasisRecord Record;
	Record.PlayerUniqueId = PlayerId;
	Record.PlayerName = ExitingController->PlayerState ? ExitingController->PlayerState->GetPlayerName() : TEXT("M.E.G. Operative");
	Record.Health = Scavenger->GetCurrentHealth();
	Record.Sanity = Scavenger->GetCurrentSanity();
	Record.CarriedCredits = Scavenger->GetCarriedCredits();
	Record.CarriedWeightKg = Scavenger->GetCurrentInventoryWeightKg();
	Record.SavedLocation = Scavenger->GetActorLocation();
	Record.SavedRotation = Scavenger->GetActorRotation();
	Record.CarriedTags = Scavenger->Tags;
	Record.DisconnectTimestamp = FPlatformTime::Seconds();
	Record.StasisPawn = Scavenger;

	ActiveStasisRecords.Add(PlayerId, Record);

	OnPlayerEnteredStasis.Broadcast(PlayerId, Record.SavedLocation);
	return true;
}

bool ULiminalSessionManager::TryRestorePlayer(AController* JoiningController)
{
	if (!JoiningController)
	{
		return false;
	}

	const FString PlayerId = GetAuthenticatedPlayerId(JoiningController);
	if (PlayerId.IsEmpty() || !ActiveStasisRecords.Contains(PlayerId))
	{
		return false;
	}

	const FPlayerStasisRecord Record = ActiveStasisRecords[PlayerId];

	// Verifier si le delai de reconnexion en stase a expire
	const double Elapsed = FPlatformTime::Seconds() - Record.DisconnectTimestamp;
	if (StasisExpirationSeconds > 0.0f && Elapsed >= static_cast<double>(StasisExpirationSeconds))
	{
		HandleStasisExpiration(Record);
		ActiveStasisRecords.Remove(PlayerId);
		return false;
	}

	AScavengerCharacter* StasisPawn = Record.StasisPawn.Get();
	if (!StasisPawn || StasisPawn->IsDead())
	{
		// Corps detruit ou elimine pendant la stase : reprise impossible
		ActiveStasisRecords.Remove(PlayerId);
		return false;
	}

	// Reprise de possession du pawn en stase : detruire le pawn temporaire genere au login si distinct
	APawn* CurrentPawn = JoiningController->GetPawn();
	if (CurrentPawn && CurrentPawn != StasisPawn)
	{
		JoiningController->UnPossess();
		CurrentPawn->Destroy();
	}

	JoiningController->Possess(StasisPawn);

	// Sortie de stase et retablissement absolu sans duplication
	StasisPawn->ExitStasis();
	StasisPawn->SetActorLocationAndRotation(Record.SavedLocation, Record.SavedRotation, false, nullptr, ETeleportType::TeleportPhysics);
	StasisPawn->AuthSetHealthAndSanity(Record.Health, Record.Sanity);
	StasisPawn->SetCarriedCredits(Record.CarriedCredits);
	StasisPawn->AuthSetInventoryWeight(Record.CarriedWeightKg);

	for (const FName& Tag : Record.CarriedTags)
	{
		StasisPawn->Tags.AddUnique(Tag);
	}

	ActiveStasisRecords.Remove(PlayerId);
	OnPlayerRestoredFromStasis.Broadcast(PlayerId, StasisPawn);

	return true;
}

bool ULiminalSessionManager::HasStasisRecord(const FString& PlayerUniqueId) const
{
	return ActiveStasisRecords.Contains(PlayerUniqueId);
}

bool ULiminalSessionManager::ValidatePlayerMovement(APawn* InPawn, const FVector& OldLocation, const FVector& NewLocation, float DeltaTime)
{
	if (!InPawn || DeltaTime <= 0.001f)
	{
		return true;
	}

	const float Distance = FVector::Dist(OldLocation, NewLocation);
	const float CalculatedSpeed = Distance / DeltaTime;

	if (CalculatedSpeed > MaxAllowedPawnSpeed)
	{
		// Deplacement anormal detecte (teleport non autorise ou speedhack)
		return false;
	}

	return true;
}

void ULiminalSessionManager::TriggerHostMigration(const FString& NewHostAddress)
{
	OnHostMigrationTriggered.Broadcast(NewHostAddress);
}
