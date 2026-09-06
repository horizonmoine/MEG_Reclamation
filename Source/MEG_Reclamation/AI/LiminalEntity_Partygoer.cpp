#include "AI/LiminalEntity_Partygoer.h"
#include "Player/ScavengerCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Perception/AIPerceptionComponent.h"

ALiminalEntity_Partygoer::ALiminalEntity_Partygoer()
{
	MonsterType = EMonsterType::Partygoer;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Partygoer.BT_Partygoer"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}
	MaxHealth = 160.0f;
	CurrentHealth = 160.0f;
	AttackDamage = 30.0f;
	AttackRange = 140.0f;
	AttackCooldownSeconds = 1.0f;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 520.0f; // Rapid cheerful stride
	}

	BalloonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalloonMesh"));
	BalloonMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
	BalloonMesh->SetRelativeLocation(FVector(10.0f, 0.0f, 60.0f));
	BalloonMesh->SetCollisionProfileName(TEXT("NoCollision"));

	BalloonLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BalloonLight"));
	BalloonLight->SetupAttachment(BalloonMesh);
	BalloonLight->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
	BalloonLight->SetLightColor(FLinearColor(1.0f, 0.1f, 0.1f));
	BalloonLight->SetIntensity(180.0f);
	BalloonLight->SetAttenuationRadius(240.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BalloonFinder(
		TEXT("/Game/Meshes/Props/SM_Partygoer_Balloon.SM_Partygoer_Balloon"));
	if (BalloonFinder.Succeeded())
	{
		BalloonMesh->SetStaticMesh(BalloonFinder.Object);
	}

	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
	static ConstructorHelpers::FClassFinder<UAnimInstance> PartyAnimBPFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	if (PartyAnimBPFinder.Succeeded())
	{
		DefaultAnimClass = PartyAnimBPFinder.Class;
	}

	SetEntityVisualScale(FVector(0.9f, 0.9f, 1.05f));

	static ConstructorHelpers::FObjectFinder<USoundBase> PartyAudio(
		TEXT("/Game/Audio/S_Partygoer_Chime.S_Partygoer_Chime"));
	if (PartyAudio.Succeeded())
	{
		AggroSound = PartyAudio.Object;
		AttackSound = PartyAudio.Object;
	}
}

void ALiminalEntity_Partygoer::BeginPlay()
{
	Super::BeginPlay();
	InfectionCooldownTimer = 0.0f;
}

void ALiminalEntity_Partygoer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BalloonMesh)
	{
		const float BobOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 4.0f;
		BalloonMesh->SetRelativeLocation(FVector(15.0f, 25.0f, 85.0f + BobOffset));
	}

	if (!HasAuthority() || IsStunned() || IsCalmed())
	{
		return;
	}

	UpdatePursuitAndInfection(DeltaSeconds);
}

void ALiminalEntity_Partygoer::UpdatePursuitAndInfection(float DeltaSeconds)
{
	if (InfectionCooldownTimer > 0.0f)
	{
		InfectionCooldownTimer -= DeltaSeconds;
	}

	TargetCheckTimer -= DeltaSeconds;
	if (TargetCheckTimer <= 0.0f)
	{
		CachedTarget = FindNearestLivingScavenger();
		TargetCheckTimer = 0.5f;
	}

	AScavengerCharacter* Target = CachedTarget.Get();
	if (!Target)
	{
		return;
	}

	const float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	// Passive sanity drain when close to this disturbing entity
	if (DistToTarget <= PresenceRadius)
	{
		Target->AuthDrainSanity(SanityDrainNearPresencePerSecond * DeltaSeconds);
	}

	// Attempt infection on contact
	if (DistToTarget <= InfectionRadius && InfectionCooldownTimer <= 0.0f)
	{
		TryInfectScavenger(Target);
		InfectionCooldownTimer = 3.0f;
	}

	if (CanAttack() && DistToTarget <= AttackRange)
	{
		PerformMeleeAttack(Target);
	}
}

bool ALiminalEntity_Partygoer::TryInfectScavenger(AScavengerCharacter* Target)
{
	if (!Target || Target->IsDead())
	{
		return false;
	}

	if (!Target->IsInfectedPartygoer())
	{
		Target->ServerSetInfected(true);
		OnScavengerInfected.Broadcast(Target);
		UE_LOG(LogTemp, Warning, TEXT("[Partygoer] Scavenger '%s' was infected by Partygoer! Social betrayal active."),
			*Target->GetName());
		return true;
	}

	return false;
}

AScavengerCharacter* ALiminalEntity_Partygoer::FindNearestLivingScavenger() const
{
	UAIPerceptionComponent* Perception = GetPerceptionComponent();
	if (!Perception)
	{
		return nullptr;
	}

	TArray<AActor*> PerceivedActors;
	Perception->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

	AScavengerCharacter* Nearest = nullptr;
	float ClosestDistSq = MAX_flt;

	for (AActor* Actor : PerceivedActors)
	{
		if (AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(Actor))
		{
			if (!Scavenger->IsDead())
			{
				const float DistSq = FVector::DistSquared(GetActorLocation(), Scavenger->GetActorLocation());
				if (DistSq < ClosestDistSq)
				{
					ClosestDistSq = DistSq;
					Nearest = Scavenger;
				}
			}
		}
	}

	return Nearest;
}
