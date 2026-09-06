#include "AI/LiminalEntity_Jerry.h"
#include "Player/ScavengerCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

ALiminalEntity_Jerry::ALiminalEntity_Jerry()
{
	MonsterType = EMonsterType::Jerry;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Jerry.BT_Jerry"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Jerry_Whisper.S_Jerry_Whisper"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Jerry_Laugh.S_Jerry_Laugh"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}
	MaxHealth = 90.0f;
	CurrentHealth = 90.0f;
	AttackDamage = 20.0f;
	AttackRange = 120.0f;
	AttackCooldownSeconds = 1.8f;

	PsionicAura = CreateDefaultSubobject<UPointLightComponent>(TEXT("PsionicAura"));
	PsionicAura->SetupAttachment(GetMesh(), TEXT("head"));
	PsionicAura->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	PsionicAura->SetLightColor(FLinearColor(0.05f, 0.75f, 1.0f));
	PsionicAura->SetIntensity(140.0f);
	PsionicAura->SetAttenuationRadius(350.0f);

	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/Characters/Bestiary/Jerry/SK_Jerry.SK_Jerry")));

	SetEntityVisualScale(FVector(0.45f, 0.45f, 0.45f));
}

void ALiminalEntity_Jerry::BeginPlay()
{
	Super::BeginPlay();
	bIsHypnotizing = false;
	HypnotizedTarget = nullptr;
}

void ALiminalEntity_Jerry::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || IsStunned() || IsCalmed())
	{
		if (bIsHypnotizing)
		{
			BreakHypnosis();
		}
		return;
	}

	UpdateHypnoticGaze(DeltaSeconds);
}

void ALiminalEntity_Jerry::UpdateHypnoticGaze(float DeltaSeconds)
{
	if (!HypnotizedTarget.IsValid() || HypnotizedTarget->IsDead())
	{
		// Find closest visible scavenger
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		AScavengerCharacter* BestTarget = nullptr;
		float BestDistSq = FMath::Square(HypnosisGazeRange);

		for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
		{
			AScavengerCharacter* Scavenger = *It;
			if (Scavenger && !Scavenger->IsDead())
			{
				const float DistSq = FVector::DistSquared(GetActorLocation(), Scavenger->GetActorLocation());
				if (DistSq < BestDistSq && HasLineOfSightTo(Scavenger))
				{
					BestDistSq = DistSq;
					BestTarget = Scavenger;
				}
			}
		}

		if (BestTarget)
		{
			HypnotizedTarget = BestTarget;
			bIsHypnotizing = true;
			BestTarget->SetHypnotized(true);
			OnJerryHypnosisChanged.Broadcast(true, BestTarget);
		}
		else
		{
			if (bIsHypnotizing)
			{
				BreakHypnosis();
			}
		}
	}
	else
	{
		// Target active: verify range and line of sight
		const float Dist = FVector::Dist(GetActorLocation(), HypnotizedTarget->GetActorLocation());
		if (Dist > HypnosisGazeRange || !HasLineOfSightTo(HypnotizedTarget.Get()))
		{
			BreakHypnosis();
			return;
		}

		// Drain target sanity heavily
		HypnotizedTarget->AuthDrainSanity(HypnosisSanityDrainPerSecond * DeltaSeconds);

		if (PsionicAura && GetWorld())
		{
			PsionicAura->SetIntensity(450.0f + 120.0f * FMath::Sin(GetWorld()->GetTimeSeconds() * 10.0f));
		}
	}
}

bool ALiminalEntity_Jerry::HasLineOfSightTo(const AActor* Target) const
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

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Target);

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
	const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);

	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
	return !bHit; // True if no obstacle
}

void ALiminalEntity_Jerry::BreakHypnosis()
{
	if (bIsHypnotizing)
	{
		bIsHypnotizing = false;
		AScavengerCharacter* OldTarget = HypnotizedTarget.Get();
		if (OldTarget)
		{
			OldTarget->SetHypnotized(false);
		}
		HypnotizedTarget = nullptr;
		OnJerryHypnosisChanged.Broadcast(false, OldTarget);

		if (PsionicAura)
		{
			PsionicAura->SetIntensity(140.0f);
		}
		UE_LOG(LogTemp, Log, TEXT("[Jerry] Hypnotic gaze broken."));
	}
}

float ALiminalEntity_Jerry::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	BreakHypnosis();
	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}
