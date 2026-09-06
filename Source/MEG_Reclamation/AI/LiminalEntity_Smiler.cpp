#include "AI/LiminalEntity_Smiler.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/ScavengerCharacter.h"

ALiminalEntity_Smiler::ALiminalEntity_Smiler()
{
	PrimaryActorTick.bCanEverTick = true;

	MonsterType = EMonsterType::Smiler;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTFinder(TEXT("/Game/AI/BT_Smiler.BT_Smiler"));
	if (BTFinder.Succeeded())
	{
		InitialBehaviorTree = BTFinder.Object;
	}

	MaxHealth = 80.0f;
	CurrentHealth = 80.0f;
	AttackDamage = 45.0f;
	AttackRange = 140.0f;

	EyeGlowLeft = CreateDefaultSubobject<UPointLightComponent>(TEXT("EyeGlowLeft"));
	EyeGlowLeft->SetupAttachment(GetMesh(), TEXT("head"));
	EyeGlowLeft->SetRelativeLocation(FVector(15.0f, -8.0f, 5.0f));
	EyeGlowLeft->SetLightColor(FLinearColor(0.9f, 1.0f, 0.9f));
	EyeGlowLeft->SetIntensity(120.0f);
	EyeGlowLeft->SetAttenuationRadius(150.0f);

	EyeGlowRight = CreateDefaultSubobject<UPointLightComponent>(TEXT("EyeGlowRight"));
	EyeGlowRight->SetupAttachment(GetMesh(), TEXT("head"));
	EyeGlowRight->SetRelativeLocation(FVector(15.0f, 8.0f, 5.0f));
	EyeGlowRight->SetLightColor(FLinearColor(0.9f, 1.0f, 0.9f));
	EyeGlowRight->SetIntensity(120.0f);
	EyeGlowRight->SetAttenuationRadius(150.0f);

	SmileMouthGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("SmileMouthGlow"));
	SmileMouthGlow->SetupAttachment(GetMesh(), TEXT("head"));
	SmileMouthGlow->SetRelativeLocation(FVector(18.0f, 0.0f, -10.0f));
	SmileMouthGlow->SetLightColor(FLinearColor(0.95f, 1.0f, 0.95f));
	SmileMouthGlow->SetIntensity(160.0f);
	SmileMouthGlow->SetAttenuationRadius(180.0f);

	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(
		FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
	static ConstructorHelpers::FClassFinder<UAnimInstance> SmilerAnimBPFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
	if (SmilerAnimBPFinder.Succeeded())
	{
		DefaultAnimClass = SmilerAnimBPFinder.Class;
	}

	// The Smiler is an apparition in the shadows: body is pitch black / tiny scale
	SetEntityVisualScale(FVector(0.01f));

	static ConstructorHelpers::FObjectFinder<USoundBase> SmilerAudio(
		TEXT("/Game/Audio/S_Smiler_Distortion.S_Smiler_Distortion"));
	if (SmilerAudio.Succeeded())
	{
		AggroSound = SmilerAudio.Object;
		AttackSound = SmilerAudio.Object;
	}
}

void ALiminalEntity_Smiler::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NormalStalkSpeed;
	}
}

void ALiminalEntity_Smiler::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && !IsStunned())
	{
		UpdateSensoryReactions(DeltaSeconds);
	}
}

void ALiminalEntity_Smiler::UpdateSensoryReactions(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!World || !Movement)
	{
		return;
	}

	bool bObservedInDarkness = false;
	bool bTargetedByLight = false;

	const FVector SmilerLocation = GetActorLocation();

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Player = *It;
		if (!Player || Player->IsDead())
		{
			continue;
		}

		const FVector PlayerLocation = Player->GetActorLocation();
		const float DistSq = FVector::DistSquared(SmilerLocation, PlayerLocation);
		if (DistSq > FMath::Square(StareDetectionRange))
		{
			continue;
		}

		const FVector ToSmiler = (SmilerLocation - PlayerLocation).GetSafeNormal();
		const FVector PlayerLookDir = Player->GetControlRotation().Vector();
		const float LookDot = FVector::DotProduct(PlayerLookDir, ToSmiler);

		if (LookDot > 0.80f)
		{
			// Le joueur regarde en direction du Smiler
			if (Player->IsHeadlampOn())
			{
				bTargetedByLight = true;
			}
			else
			{
				bObservedInDarkness = true;
				// Le regard dans l'obscurite draine la sante mentale face a l'abysse
				Player->AuthDrainSanity(2.0f * DeltaSeconds);
			}
		}
	}

	if (bTargetedByLight)
	{
		// Eclaire directement par la lampe : charge furieuse immediate !
		bIsCharging = true;
		bIsParalyzedByStare = false;
		Movement->MaxWalkSpeed = ChargeSpeed;

		if (EyeGlowLeft && EyeGlowRight && SmileMouthGlow)
		{
			EyeGlowLeft->SetIntensity(650.0f);
			EyeGlowRight->SetIntensity(650.0f);
			SmileMouthGlow->SetIntensity(750.0f);
			SmileMouthGlow->SetLightColor(FLinearColor(1.0f, 0.15f, 0.15f));
		}
	}
	else if (bObservedInDarkness)
	{
		// Regarde fixement dans le noir sans torche : paralyse par le contact visuel
		bIsParalyzedByStare = true;
		bIsCharging = false;
		Movement->StopMovementImmediately();
		Movement->MaxWalkSpeed = 0.0f;

		if (EyeGlowLeft && EyeGlowRight && SmileMouthGlow)
		{
			EyeGlowLeft->SetIntensity(80.0f);
			EyeGlowRight->SetIntensity(80.0f);
			SmileMouthGlow->SetIntensity(90.0f);
			SmileMouthGlow->SetLightColor(FLinearColor(0.85f, 1.0f, 0.85f));
		}
	}
	else if (bIsCharging)
	{
		bIsParalyzedByStare = false;
		Movement->MaxWalkSpeed = ChargeSpeed;

		if (EyeGlowLeft && EyeGlowRight && SmileMouthGlow)
		{
			EyeGlowLeft->SetIntensity(650.0f);
			EyeGlowRight->SetIntensity(650.0f);
			SmileMouthGlow->SetIntensity(750.0f);
			SmileMouthGlow->SetLightColor(FLinearColor(1.0f, 0.15f, 0.15f));
		}
	}
	else
	{
		bIsParalyzedByStare = false;
		Movement->MaxWalkSpeed = NormalStalkSpeed;

		if (EyeGlowLeft && EyeGlowRight && SmileMouthGlow)
		{
			EyeGlowLeft->SetIntensity(140.0f);
			EyeGlowRight->SetIntensity(140.0f);
			SmileMouthGlow->SetIntensity(180.0f);
			SmileMouthGlow->SetLightColor(FLinearColor(0.95f, 1.0f, 0.95f));
		}
	}
}
