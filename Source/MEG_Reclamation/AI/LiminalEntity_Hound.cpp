#include "AI/LiminalEntity_Hound.h"
#include "Player/ScavengerCharacter.h"
#include "Audio/LiminalProximityVoiceComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	MaxHealth = 120.0f;
	CurrentHealth = 120.0f;
	AttackDamage = 40.0f;
	AttackRange = 150.0f;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalStalkSpeed;
	}
}

void ALiminalEntity_Hound::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalEntity_Hound::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && !IsStunned() && !IsCalmed())
	{
		UpdateAcousticTracking(DeltaSeconds);
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

	const FVector HoundLoc = GetActorLocation();
	float HighestPerceivedRMS = 0.0f;
	AScavengerCharacter* BestTargetScavenger = nullptr;
	bool bTargetIsIntimidating = false;

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Player = *It;
		if (!Player || Player->IsDead() || Player->IsDowned() || Player->IsHiddenInSpot())
		{
			continue;
		}

		ULiminalProximityVoiceComponent* VoiceComp = Player->GetProximityVoice();
		if (!VoiceComp)
		{
			continue;
		}

		const float RawRMS = VoiceComp->CurrentAcousticRMS;
		if (RawRMS <= 0.001f)
		{
			continue;
		}

		const FVector PlayerLoc = Player->GetActorLocation();
		const float Dist = FVector::Dist(HoundLoc, PlayerLoc);
		if (Dist > MaxHearingRange)
		{
			continue;
		}

		// Distance falloff: linear attenuation over MaxHearingRange
		const float DistanceFactor = FMath::Clamp(1.0f - (Dist / MaxHearingRange), 0.05f, 1.0f);

		// Occlusion test: line trace for acoustic obstruction (walls/doors)
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HoundAudioOcclusion), false, this);
		QueryParams.AddIgnoredActor(Player);
		FHitResult HitResult;
		const bool bOccluded = World->LineTraceSingleByChannel(
			HitResult, HoundLoc + FVector(0.0f, 0.0f, 40.0f), PlayerLoc + FVector(0.0f, 0.0f, 60.0f),
			ECC_Visibility, QueryParams);
		const float OcclusionMultiplier = bOccluded ? OcclusionFactor : 1.0f;

		const float PerceivedRMS = RawRMS * DistanceFactor * OcclusionMultiplier;

		if (PerceivedRMS > HighestPerceivedRMS)
		{
			HighestPerceivedRMS = PerceivedRMS;
			BestTargetScavenger = Player;

			// Canonical GDD check: Intimidation eye contact at close range (< 4m)
			if (Dist <= IntimidationRange && !bOccluded)
			{
				const FVector ToHound = (HoundLoc - PlayerLoc).GetSafeNormal();
				const FVector LookDir = Player->GetControlRotation().Vector();
				if (FVector::DotProduct(LookDir, ToHound) > 0.70f)
				{
					bTargetIsIntimidating = true;
				}
			}
		}
	}

	if (HighestPerceivedRMS > RMSTrackingThreshold && BestTargetScavenger && !bTargetIsIntimidating)
	{
		bIsChargingByRMS = true;
		LastAcousticTarget = BestTargetScavenger;
		CurrentDeAggroTimer = DeAggroTime;
		Movement->MaxWalkSpeed = ChargeSpeed;

		// If in melee attack range and authorized, strike
		const float DistSq = FVector::DistSquared(HoundLoc, BestTargetScavenger->GetActorLocation());
		if (CanAttack() && DistSq <= FMath::Square(AttackRange + 30.0f))
		{
			PerformMeleeAttack(BestTargetScavenger);
		}
	}
	else if (bTargetIsIntimidating && BestTargetScavenger)
	{
		// Intimidated by direct eye contact: pause charge, slow down or hold ground
		bIsChargingByRMS = false;
		Movement->MaxWalkSpeed = NormalStalkSpeed * 0.5f;
	}
	else if (CurrentDeAggroTimer > 0.0f)
	{
		// Maintain pursuit towards last known target briefly before giving up
		CurrentDeAggroTimer -= DeltaSeconds;
		Movement->MaxWalkSpeed = ChargeSpeed;
		if (CurrentDeAggroTimer <= 0.0f)
		{
			bIsChargingByRMS = false;
			LastAcousticTarget = nullptr;
			Movement->MaxWalkSpeed = NormalStalkSpeed;
		}
	}
	else
	{
		bIsChargingByRMS = false;
		Movement->MaxWalkSpeed = NormalStalkSpeed;
	}
}
