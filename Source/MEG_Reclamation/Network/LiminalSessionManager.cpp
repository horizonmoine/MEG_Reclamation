#include "Network/LiminalSessionManager.h"

#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Player/ScavengerCharacter.h"

void ULiminalSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveStasisRecords.Empty();
}

void ULiminalSessionManager::Deinitialize()
{
	ActiveStasisRecords.Empty();
	Super::Deinitialize();
}

FString ULiminalSessionManager::GetUniquePlayerIdFromController(AController* Controller) const
{
	if (!Controller)
	{
		return FString();
	}

	if (APlayerState* PS = Controller->PlayerState)
	{
		if (PS->GetPlayerId() > 0)
		{
			return FString::Printf(TEXT("PlayerStateId_%d"), PS->GetPlayerId());
		}

		const FString PlayerName = PS->GetPlayerName();
		if (!PlayerName.IsEmpty())
		{
			return PlayerName;
		}
	}

	return Controller->GetName();
}

bool ULiminalSessionManager::RegisterPlayerDisconnect(AController* ExitingController)
{
	if (!ExitingController)
	{
		return false;
	}

	const FString PlayerId = GetUniquePlayerIdFromController(ExitingController);
	if (PlayerId.IsEmpty())
	{
		return false;
	}

	AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(ExitingController->GetPawn());
	if (!Scavenger)
	{
		return false;
	}

	Scavenger->DropCarriedLootOnGround();

	FPlayerStasisRecord Record;
	Record.PlayerUniqueId = PlayerId;
	Record.PlayerName = ExitingController->PlayerState ? ExitingController->PlayerState->GetPlayerName() : TEXT("M.E.G. Operative");
	Record.Health = Scavenger->GetHealthPercent() * 100.0f;
	Record.Sanity = Scavenger->GetSanityPercent() * 100.0f;
	Record.CarriedCredits = Scavenger->GetCarriedCredits();
	Record.SavedLocation = Scavenger->GetActorLocation();
	Record.SavedRotation = Scavenger->GetActorRotation();
	Record.CarriedTags = Scavenger->Tags;
	Record.DisconnectTimestamp = FPlatformTime::Seconds();
	Record.StasisPawn = Scavenger;

	ActiveStasisRecords.Add(PlayerId, Record);

	// Congeler le mouvement en attendant reconnexion ou extraction
	Scavenger->SetActorEnableCollision(true);

	OnPlayerEnteredStasis.Broadcast(PlayerId, Record.SavedLocation);
	return true;
}

bool ULiminalSessionManager::TryRestorePlayer(AController* JoiningController)
{
	if (!JoiningController)
	{
		return false;
	}

	const FString PlayerId = GetUniquePlayerIdFromController(JoiningController);
	if (PlayerId.IsEmpty() || !ActiveStasisRecords.Contains(PlayerId))
	{
		return false;
	}

	const FPlayerStasisRecord Record = ActiveStasisRecords[PlayerId];

	AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(JoiningController->GetPawn());
	if (!Scavenger)
	{
		return false;
	}

	// Restaurer la position de jeu
	Scavenger->SetActorLocationAndRotation(Record.SavedLocation, Record.SavedRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Restaurer les credits et la sante en valeur absolue
	Scavenger->SetCarriedCredits(Record.CarriedCredits);
	Scavenger->AuthSetHealthAndSanity(Record.Health, Record.Sanity);

	for (const FName& Tag : Record.CarriedTags)
	{
		Scavenger->Tags.AddUnique(Tag);
	}

	ActiveStasisRecords.Remove(PlayerId);
	OnPlayerRestoredFromStasis.Broadcast(PlayerId, Scavenger);

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
