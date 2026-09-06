#include "GameModes/LiminalGameMode.h"

#include "Data/LiminalGameInstance.h"
#include "Data/QuotaManager.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Player/LiminalSpectatorPawn.h"
#include "Player/ScavengerCharacter.h"
#include "ProcGen/LiminalLevelGenerator.h"
#include "AI/LiminalEntityDirector.h"
#include "Events/LiminalEventSubsystem.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "UI/LiminalScavengerHUD.h"
#include "Audio/LiminalAudioSubsystem.h"

#include "GameModes/LiminalGameState.h"

bool ALiminalGameMode::IsBlackoutActive() const
{
	if (ALiminalGameState* GS = GetGameState<ALiminalGameState>())
	{
		return GS->bIsBlackoutActive;
	}
	return false;
}

ALiminalGameMode::ALiminalGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AScavengerCharacter::StaticClass();
	SpectatorPawnClass = ALiminalSpectatorPawn::StaticClass();
	HUDClass = ALiminalScavengerHUD::StaticClass();
	GameStateClass = ALiminalGameState::StaticClass();
}

void ALiminalGameMode::BeginPlay()
{
	Super::BeginPlay();

	MatchState = EExtractionMatchState::InMission;
	UsedPlayerStarts.Empty();
	RealityCollapseTimer = TotalMissionDuration;
	StateTransitionTimer = 0.0f;
	bTransitionPending = false;

	UWorld* World = GetWorld();
	if (World && HasAuthority())
	{
		bool bHasGenerator = false;
		for (TActorIterator<ALiminalLevelGenerator> It(World); It; ++It)
		{
			bHasGenerator = true;
			break;
		}

		if (!bHasGenerator)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ALiminalLevelGenerator* Gen = World->SpawnActor<ALiminalLevelGenerator>(
				ALiminalLevelGenerator::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
			if (Gen)
			{
				if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
				{
					Gen->SetBiome(GI->GetSelectedBiome());
				}
				else
				{
					Gen->Generate(FMath::Rand());
				}
			}
		}

		if (ULiminalAudioSubsystem* AudioSub = World->GetSubsystem<ULiminalAudioSubsystem>())
		{
			if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
			{
				AudioSub->SetCurrentBiome(GI->GetSelectedBiome());
			}
		}
	}
}

AActor* ALiminalGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* Start = *It;
			if (Start && !UsedPlayerStarts.Contains(Start))
			{
				UsedPlayerStarts.Add(Start);
				return Start;
			}
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

int32 ALiminalGameMode::GetAlivePlayerCount() const
{
	int32 Count = 0;
	UWorld* World = GetWorld();
	if (!World)
	{
		return Count;
	}

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		if (*It && !It->IsDead())
		{
			++Count;
		}
	}

	return Count;
}

void ALiminalGameMode::OnPlayerDied(AScavengerCharacter* DeadCharacter)
{
	if (!DeadCharacter)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(DeadCharacter->GetController());

	UWorld* World = GetWorld();
	if (PC && World && SpectatorPawnClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FTransform SpawnTransform = DeadCharacter->GetActorTransform();
		ALiminalSpectatorPawn* Spectator = World->SpawnActor<ALiminalSpectatorPawn>(
			SpectatorPawnClass, SpawnTransform, Params);

		if (Spectator)
		{
			PC->UnPossess();
			PC->Possess(Spectator);
		}
	}

	CheckGameOverCondition();
}

void ALiminalGameMode::CheckGameOverCondition()
{
	if (MatchState != EExtractionMatchState::InMission)
	{
		return;
	}

	if (GetAlivePlayerCount() == 0)
	{
		HandleSquadWiped();
	}
}

void ALiminalGameMode::HandleSquadWiped()
{
	if (MatchState == EExtractionMatchState::SquadWiped || MatchState == EExtractionMatchState::MissionSuccess)
	{
		return;
	}

	MatchState = EExtractionMatchState::SquadWiped;
	StateTransitionTimer = 4.5f;
	bTransitionPending = true;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
			TEXT("MISSION ECHOUEE - Toute l'escouade a succombe dans le Liminal. Rapatriement au campement de fortune..."));
	}

	if (UQuotaManager* Quota = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuotaManager>() : nullptr)
	{
		Quota->ApplyQuotaFailure();
	}
}

void ALiminalGameMode::TriggerExtraction(AScavengerCharacter* Extractor)
{
	if (MatchState == EExtractionMatchState::MissionSuccess || MatchState == EExtractionMatchState::SquadWiped || MatchState == EExtractionMatchState::ExtractionPending)
	{
		return;
	}

	MatchState = EExtractionMatchState::ExtractionPending;
	StateTransitionTimer = 3.5f;
	bTransitionPending = true;

	int32 TotalDeliveredCredits = 0;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
		{
			if (AScavengerCharacter* Scavenger = *It)
			{
				if (!Scavenger->IsDead())
				{
					TotalDeliveredCredits += Scavenger->GetCarriedCredits();
					Scavenger->DeliverCarriedLoot();
				}
			}
		}
	}

	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
	{
		GI->AddCredits(TotalDeliveredCredits);

		const uint8 NextBiomeIndex = static_cast<uint8>(GI->GetSelectedBiome()) + 1;
		if (NextBiomeIndex < 11)
		{
			GI->UnlockBiome(static_cast<ELevelBiome>(NextBiomeIndex));
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green,
			FString::Printf(TEXT("EXTRACTION REUSSIE ! Butin total livre : %d cr. Rapatriement au Hub Base Alpha..."), TotalDeliveredCredits));
	}
}

void ALiminalGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	// Mise a jour du Director de tension et du sous-systeme d'evenements
	if (UWorld* World = GetWorld())
	{
		if (ULiminalEntityDirector* Director = World->GetSubsystem<ULiminalEntityDirector>())
		{
			Director->UpdateDirector(DeltaSeconds);
		}
		if (ULiminalEventSubsystem* EventSys = World->GetSubsystem<ULiminalEventSubsystem>())
		{
			EventSys->UpdateEvents(DeltaSeconds);
		}
	}

	// 1. Compte a rebours de stabilite dimensionnelle
	if (MatchState == EExtractionMatchState::InMission)
	{
		const float PrevTimer = RealityCollapseTimer;
		RealityCollapseTimer -= DeltaSeconds;

		if (PrevTimer > 60.0f && RealityCollapseTimer <= 60.0f)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange,
					TEXT("[ALERTE M.E.G.] EFFONDREMENT DE REALITE DANS 60 SECONDES ! GAGNEZ LA ZONE D'EXTRACTION !"));
			}
		}
		else if (PrevTimer > 15.0f && RealityCollapseTimer <= 15.0f)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red,
					TEXT("[ALERTE CRITIQUE] EFFONDREMENT IMMINENT DANS 15 SECONDES !"));
			}
		}
		else if (RealityCollapseTimer <= 0.0f)
		{
			RealityCollapseTimer = 0.0f;

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red,
					TEXT("[EFFONDREMENT TOTAL] La stabilite de l'etage est aneantie. La realite s'est disloquee."));
			}

			if (UWorld* World = GetWorld())
			{
				for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
				{
					if (AScavengerCharacter* Scav = *It)
					{
						if (!Scav->IsDead())
						{
							Scav->Die(nullptr);
						}
					}
				}
			}

			HandleSquadWiped();
		}

		// 2. Gestion des pannes de courant "Lights Out" (style Escape the Backrooms)
		if (IsBlackoutActive())
		{
			BlackoutTimer -= DeltaSeconds;
			if (BlackoutTimer <= 0.0f)
			{
				RestorePower();
			}
		}
		else
		{
			NextRandomBlackoutTime -= DeltaSeconds;
			if (NextRandomBlackoutTime <= 0.0f)
			{
				TriggerBlackout(45.0f);
				NextRandomBlackoutTime = FMath::FRandRange(140.0f, 220.0f);
			}
		}
	}

	// 3. Traitement des transitions differees vers le Hub
	if (bTransitionPending)
	{
		StateTransitionTimer -= DeltaSeconds;
		if (StateTransitionTimer <= 0.0f)
		{
			bTransitionPending = false;
			if (MatchState == EExtractionMatchState::ExtractionPending)
			{
				MatchState = EExtractionMatchState::MissionSuccess;
			}

			if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
			{
				GI->ReturnToHub();
			}
		}
	}
}

void ALiminalGameMode::TriggerBlackout(float DurationSeconds)
{
	if (ALiminalGameState* GS = GetGameState<ALiminalGameState>())
	{
		if (GS->bIsBlackoutActive)
		{
			return;
		}
		
		GS->bIsBlackoutActive = true;
		GS->OnRep_BlackoutActive(); // Execute locally on server
	}

	BlackoutTimer = DurationSeconds;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow,
			TEXT("[ALERTE SECTEUR] COUPURE DE COURANT MAJEURE ! Eclairage principal hors service. Utilisez vos lampes !"));
	}
}

void ALiminalGameMode::RestorePower()
{
	if (ALiminalGameState* GS = GetGameState<ALiminalGameState>())
	{
		if (!GS->bIsBlackoutActive)
		{
			return;
		}
		
		GS->bIsBlackoutActive = false;
		GS->OnRep_BlackoutActive(); // Execute locally on server
	}

	BlackoutTimer = 0.0f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Green,
			TEXT("[ALERTE SECTEUR] ALIMENTATION RESTAUREE ! Le reseau electrique de l'etage est retabli."));
	}
}
