#include "AI/LiminalEntity_Hound.h"
#include "AI/LiminalAIController.h"
#include "Player/ScavengerCharacter.h"
#include "Audio/LiminalProximityVoiceComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tools/AudioDecoyTool.h"
#include "UObject/ConstructorHelpers.h"

ALiminalEntity_Hound::ALiminalEntity_Hound()
{
	PrimaryActorTick.bCanEverTick = true;
	MonsterType = EMonsterType::Hound;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Hound.BT_Hound"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelFinder(
		TEXT("/Game/Characters/Bestiary/Hound/SK_Hound.SK_Hound"));
	if (SkelFinder.Succeeded())
	{
		DefaultSkeletalMesh = SkelFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClassFinder(
		TEXT("/Game/Characters/Bestiary/Hound/ABP_Hound.ABP_Hound_C"));
	if (AnimClassFinder.Succeeded())
	{
		DefaultAnimClass = AnimClassFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> MontageFinder(
		TEXT("/Game/Characters/Bestiary/Hound/AM_Hound_Attack.AM_Hound_Attack"));
	if (MontageFinder.Succeeded())
	{
		AttackMontage = MontageFinder.Object;
	}

	MaxHealth = 120.0f;
	CurrentHealth = 120.0f;
	AttackDamage = 40.0f;
	AttackRange = 150.0f;
	DefaultAttackSocket = FName(TEXT("AttackSocket"));

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalStalkSpeed;
	}
}

void ALiminalEntity_Hound::BeginPlay()
{
	Super::BeginPlay();

	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ALiminalEntity_Hound::OnPerceptionUpdated);
	}
}

void ALiminalEntity_Hound::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && !IsStunned() && !IsCalmed())
	{
		UpdateAcousticTracking(DeltaSeconds);
	}
}

void ALiminalEntity_Hound::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	const float Strength = Stimulus.Strength > 0.0f ? Stimulus.Strength : 1.0f;
	RegisterAcousticStimulus(Stimulus.StimulusLocation, Strength, Actor, Stimulus.Tag);
}

void ALiminalEntity_Hound::RegisterAcousticStimulus(
	const FVector& StimulusLocation,
	float Loudness,
	AActor* SourceActor,
	FName Tag)
{
	UWorld* World = GetWorld();
	if (!World || Loudness <= 0.0f)
	{
		LastPerceivedLoudness = 0.0f;
		return;
	}

	const FVector HoundHeadLoc = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
	const float Dist = FVector::Dist(HoundHeadLoc, StimulusLocation);

	// Distant micro-noises beyond MaxHearingRange do not trigger charge
	if (Dist > MaxHearingRange)
	{
		LastPerceivedLoudness = 0.0f;
		return;
	}

	const float DistanceFactor = FMath::Clamp(1.0f - (Dist / MaxHearingRange), 0.0f, 1.0f);

	// Occlusion line trace against walls and doors
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HoundAcousticOcclusion), false, this);
	if (SourceActor)
	{
		QueryParams.AddIgnoredActor(SourceActor);
	}

	FHitResult HitResult;
	const bool bOccluded = World->LineTraceSingleByChannel(
		HitResult, HoundHeadLoc, StimulusLocation, ECC_Visibility, QueryParams);

	const float OcclusionMultiplier = bOccluded ? OcclusionFactor : 1.0f;
	const float Perceived = Loudness * DistanceFactor * OcclusionMultiplier;
	LastPerceivedLoudness = Perceived;

	FHoundAcousticStimulus Stim;
	Stim.Location = StimulusLocation;
	Stim.SourceLoudness = Loudness;
	Stim.PerceivedIntensity = Perceived;
	Stim.Timestamp = World->GetTimeSeconds();
	Stim.Instigator = SourceActor;
	Stim.bIsOccluded = bOccluded;
	Stim.StimulusTag = Tag;

	StimulusMemory.Add(Stim);
}

void ALiminalEntity_Hound::UpdateAcousticStimuliFromEnvironment()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector HoundLoc = GetActorLocation();

	// 1. Scan active players for locomotion and vocal audio
	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Player = *It;
		if (!Player || Player->IsDead() || Player->IsDowned() || Player->IsHiddenInSpot())
		{
			continue;
		}

		float EmissionLoudness = 0.0f;
		const float PlayerSpeed = Player->GetVelocity().Size();

		if (PlayerSpeed > 450.0f)
		{
			// Sprinting: loud physical footfalls
			EmissionLoudness = 0.85f;
		}
		else if (PlayerSpeed > 180.0f)
		{
			// Jogging/walking
			EmissionLoudness = 0.45f;
		}
		else if (PlayerSpeed > 15.0f)
		{
			// Crouching / stealth shuffle: very quiet
			EmissionLoudness = 0.10f;
		}

		if (ULiminalProximityVoiceComponent* VoiceComp = Player->GetProximityVoice())
		{
			if (VoiceComp->IsSpeaking())
			{
				EmissionLoudness = FMath::Max(EmissionLoudness, 0.80f);
			}
		}

		if (EmissionLoudness > 0.02f)
		{
			RegisterAcousticStimulus(Player->GetActorLocation(), EmissionLoudness, Player, FName(TEXT("PlayerLocomotion")));
		}
	}

	// 2. Scan audio decoy tools attracting the Hound to their exact physical position
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (Candidate && (Candidate->ActorHasTag(FName(TEXT("AudioDecoy"))) || Candidate->IsA<AAudioDecoyTool>()))
		{
			RegisterAcousticStimulus(Candidate->GetActorLocation(), 1.5f, Candidate, FName(TEXT("AudioDecoy")));
		}
	}
}

void ALiminalEntity_Hound::SelectBestStimulus(float CurrentTime)
{
	// Prune expired stimuli past memory duration and stale/destroyed actors
	StimulusMemory.RemoveAll([CurrentTime, this](const FHoundAcousticStimulus& S)
	{
		const bool bExpired = (CurrentTime - S.Timestamp) > StimulusMemoryDuration;
		const bool bStaleActor = S.Instigator.IsStale();
		return bExpired || bStaleActor;
	});

	if (CurrentTargetActor.IsStale() || !CurrentTargetActor.IsValid())
	{
		CurrentTargetActor = nullptr;
	}

	if (StimulusMemory.Num() == 0)
	{
		CurrentBestStimulus = FHoundAcousticStimulus();
		CurrentAcousticRMS = 0.0f;
		return;
	}

	FHoundAcousticStimulus BestCandidate;
	BestCandidate.PerceivedIntensity = -1.0f;

	for (const FHoundAcousticStimulus& S : StimulusMemory)
	{
		if (S.PerceivedIntensity > BestCandidate.PerceivedIntensity)
		{
			BestCandidate = S;
		}
	}

	// Target stability with hysteresis: two players at similar distances do not oscillate the target every frame
	if (CurrentTargetActor.IsValid() && BestCandidate.Instigator.IsValid() && BestCandidate.Instigator != CurrentTargetActor)
	{
		if (BestCandidate.PerceivedIntensity < CurrentBestStimulus.PerceivedIntensity * TargetSwitchHysteresis)
		{
			// Retain current locked target stimulus to prevent target flapping
			CurrentAcousticRMS = CurrentBestStimulus.PerceivedIntensity;
			return;
		}
	}

	CurrentBestStimulus = BestCandidate;
	CurrentAcousticRMS = CurrentBestStimulus.PerceivedIntensity;

	if (CurrentBestStimulus.Instigator.IsValid())
	{
		CurrentTargetActor = CurrentBestStimulus.Instigator;
	}
}

void ALiminalEntity_Hound::CheckEyeContactIntimidation()
{
	bTargetIsIntimidating = false;

	if (!CurrentTargetActor.IsValid())
	{
		return;
	}

	const AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(CurrentTargetActor.Get());
	if (!Scavenger || Scavenger->IsDead() || Scavenger->IsDowned() || Scavenger->IsHiddenInSpot())
	{
		return;
	}

	const FVector HoundLoc = GetActorLocation();
	const FVector PlayerLoc = Scavenger->GetActorLocation();
	const float Dist = FVector::Dist(HoundLoc, PlayerLoc);

	if (Dist > IntimidationRange)
	{
		return;
	}

	// Line of sight check between Hound and player head
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HoundGazeIntimidation), false, this);
	QueryParams.AddIgnoredActor(Scavenger);

	FHitResult HitResult;
	const bool bOccluded = World->LineTraceSingleByChannel(
		HitResult, HoundLoc + FVector(0.0f, 0.0f, 40.0f), PlayerLoc + FVector(0.0f, 0.0f, 60.0f),
		ECC_Visibility, QueryParams);

	if (bOccluded)
	{
		return;
	}

	const FVector ToHound = (HoundLoc - PlayerLoc).GetSafeNormal();
	const FVector LookDir = Scavenger->GetController() ?
		Scavenger->GetControlRotation().Vector() : Scavenger->GetActorForwardVector();

	// Direct eye contact (dot > 0.707 = 45 degrees cone) holds the Hound at bay
	if (FVector::DotProduct(LookDir, ToHound) > 0.707f)
	{
		bTargetIsIntimidating = true;
	}
}

void ALiminalEntity_Hound::UpdateAcousticTracking(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!World || !Movement)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();

	UpdateAcousticStimuliFromEnvironment();
	SelectBestStimulus(CurrentTime);
	CheckEyeContactIntimidation();

	AAIController* AIC = Cast<AAIController>(GetController());
	UBlackboardComponent* BB = AIC ? AIC->GetBlackboardComponent() : nullptr;

	switch (CurrentState)
	{
	case EHoundAcousticState::Attente:
		Movement->MaxWalkSpeed = NormalStalkSpeed;
		if (CurrentBestStimulus.PerceivedIntensity >= ChargeThreshold)
		{
			CurrentState = EHoundAcousticState::Poursuite;
			LastKnownSoundLocation = CurrentBestStimulus.Location;
			PursuitLostTimer = 0.0f;
		}
		else if (CurrentBestStimulus.PerceivedIntensity >= SuspicionThreshold)
		{
			CurrentState = EHoundAcousticState::Suspicion;
			LastKnownSoundLocation = CurrentBestStimulus.Location;
		}
		break;

	case EHoundAcousticState::Suspicion:
		Movement->MaxWalkSpeed = NormalStalkSpeed;
		LastKnownSoundLocation = CurrentBestStimulus.Location;

		if (BB && !LastKnownSoundLocation.IsZero())
		{
			BB->SetValueAsVector(ALiminalAIController::PriorityTargetKey, LastKnownSoundLocation);
		}

		if (CurrentBestStimulus.PerceivedIntensity >= ChargeThreshold)
		{
			CurrentState = EHoundAcousticState::Poursuite;
			PursuitLostTimer = 0.0f;
		}
		else if (FVector::Dist2D(GetActorLocation(), LastKnownSoundLocation) < 140.0f ||
			CurrentBestStimulus.PerceivedIntensity < SuspicionThreshold)
		{
			CurrentState = EHoundAcousticState::Recherche;
			SearchTimer = 0.0f;
		}
		break;

	case EHoundAcousticState::Poursuite:
		{
			const FVector Destination = CurrentTargetActor.IsValid() ?
				CurrentTargetActor->GetActorLocation() : CurrentBestStimulus.Location;
			LastKnownSoundLocation = Destination;

			if (BB && !Destination.IsZero())
			{
				BB->SetValueAsVector(ALiminalAIController::PriorityTargetKey, Destination);
			}

			if (bTargetIsIntimidating)
			{
				// Intimidated by direct eye contact: pause charge, stalk slowly or hold ground
				Movement->MaxWalkSpeed = NormalStalkSpeed * 0.35f;
			}
			else
			{
				Movement->MaxWalkSpeed = ChargeSpeed;
			}

			// Strike if in melee range and authorized
			if (CurrentTargetActor.IsValid())
			{
				const float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTargetActor->GetActorLocation());
				if (CanAttack() && DistSq <= FMath::Square(AttackRange + 30.0f))
				{
					if (PerformMeleeAttack(CurrentTargetActor.Get()))
					{
						CurrentState = EHoundAcousticState::Attaque;
						RecoveryTimer = 0.0f;
						break;
					}
				}
			}

			// Bounded loss of visibility / acoustic silence: target broke line of sight
			if (CurrentBestStimulus.PerceivedIntensity < SuspicionThreshold || CurrentBestStimulus.bIsOccluded)
			{
				PursuitLostTimer += DeltaSeconds;
				if (PursuitLostTimer > 2.0f)
				{
					CurrentState = EHoundAcousticState::Recherche;
					SearchTimer = 0.0f;
					PursuitLostTimer = 0.0f;
				}
			}
			else
			{
				PursuitLostTimer = 0.0f;
			}
		}
		break;

	case EHoundAcousticState::Recherche:
		Movement->MaxWalkSpeed = NormalStalkSpeed;
		if (BB && !LastKnownSoundLocation.IsZero())
		{
			BB->SetValueAsVector(ALiminalAIController::PriorityTargetKey, LastKnownSoundLocation);
		}

		SearchTimer += DeltaSeconds;

		// Fresh loud noise during search immediately re-engages pursuit
		if (CurrentBestStimulus.PerceivedIntensity >= ChargeThreshold)
		{
			CurrentState = EHoundAcousticState::Poursuite;
			PursuitLostTimer = 0.0f;
			SearchTimer = 0.0f;
		}
		else if (SearchTimer >= BoundedSearchDuration)
		{
			// Search timeout: target escaped cleanly
			CurrentState = EHoundAcousticState::Recuperation;
			RecoveryTimer = 0.0f;
			SearchTimer = 0.0f;
		}
		break;

	case EHoundAcousticState::Attaque:
		CurrentState = EHoundAcousticState::Recuperation;
		RecoveryTimer = 0.0f;
		break;

	case EHoundAcousticState::Recuperation:
		Movement->MaxWalkSpeed = NormalStalkSpeed * 0.5f;
		RecoveryTimer += DeltaSeconds;
		if (RecoveryTimer >= RecoveryDuration)
		{
			CurrentState = EHoundAcousticState::Attente;
			CurrentTargetActor = nullptr;
			CurrentBestStimulus = FHoundAcousticStimulus();
			RecoveryTimer = 0.0f;
		}
		break;
	}
}
