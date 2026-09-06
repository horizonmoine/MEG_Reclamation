#include "AI/LiminalEntity_Deathmoth.h"

#include "AIController.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/ScavengerCharacter.h"

ALiminalEntity_Deathmoth::ALiminalEntity_Deathmoth()
{
	PrimaryActorTick.bCanEverTick = true;

	MonsterType = EMonsterType::Deathmoth;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Deathmoth.BT_Deathmoth"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Deathmoth_Flutter.S_Deathmoth_Flutter"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Deathmoth_Screech.S_Deathmoth_Screech"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}

	MaxHealth = 180.0f;
	CurrentHealth = 180.0f;
	AttackDamage = 25.0f;
	AttackRange = 140.0f;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Flying);
		Movement->MaxFlySpeed = DiveSpeed;
	}

	AbdomenBioluminescence = CreateDefaultSubobject<UPointLightComponent>(TEXT("AbdomenBioluminescence"));
	AbdomenBioluminescence->SetupAttachment(RootComponent);
	AbdomenBioluminescence->SetRelativeLocation(FVector(-20.0f, 0.0f, 0.0f));
	AbdomenBioluminescence->SetLightColor(FLinearColor(0.8f, 1.0f, 0.3f));
	AbdomenBioluminescence->SetIntensity(150.0f);
	AbdomenBioluminescence->SetAttenuationRadius(260.0f);

	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(FVector(1.3f, 1.6f, 0.6f));
	}
}

void ALiminalEntity_Deathmoth::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalEntity_Deathmoth::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (AbdomenBioluminescence && GetWorld())
	{
		AbdomenBioluminescence->SetIntensity(140.0f + 40.0f * FMath::Sin(GetWorld()->GetTimeSeconds() * 6.0f));
	}

	if (!HasAuthority() || IsStunned())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AScavengerCharacter* NearestPlayer = nullptr;
	float NearestDistSq = FMath::Square(LightAttractionRadius);

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Scavenger = *It;
		if (Scavenger && !Scavenger->IsDead())
		{
			const float DistSq = FVector::DistSquared(GetActorLocation(), Scavenger->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestPlayer = Scavenger;
			}
		}
	}

	if (NearestPlayer)
	{
		if (NearestDistSq <= FMath::Square(AttackRange))
		{
			PerformMeleeAttack(NearestPlayer);
		}
		else if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->MoveToActor(NearestPlayer, AttackRange * 0.8f);
		}
	}
}
