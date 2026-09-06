#include "AI/LiminalEntity_Watcher.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ScavengerCharacter.h"

ALiminalEntity_Watcher::ALiminalEntity_Watcher()
{
	PrimaryActorTick.bCanEverTick = true;

	MonsterType = EMonsterType::Watcher;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Watcher.BT_Watcher"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Watcher_Hum.S_Watcher_Hum"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Watcher_Alert.S_Watcher_Alert"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}

	MaxHealth = 500.0f;
	CurrentHealth = 500.0f;
	AttackDamage = 0.0f;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
		Movement->MaxWalkSpeed = 0.0f;
	}

	EyeGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("EyeGlow"));
	EyeGlow->SetupAttachment(RootComponent);
	EyeGlow->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	EyeGlow->SetLightColor(FLinearColor(1.0f, 0.15f, 0.10f));
	EyeGlow->SetIntensity(220.0f);
	EyeGlow->SetAttenuationRadius(350.0f);

	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
	}
}

void ALiminalEntity_Watcher::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalEntity_Watcher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || IsStunned())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Scavenger = *It;
		if (!Scavenger || Scavenger->IsDead())
		{
			continue;
		}

		const float Dist = FVector::Dist(GetActorLocation(), Scavenger->GetActorLocation());
		if (Dist < TeleportProximityThreshold)
		{
			TeleportAway();
			return;
		}

		if (Dist < ObservationRange)
		{
			const FVector WatcherCenter = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
			const FVector PlayerViewLoc = Scavenger->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
			const FVector DirToWatcher = (WatcherCenter - PlayerViewLoc).GetSafeNormal();

			const FVector PlayerForward = Scavenger->GetActorForwardVector();
			const float Dot = FVector::DotProduct(PlayerForward, DirToWatcher);

			if (Dot > GazeDotThreshold)
			{
				FHitResult Hit;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(Scavenger);

				if (World->LineTraceSingleByChannel(Hit, PlayerViewLoc, WatcherCenter, ECC_Visibility, Params))
				{
					if (Hit.GetActor() == this)
					{
						Scavenger->AuthDrainSanity(SanityDrainPerSecondLooking * DeltaSeconds);
						CurrentEyeIntensity = 520.0f;
						if (EyeGlow)
						{
							EyeGlow->SetIntensity(CurrentEyeIntensity);
						}
					}
				}
			}
		}
	}

	if (EyeGlow && CurrentEyeIntensity > 220.0f)
	{
		CurrentEyeIntensity = FMath::FInterpTo(CurrentEyeIntensity, 220.0f, DeltaSeconds, 3.0f);
		EyeGlow->SetIntensity(CurrentEyeIntensity);
	}
}

void ALiminalEntity_Watcher::TeleportAway()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector CurrentLoc = GetActorLocation();
	const FVector Offset(
		FMath::FRandRange(-1200.0f, 1200.0f),
		FMath::FRandRange(-1200.0f, 1200.0f),
		0.0f);

	SetActorLocation(CurrentLoc + Offset, false, nullptr, ETeleportType::TeleportPhysics);
}
