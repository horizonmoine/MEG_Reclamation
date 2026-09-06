#include "AI/LiminalEntity_Duller.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ALiminalEntity_Duller::ALiminalEntity_Duller()
{
	MonsterType = EMonsterType::Duller;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Duller.BT_Duller"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AggroFinder(TEXT("/Game/Audio/S_Duller_Growl.S_Duller_Growl"));
	if (AggroFinder.Succeeded())
	{
		AggroSound = AggroFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> AttackFinder(TEXT("/Game/Audio/S_Duller_Rush.S_Duller_Rush"));
	if (AttackFinder.Succeeded())
	{
		AttackSound = AttackFinder.Object;
	}
	MaxHealth = 130.0f;
	CurrentHealth = 130.0f;
	AttackDamage = 35.0f;
	AttackRange = 150.0f;
	AttackCooldownSeconds = 1.4f;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 390.0f;
	}

	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
	static ConstructorHelpers::FClassFinder<UAnimInstance> DullerAnimBPFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	if (DullerAnimBPFinder.Succeeded())
	{
		DefaultAnimClass = DullerAnimBPFinder.Class;
	}

	SetEntityVisualScale(FVector(1.1f, 1.1f, 1.0f));
}

void ALiminalEntity_Duller::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALiminalEntity_Duller, bIsRevealed);
}

void ALiminalEntity_Duller::BeginPlay()
{
	Super::BeginPlay();
	UpdateVisualCloakAppearance();
}

void ALiminalEntity_Duller::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		UpdateCloaking(DeltaSeconds);
	}
}

void ALiminalEntity_Duller::UpdateCloaking(float DeltaSeconds)
{
	if (bIsRevealed)
	{
		RevealTimer -= DeltaSeconds;
		if (RevealTimer <= 0.0f)
		{
			bIsRevealed = false;
			OnRep_IsRevealed();
		}
	}
}

void ALiminalEntity_Duller::RevealFromScanner(float DurationSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsRevealed = true;
	RevealTimer = FMath::Max(RevealTimer, DurationSeconds > 0.0f ? DurationSeconds : DefaultRevealDuration);
	OnRep_IsRevealed();

	UE_LOG(LogTemp, Log, TEXT("[Duller] Duller entity revealed by scan for %.1f seconds."), RevealTimer);
}

void ALiminalEntity_Duller::OnRep_IsRevealed()
{
	UpdateVisualCloakAppearance();
	OnDullerRevealStateChanged.Broadcast(bIsRevealed);
}

void ALiminalEntity_Duller::UpdateVisualCloakAppearance()
{
	SetEntityVisualVisibility(bIsRevealed);
}
