#include "AI/LiminalEntity.h"

#include "AI/LiminalAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Objects/LootActor.h"

ALiminalEntity::ALiminalEntity()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ALiminalAIController::StaticClass();

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2500.0f;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*HearingConfig);
	AIPerception->SetDominantSense(UAISense_Hearing::StaticClass());

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DefaultBodyMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Hound/SM_Hound.SM_Hound")));
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
	BodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	BodyMesh->SetRelativeScale3D(FVector(0.9f));


	static ConstructorHelpers::FObjectFinder<USoundBase> BiteFinder(
		TEXT("/Game/Audio/S_Hound_Bite.S_Hound_Bite"));
	if (BiteFinder.Succeeded())
	{
		AttackSound = BiteFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SnarlFinder(
		TEXT("/Game/Audio/S_Hound_Snarl.S_Hound_Snarl"));
	if (SnarlFinder.Succeeded())
	{
		AggroSound = SnarlFinder.Object;
	}
}

EMonsterType ALiminalEntity::GetMonsterType() const
{
	return MonsterType;
}

UAIPerceptionComponent* ALiminalEntity::GetPerceptionComponent() const
{
	return AIPerception;
}

UBehaviorTree* ALiminalEntity::GetInitialBehaviorTree() const
{
	return InitialBehaviorTree;
}

void ALiminalEntity::BeginPlay()
{
	Super::BeginPlay();
	
	if (DefaultBodyMesh.IsPending())
	{
		DefaultBodyMesh.LoadSynchronous();
	}
	if (BodyMesh && DefaultBodyMesh.IsValid())
	{
		BodyMesh->SetStaticMesh(DefaultBodyMesh.Get());
	}
}

void ALiminalEntity::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (StunTimer > 0.0f)
	{
		StunTimer -= DeltaSeconds;
		if (StunTimer <= 0.0f)
		{
			StunTimer = 0.0f;
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->MaxWalkSpeed = OriginalWalkSpeed;
			}
		}
	}

	if (CalmTimer > 0.0f)
	{
		CalmTimer = FMath::Max(CalmTimer - DeltaSeconds, 0.0f);
	}

	if (AttackCooldownTimer > 0.0f)
	{
		AttackCooldownTimer = FMath::Max(AttackCooldownTimer - DeltaSeconds, 0.0f);
	}
}

void ALiminalEntity::ApplyStun(float DurationSeconds)
{
	if (DurationSeconds <= 0.0f)
	{
		return;
	}

	if (StunTimer <= 0.0f)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			OriginalWalkSpeed = Movement->MaxWalkSpeed;
			Movement->StopMovementImmediately();
			Movement->MaxWalkSpeed = 0.0f;
		}
	}

	StunTimer = FMath::Max(StunTimer, DurationSeconds);
}

void ALiminalEntity::ApplyCalm(float DurationSeconds)
{
	if (DurationSeconds <= 0.0f)
	{
		return;
	}

	CalmTimer = FMath::Max(CalmTimer, DurationSeconds);
}

bool ALiminalEntity::IsStunned() const
{
	return StunTimer > 0.0f;
}

bool ALiminalEntity::IsCalmed() const
{
	return CalmTimer > 0.0f;
}

bool ALiminalEntity::CanAttack() const
{
	return StunTimer <= 0.0f && AttackCooldownTimer <= 0.0f && CurrentHealth > 0.0f;
}

bool ALiminalEntity::PerformMeleeAttack(AActor* Target)
{
	if (!CanAttack() || !Target || !HasAuthority())
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	if (DistSq > FMath::Square(AttackRange + 50.0f))
	{
		return false;
	}

	AttackCooldownTimer = AttackCooldownSeconds;

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, this);

	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetActorLocation(),
			1.0f, FMath::FRandRange(0.85f, 1.15f));
	}

	return true;
}

float ALiminalEntity::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || Damage <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.0f);

	if (CurrentHealth <= 0.0f)
	{
		SetActorEnableCollision(false);

		// Recompense d'expedition : drop de butin / residu anomalique sur le monstre neutralise
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			const FVector DropLocation = GetActorLocation() + FVector(0.0f, 0.0f, 25.0f);
			if (ALootActor* Loot = World->SpawnActor<ALootActor>(ALootActor::StaticClass(), DropLocation, GetActorRotation(), SpawnParams))
			{
				Loot->SetRandomizedStats(FMath::FRandRange(1.8f, 3.8f), FMath::RandRange(85, 180));
			}
		}

		SetLifeSpan(3.0f);
	}

	return ActualDamage;
}
