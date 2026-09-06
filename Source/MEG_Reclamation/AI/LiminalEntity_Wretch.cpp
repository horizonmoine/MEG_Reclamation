#include "AI/LiminalEntity_Wretch.h"

#include "AIController.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ALiminalEntity_Wretch::ALiminalEntity_Wretch()
{
	PrimaryActorTick.bCanEverTick = true;

	MonsterType = EMonsterType::Wretch;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Wretch.BT_Wretch"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Wretch_Snarl.S_Wretch_Snarl"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Wretch_Lunge.S_Wretch_Lunge"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}

	MaxHealth = 120.0f;
	CurrentHealth = 120.0f;
	AttackDamage = 32.0f;
	AttackRange = 130.0f;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WanderSpeed;
	}

	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 1.15f));
	}
}

void ALiminalEntity_Wretch::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalEntity_Wretch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (bIsFrenzied)
	{
		FrenzyDurationRemaining -= DeltaSeconds;
		if (FrenzyDurationRemaining <= 0.0f)
		{
			bIsFrenzied = false;
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->MaxWalkSpeed = WanderSpeed;
			}
		}
	}
}

void ALiminalEntity_Wretch::AlertToNoise(const FVector& NoiseLocation, float Loudness)
{
	if (!HasAuthority() || IsStunned())
	{
		return;
	}

	bIsFrenzied = true;
	FrenzyDurationRemaining = FMath::Clamp(Loudness * 5.0f, 3.0f, 8.0f);
	TargetNoiseLocation = NoiseLocation;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = FrenzySprintSpeed;
	}

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->MoveToLocation(TargetNoiseLocation, 80.0f);
	}
}
