#include "AI/LiminalEntity_Skinwalker.h"
#include "Player/ScavengerCharacter.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

ALiminalEntity_Skinwalker::ALiminalEntity_Skinwalker()
{
	MonsterType = EMonsterType::Skinwalker;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Skinwalker.BT_Skinwalker"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Skinwalker_Mimic.S_Skinwalker_Mimic"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Skinwalker_Scream.S_Skinwalker_Scream"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}
	MaxHealth = 180.0f;
	CurrentHealth = 180.0f;
	AttackDamage = 45.0f;
	AttackRange = 175.0f;
	AttackCooldownSeconds = 1.2f;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 480.0f;
	}

	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
	static ConstructorHelpers::FClassFinder<UAnimInstance> SkinwalkerAnimBPFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	if (SkinwalkerAnimBPFinder.Succeeded())
	{
		DefaultAnimClass = SkinwalkerAnimBPFinder.Class;
	}

	SetEntityVisualScale(FVector(1.0f, 1.0f, 1.0f));

	VoiceMimicry = CreateDefaultSubobject<UVoiceMimicryComponent>(TEXT("VoiceMimicry"));

	GlitchedHeadlamp = CreateDefaultSubobject<USpotLightComponent>(TEXT("GlitchedHeadlamp"));
	GlitchedHeadlamp->SetupAttachment(GetMesh(), TEXT("head"));
	GlitchedHeadlamp->SetRelativeLocation(FVector(15.0f, 0.0f, 5.0f));
	GlitchedHeadlamp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f));
	GlitchedHeadlamp->SetIntensity(300.0f);
	GlitchedHeadlamp->SetInnerConeAngle(18.0f);
	GlitchedHeadlamp->SetOuterConeAngle(38.0f);
}

void ALiminalEntity_Skinwalker::BeginPlay()
{
	Super::BeginPlay();
	MimicryTimer = FMath::FRandRange(2.0f, 5.0f);
}

void ALiminalEntity_Skinwalker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GlitchedHeadlamp && GetWorld())
	{
		// Erratic flickering mimicking a damaged team headlamp
		const float Flicker = (FMath::Sin(GetWorld()->GetTimeSeconds() * 25.0f) > 0.2f && FMath::FRand() > 0.15f) ? 300.0f : 20.0f;
		GlitchedHeadlamp->SetIntensity(Flicker);
	}

	if (!HasAuthority() || IsStunned() || IsCalmed())
	{
		return;
	}

	UpdateStalkingBehavior(DeltaSeconds);
}

void ALiminalEntity_Skinwalker::UpdateStalkingBehavior(float DeltaSeconds)
{
	MimicryTimer -= DeltaSeconds;

	if (!StalkedTarget.IsValid() || StalkedTarget->IsDead())
	{
		StalkedTarget = FindBestStalkTarget();
	}

	if (!StalkedTarget.IsValid())
	{
		CurrentState = ESkinwalkerState::Stalking;
		return;
	}

	const float DistToTarget = FVector::Dist(GetActorLocation(), StalkedTarget->GetActorLocation());
	const bool bIsIsolated = IsTargetIsolated(StalkedTarget.Get());

	if (DistToTarget <= AmbushTriggerDistance && bIsIsolated)
	{
		CurrentState = ESkinwalkerState::Ambushing;
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = 620.0f; // Rapid sprint on isolated prey
		}

		if (CanAttack() && DistToTarget <= AttackRange)
		{
			PerformMeleeAttack(StalkedTarget.Get());
		}
	}
	else
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = 380.0f;
		}

		if (bIsIsolated && MimicryTimer <= 0.0f)
		{
			AttemptVoiceLure(StalkedTarget.Get());
			MimicryTimer = MimicryCooldownSeconds;
		}
		else
		{
			CurrentState = ESkinwalkerState::Stalking;
		}
	}
}

bool ALiminalEntity_Skinwalker::IsTargetIsolated(const AScavengerCharacter* Target) const
{
	if (!Target)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation();

	// Check if any other living scavenger is within the threshold distance
	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		const AScavengerCharacter* Other = *It;
		if (Other && Other != Target && !Other->IsDead())
		{
			const float AllyDistSq = FVector::DistSquared(TargetLocation, Other->GetActorLocation());
			if (AllyDistSq <= FMath::Square(IsolationDistanceThreshold))
			{
				return false; // Target has support nearby
			}
		}
	}

	return true; // Completely isolated!
}

bool ALiminalEntity_Skinwalker::AttemptVoiceLure(AScavengerCharacter* Target)
{
	if (!Target || !VoiceMimicry)
	{
		return false;
	}

	CurrentState = ESkinwalkerState::MimickingVoice;

	// Emit voice from an angle away from the monster to draw the player deeper into the dark
	const FVector TargetLoc = Target->GetActorLocation();
	const FVector Offset = FMath::VRand().GetSafeNormal2D() * FMath::FRandRange(600.0f, 900.0f);
	const FVector LureLocation = TargetLoc + Offset;

	FVoiceSnippet UsedSnippet;
	const bool bSuccess = VoiceMimicry->TriggerMimicryCall(LureLocation, UsedSnippet);

	// Slightly drain target sanity due to auditory confusion
	Target->AuthDrainSanity(6.0f);

	return bSuccess;
}

AScavengerCharacter* ALiminalEntity_Skinwalker::FindBestStalkTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AScavengerCharacter* BestTarget = nullptr;
	float ClosestDistSq = MAX_flt;

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Scavenger = *It;
		if (Scavenger && !Scavenger->IsDead())
		{
			const float DistSq = FVector::DistSquared(GetActorLocation(), Scavenger->GetActorLocation());
			if (DistSq < ClosestDistSq)
			{
				ClosestDistSq = DistSq;
				BestTarget = Scavenger;
			}
		}
	}

	return BestTarget;
}
