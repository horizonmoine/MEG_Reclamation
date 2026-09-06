#include "AI/LiminalEntity_Clump.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ScavengerCharacter.h"

ALiminalEntity_Clump::ALiminalEntity_Clump()
{
	PrimaryActorTick.bCanEverTick = true;

	MonsterType = EMonsterType::Clump;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Clump.BT_Clump"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Clump_Gurgle.S_Clump_Gurgle"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Clump_Drag.S_Clump_Drag"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}

	MaxHealth = 150.0f;
	CurrentHealth = 150.0f;
	AttackDamage = 18.0f;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
		Movement->MaxWalkSpeed = 0.0f;
	}

	GrabTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("GrabTrigger"));
	GrabTrigger->SetupAttachment(RootComponent);
	GrabTrigger->SetSphereRadius(GrabRadius);
	GrabTrigger->SetCollisionProfileName(TEXT("Trigger"));

	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/Characters/Bestiary/Clump/SK_Clump.SK_Clump")));

	SetEntityVisualScale(FVector(1.8f, 1.8f, 0.35f));
	if (BodyMesh)
	{
		BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));
	}
}

void ALiminalEntity_Clump::BeginPlay()
{
	Super::BeginPlay();

	if (GrabTrigger)
	{
		GrabTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALiminalEntity_Clump::OnGrabTriggerOverlap);
	}
}

void ALiminalEntity_Clump::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (IsStunned())
	{
		ReleaseGrabbedPlayer();
		return;
	}

	if (GrabbedPlayer.IsValid())
	{
		AScavengerCharacter* Target = GrabbedPlayer.Get();
		if (Target->IsDead())
		{
			ReleaseGrabbedPlayer();
		}
		else
		{
			UGameplayStatics::ApplyDamage(Target, ConstrictionDamagePerSecond * DeltaSeconds,
				GetController(), this, UDamageType::StaticClass());

			if (UCharacterMovementComponent* PlayerMove = Target->GetCharacterMovement())
			{
				PlayerMove->StopMovementImmediately();
			}
		}
	}
}

void ALiminalEntity_Clump::OnGrabTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || GrabbedPlayer.IsValid() || IsStunned())
	{
		return;
	}

	if (AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(OtherActor))
	{
		if (!Scavenger->IsDead())
		{
			GrabbedPlayer = Scavenger;
		}
	}
}

void ALiminalEntity_Clump::ReleaseGrabbedPlayer()
{
	GrabbedPlayer = nullptr;
}
