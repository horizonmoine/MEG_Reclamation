#include "Player/ScavengerCharacter.h"

#include "AI/LiminalEntity.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Data/QuotaManager.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameModes/LiminalGameMode.h"
#include "GameModes/LiminalZoneRulesSubsystem.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LiminalScavengerHUD.h"
#include "Net/UnrealNetwork.h"
#include "Objects/LootActor.h"
#include "Objects/LiminalTerminalActor.h"
#include "Objects/LiminalAirlockActor.h"
#include "Perception/AISense_Hearing.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Player/LiminalFootstepComponent.h"
#include "Player/LiminalSpectatorPawn.h"
#include "Sanity/LiminalSanityPostProcessComponent.h"
#include "Sound/SoundBase.h"
#include "Components/SpotLightComponent.h"
#include "Tools/AudioDecoyTool.h"
#include "Tools/AlmondWaterSprayTool.h"
#include "Tools/BaseTool.h"
#include "Tools/FlashStrobeTool.h"
#include "Tools/SonicMicrowaveTool.h"
#include "Tools/LidarScannerTool.h"
#include "Tools/SignalAnalyzerTool.h"
#include "Tools/RealityAnchorTool.h"
#include "Tools/TetherTool.h"
#include "Tools/ChalkMarkerTool.h"
#include "Tools/AdrenalineInjectorTool.h"
#include "Data/LiminalGameInstance.h"
#include "Objects/LiminalDoorActor.h"
#include "Objects/LiminalHidingSpot.h"
#include "Objects/LiminalVentActor.h"
#include "Objects/LiminalBreakerActor.h"
#include "Player/LiminalBodycamComponent.h"
#include "Objects/LiminalValvePuzzleActor.h"
#include "Objects/LiminalFuseBoxActor.h"
#include "Objects/LiminalKeyItemActor.h"
#include "Objects/LiminalKeypadActor.h"
#include "Tools/WalkieTalkieTool.h"
#include "Inventory/LiminalTetrisInventory.h"
#include "Audio/LiminalProximityVoiceComponent.h"

AScavengerCharacter::AScavengerCharacter()
{
	bReplicates = true;

	// Controle First Person omnidirectionnel strict
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->MaxWalkSpeed = 450.0f;
		Movement->MaxAcceleration = 2400.0f;
		Movement->BrakingDecelerationWalking = 2400.0f;
		Movement->GroundFriction = 8.0f;
		Movement->bCanWalkOffLedges = true;
		Movement->bCanWalkOffLedgesWhenCrouching = true;
		Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
	}

	SanityPostProcess = CreateDefaultSubobject<ULiminalSanityPostProcessComponent>(TEXT("SanityPostProcess"));

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->InterpolationSpeed = 30.0f;
	PhysicsHandle->LinearDamping = 150.0f;
	PhysicsHandle->LinearStiffness = 4000.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimBPFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->bCastHiddenShadow = false;
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FirstPersonMesh->SetRelativeLocation(FVector(-10.0f, 0.0f, -155.0f));
	FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	if (MannyMeshFinder.Succeeded())
	{
		FirstPersonMesh->SetSkeletalMesh(MannyMeshFinder.Object);
	}
	if (UnarmedAnimBPFinder.Succeeded())
	{
		FirstPersonMesh->SetAnimInstanceClass(UnarmedAnimBPFinder.Class);
	}

	if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
	{
		ThirdPersonMesh->SetOwnerNoSee(true);
		ThirdPersonMesh->SetCastShadow(true);
		ThirdPersonMesh->bCastHiddenShadow = true;
		ThirdPersonMesh->SetCollisionProfileName(TEXT("CharacterMesh"));
		ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
		ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		if (MannyMeshFinder.Succeeded())
		{
			ThirdPersonMesh->SetSkeletalMesh(MannyMeshFinder.Object);
		}
		if (UnarmedAnimBPFinder.Succeeded())
		{
			ThirdPersonMesh->SetAnimInstanceClass(UnarmedAnimBPFinder.Class);
		}
	}

	FirstPersonToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonToolMesh"));
	FirstPersonToolMesh->SetupAttachment(FirstPersonMesh, TEXT("hand_r"));
	FirstPersonToolMesh->SetRelativeLocation(FVector(4.0f, 2.0f, -1.0f));
	FirstPersonToolMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	FirstPersonToolMesh->SetRelativeScale3D(FVector(0.35f));
	FirstPersonToolMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FirstPersonToolMesh->SetCastShadow(false);
	FirstPersonToolMesh->SetOnlyOwnerSee(true);

	DefaultToolMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Tools/SM_FlashStrobe.SM_FlashStrobe")));

	HeadlampLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlampLight"));
	HeadlampLight->SetupAttachment(FirstPersonCamera);
	HeadlampLight->SetRelativeLocation(FVector(20.0f, 0.0f, -10.0f));
	HeadlampLight->SetIntensity(3500.0f);
	HeadlampLight->SetAttenuationRadius(3000.0f);
	HeadlampLight->SetInnerConeAngle(30.0f);
	HeadlampLight->SetOuterConeAngle(55.0f);
	HeadlampLight->SetLightColor(FLinearColor(1.0f, 0.96f, 0.88f));
	HeadlampLight->SetVisibility(true);

	FootstepAudio = CreateDefaultSubobject<ULiminalFootstepComponent>(TEXT("FootstepAudio"));
	BodycamComponent = CreateDefaultSubobject<ULiminalBodycamComponent>(TEXT("BodycamComponent"));
	TetrisInventory = CreateDefaultSubobject<ULiminalTetrisInventoryComponent>(TEXT("TetrisInventory"));
	ProximityVoice = CreateDefaultSubobject<ULiminalProximityVoiceComponent>(TEXT("ProximityVoice"));

	DefaultToolClasses = {
		AFlashStrobeTool::StaticClass(),
		ASonicMicrowaveTool::StaticClass(),
		AAlmondWaterSprayTool::StaticClass(),
		AAudioDecoyTool::StaticClass()
	};

	InputActionUseTool = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_UseTool.IA_UseTool")));
	InputActionCycleTool = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_CycleTool.IA_CycleTool")));
	InputActionDeployTool = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_DeployTool.IA_DeployTool")));
	InputActionToggleHeadlamp = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_ToggleHeadlamp.IA_ToggleHeadlamp")));
	InputActionInteract = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Interact.IA_Interact")));
	InputActionCrouch = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Crouch.IA_Crouch")));
	InputActionDropLoot = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_DropLoot.IA_DropLoot")));
	InputActionThrowLoot = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_ThrowLoot.IA_ThrowLoot")));
	InputActionNightVision = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_NightVision.IA_NightVision")));
	InputActionFieldManual = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_FieldManual.IA_FieldManual")));

	ScavengerMappingContext = TSoftObjectPtr<UInputMappingContext>(
		FSoftObjectPath(TEXT("/Game/Input/Scavenger/IMC_Scavenger.IMC_Scavenger")));
	InputActionMove = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Move.IA_Move")));
	InputActionLook = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Look.IA_Look")));
	InputActionJump = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Jump.IA_Jump")));
	InputActionGrab = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Grab.IA_Grab")));
	InputActionSprint = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Scavenger/IA_Sprint.IA_Sprint")));
}


void AScavengerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsDead = false;
	CurrentStamina = MaxStamina;
	CurrentSanity = MaxSanity;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		BaseWalkSpeed = Movement->MaxWalkSpeed;
	}

	if (GetCapsuleComponent())
	{
		StandingCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	}
	if (FirstPersonCamera)
	{
		StandingCameraZ = FirstPersonCamera->GetRelativeLocation().Z;
		if (GConfig)
		{
			float UserFOV = 90.0f;
			if (GConfig->GetFloat(TEXT("Gameplay"), TEXT("FOV"), UserFOV, GGameUserSettingsIni))
			{
				FirstPersonCamera->SetFieldOfView(FMath::Clamp(UserFOV, 60.0f, 120.0f));
			}
		}
	}

	if (DefaultToolMesh.IsPending())
	{
		DefaultToolMesh.LoadSynchronous();
	}
	if (FirstPersonToolMesh && DefaultToolMesh.IsValid())
	{
		FirstPersonToolMesh->SetStaticMesh(DefaultToolMesh.Get());
	}

	UpdateMovementFromWeight();

	if (HasAuthority())
	{
		EquipLoadout();
	}
}

void AScavengerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Lean interpolation
	CurrentLeanAmount = FMath::FInterpTo(CurrentLeanAmount, TargetLeanAmount, DeltaSeconds, LeanSpeed);

	// Crouch transition
	const float TargetCrouchAlpha = bIsCrouching ? 1.0f : 0.0f;
	CrouchAlpha = FMath::FInterpTo(CrouchAlpha, TargetCrouchAlpha, DeltaSeconds, CrouchTransitionSpeed);

	if (FirstPersonCamera)
	{
		const float TargetCamZ = FMath::Lerp(StandingCameraZ, CrouchedCameraZ, CrouchAlpha);
		const float LeanY = CurrentLeanAmount * MaxLeanOffset;
		FirstPersonCamera->SetRelativeLocation(FVector(0.0f, LeanY, TargetCamZ));
	}

	if (HasAuthority())
	{
		// Exits can complete asynchronously or at the far end of a vent.
		bIsHiddenInSpot = CurrentHidingSpot.IsValid() && CurrentHidingSpot->GetOccupant() == this;
		bIsInVent = CurrentVent.IsValid() && CurrentVent->GetOccupant() == this;
		if (!bIsHiddenInSpot) CurrentHidingSpot.Reset();
		if (!bIsInVent) CurrentVent.Reset();
		UpdateRevive(DeltaSeconds);
		UpdateStamina(DeltaSeconds);
		EmitFootstepNoise(DeltaSeconds);
		UpdateCarriedObjectTarget();
		UpdateSanityPressure(DeltaSeconds);

		if (bIsSprinting)
		{
			const bool bMoving = GetVelocity().SizeSquared2D() > FMath::Square(MinimumNoiseSpeed);
			if (!bMoving || CurrentStamina <= 0.0f)
			{
				ServerSetSprinting_Implementation(false);
			}
			else
			{
				ServerDrainStamina_Implementation(SprintStaminaDrainPerSecond * DeltaSeconds);
				if (CurrentStamina <= 0.0f)
				{
					ServerSetSprinting_Implementation(false);
				}
			}
		}
	}

	if (HasAuthority())
	{
		if (bIsDowned && !bIsDead)
		{
			DownedTimeRemaining -= DeltaSeconds;
			if (DownedTimeRemaining <= 0.0f)
			{
				Die(nullptr);
			}
		}
	}

	if (bHasAdrenalineRush)
	{
		AdrenalineRushTimer -= DeltaSeconds;
		if (AdrenalineRushTimer <= 0.0f)
		{
			bHasAdrenalineRush = false;
			AdrenalineRushTimer = 0.0f;
		}
	}

	UpdateDebugHud();
	UpdateLocalEffects(DeltaSeconds);
	UpdateLootGaze();
}

void AScavengerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AScavengerCharacter, CurrentHealth, COND_OwnerOnly);
	DOREPLIFETIME(AScavengerCharacter, bIsDead);
	DOREPLIFETIME(AScavengerCharacter, bIsDowned);
	DOREPLIFETIME_CONDITION(AScavengerCharacter, bIsHiddenInSpot, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AScavengerCharacter, bIsInVent, COND_OwnerOnly);
	DOREPLIFETIME(AScavengerCharacter, DownedTimeRemaining);
	DOREPLIFETIME(AScavengerCharacter, CurrentToolIndex);
	DOREPLIFETIME(AScavengerCharacter, OwnedTools);

	DOREPLIFETIME_CONDITION(AScavengerCharacter, CurrentStamina, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AScavengerCharacter, CurrentInventoryWeightKg, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AScavengerCharacter, CurrentSanity, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AScavengerCharacter, CarriedCredits, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AScavengerCharacter, bIsSprinting, COND_OwnerOnly);
	DOREPLIFETIME(AScavengerCharacter, bIsInfectedPartygoer);
	DOREPLIFETIME(AScavengerCharacter, bIsInStasis);
}

void AScavengerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (UInputMappingContext* MappingContext = ScavengerMappingContext.LoadSynchronous())
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void AScavengerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (UInputMappingContext* MappingContext = ScavengerMappingContext.LoadSynchronous())
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void AScavengerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 1. Fallback analogique standard (garantit le deplacement et la camera meme si les assets IMC ne sont pas charges)
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AScavengerCharacter::FallbackMoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AScavengerCharacter::FallbackMoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AScavengerCharacter::FallbackTurn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AScavengerCharacter::FallbackLookUp);

	// Actions hardware directes (depuis DefaultInput.ini)
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AScavengerCharacter::HandleSprintStarted);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AScavengerCharacter::HandleSprintStopped);
	PlayerInputComponent->BindAction(TEXT("Grab"), IE_Pressed, this, &AScavengerCharacter::HandleGrabPressed);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AScavengerCharacter::Interact);
	PlayerInputComponent->BindAction(TEXT("UseTool"), IE_Pressed, this, &AScavengerCharacter::HandleUseToolPressed);
	PlayerInputComponent->BindAction(TEXT("DeployTool"), IE_Pressed, this, &AScavengerCharacter::HandleDeployToolPressed);
	PlayerInputComponent->BindAction(TEXT("ToggleHeadlamp"), IE_Pressed, this, &AScavengerCharacter::HandleToggleHeadlamp);
	PlayerInputComponent->BindAction(TEXT("CycleToolNext"), IE_Pressed, this, &AScavengerCharacter::HandleCycleToolPressed);
	PlayerInputComponent->BindAction(TEXT("CycleToolPrev"), IE_Pressed, this, &AScavengerCharacter::HandleCycleToolPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AScavengerCharacter::StartCrouch);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &AScavengerCharacter::StopCrouch);
	PlayerInputComponent->BindAction(TEXT("NightVision"), IE_Pressed, this, &AScavengerCharacter::ToggleNightVision);
	PlayerInputComponent->BindAction(TEXT("DropLoot"), IE_Pressed, this, &AScavengerCharacter::DropCarriedLootOnGround);

	// Liaisons directes pour le manuel de terrain tactique M.E.G.
	PlayerInputComponent->BindKey(EKeys::M, IE_Pressed, this, &AScavengerCharacter::ToggleFieldManual);
	PlayerInputComponent->BindKey(EKeys::J, IE_Pressed, this, &AScavengerCharacter::ToggleFieldManual);

	// Touches directes universelles de secours
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &AScavengerCharacter::Interact);
	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AScavengerCharacter::HandleToggleHeadlamp);
	PlayerInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AScavengerCharacter::StartCrouch);
	PlayerInputComponent->BindKey(EKeys::C, IE_Released, this, &AScavengerCharacter::StopCrouch);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &AScavengerCharacter::StartCrouch);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Released, this, &AScavengerCharacter::StopCrouch);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &AScavengerCharacter::ToggleNightVision);
	PlayerInputComponent->BindKey(EKeys::X, IE_Pressed, this, &AScavengerCharacter::DropCarriedLootOnGround);

	// Liaisons directes pour le lancer physique et la prise (conforme a DefaultInput.ini)
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AScavengerCharacter::InputThrowLoot);
	PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AScavengerCharacter::HandleGrabPressed);

	// Note: Les touches Z/S/Q/D et W/S/A/D sont deja liees via MoveForward et MoveRight
	// dans DefaultInput.ini. Les doublons BindAxisKey ont ete supprimes pour eviter le doublement de vitesse.

	// 2. Enhanced Input system
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	// Les axes de deplacement (ZQSD / WASD / Fleches) et de camera (Souris) sont geres nativement
	// et sans conflit par les BindAxis ci-dessus, garantissant zero saccade diagonale.
	if (UInputAction* JumpAction = InputActionJump.LoadSynchronous())
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (UInputAction* GrabAction = InputActionGrab.LoadSynchronous())
	{
		EnhancedInput->BindAction(GrabAction, ETriggerEvent::Started, this, &AScavengerCharacter::HandleGrabPressed);
	}
	if (UInputAction* SprintAction = InputActionSprint.LoadSynchronous())
	{
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AScavengerCharacter::HandleSprintStarted);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AScavengerCharacter::HandleSprintStopped);
	}
	if (UInputAction* UseToolAction = InputActionUseTool.LoadSynchronous())
	{
		EnhancedInput->BindAction(UseToolAction, ETriggerEvent::Started, this, &AScavengerCharacter::HandleUseToolPressed);
	}
	if (UInputAction* CycleToolAction = InputActionCycleTool.LoadSynchronous())
	{
		EnhancedInput->BindAction(CycleToolAction, ETriggerEvent::Started, this, &AScavengerCharacter::HandleCycleToolPressed);
	}
	if (UInputAction* DeployAction = InputActionDeployTool.LoadSynchronous())
	{
		EnhancedInput->BindAction(DeployAction, ETriggerEvent::Started, this, &AScavengerCharacter::HandleDeployToolPressed);
	}
	if (UInputAction* LampAction = InputActionToggleHeadlamp.LoadSynchronous())
	{
		EnhancedInput->BindAction(LampAction, ETriggerEvent::Started, this, &AScavengerCharacter::HandleToggleHeadlamp);
	}
	if (UInputAction* InteractAction = InputActionInteract.LoadSynchronous())
	{
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AScavengerCharacter::Interact);
	}
	if (UInputAction* CrouchAction = InputActionCrouch.LoadSynchronous())
	{
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AScavengerCharacter::StartCrouch);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AScavengerCharacter::StopCrouch);
	}
	if (UInputAction* DropLootAction = InputActionDropLoot.LoadSynchronous())
	{
		EnhancedInput->BindAction(DropLootAction, ETriggerEvent::Started, this, &AScavengerCharacter::DropCarriedLootOnGround);
	}
	if (UInputAction* ThrowLootAction = InputActionThrowLoot.LoadSynchronous())
	{
		EnhancedInput->BindAction(ThrowLootAction, ETriggerEvent::Started, this, &AScavengerCharacter::InputThrowLoot);
	}
	if (UInputAction* NightVisionAction = InputActionNightVision.LoadSynchronous())
	{
		EnhancedInput->BindAction(NightVisionAction, ETriggerEvent::Started, this, &AScavengerCharacter::ToggleNightVision);
	}
	if (UInputAction* FieldManualAction = InputActionFieldManual.LoadSynchronous())
	{
		EnhancedInput->BindAction(FieldManualAction, ETriggerEvent::Started, this, &AScavengerCharacter::ToggleFieldManual);
	}
}


void AScavengerCharacter::HandleMove(const FInputActionValue& Value)
{
	if (bIsHypnotized)
	{
		return;
	}

	const FVector2D RawAxis = Value.Get<FVector2D>();
	if (RawAxis.IsNearlyZero() || !Controller)
	{
		return;
	}

	// Normalisation / clamp composite pour eviter le boost diagonal (sqrt(2))
	const FVector2D Axis = RawAxis.GetClampedToMaxSize(1.0f);

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRot.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, Axis.Y);
	AddMovementInput(RightDirection, Axis.X);

	if (TetheredPartner.IsValid())
	{
		const float TetherDist = FVector::Dist(GetActorLocation(), TetheredPartner->GetActorLocation());
		if (TetherDist > 1600.0f)
		{
			const FVector DirToPartner = (TetheredPartner->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			AddMovementInput(DirToPartner, 0.4f);
		}
	}
}

void AScavengerCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	float MouseSensitivity = 1.0f;
	bool bInvertY = false;
	if (GConfig)
	{
		GConfig->GetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), MouseSensitivity, GGameUserSettingsIni);
		GConfig->GetBool(TEXT("Gameplay"), TEXT("InvertY"), bInvertY, GGameUserSettingsIni);
	}
	const float SensitivityScale = FMath::Clamp(MouseSensitivity, 0.1f, 5.0f) * 0.45f;
	const float PitchSign = bInvertY ? -1.0f : 1.0f;

	AddControllerYawInput(Axis.X * SensitivityScale);
	AddControllerPitchInput(Axis.Y * SensitivityScale * PitchSign);
}

void AScavengerCharacter::FallbackMoveForward(float Val)
{
	if (FMath::Abs(Val) > 0.001f && Controller && !bIsHypnotized)
	{
		const FRotator ControlRot = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRot.Yaw, 0.0f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, FMath::Clamp(Val, -1.0f, 1.0f));
	}
}

void AScavengerCharacter::FallbackMoveBackward(float Val)
{
	FallbackMoveForward(-Val);
}

void AScavengerCharacter::FallbackMoveLeft(float Val)
{
	FallbackMoveRight(-Val);
}

void AScavengerCharacter::FallbackMoveRight(float Val)
{
	if (FMath::Abs(Val) > 0.001f && Controller && !bIsHypnotized)
	{
		const FRotator ControlRot = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRot.Yaw, 0.0f);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, FMath::Clamp(Val, -1.0f, 1.0f));
	}
}

void AScavengerCharacter::FallbackTurn(float Val)
{
	if (FMath::Abs(Val) > 0.0001f)
	{
		float MouseSensitivity = 1.0f;
		if (GConfig)
		{
			GConfig->GetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), MouseSensitivity, GGameUserSettingsIni);
		}
		const float SensitivityScale = FMath::Clamp(MouseSensitivity, 0.1f, 5.0f) * 0.45f;
		AddControllerYawInput(Val * SensitivityScale);
	}
}

void AScavengerCharacter::FallbackLookUp(float Val)
{
	if (FMath::Abs(Val) > 0.0001f)
	{
		float MouseSensitivity = 1.0f;
		bool bInvertY = false;
		if (GConfig)
		{
			GConfig->GetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), MouseSensitivity, GGameUserSettingsIni);
			GConfig->GetBool(TEXT("Gameplay"), TEXT("InvertY"), bInvertY, GGameUserSettingsIni);
		}
		const float SensitivityScale = FMath::Clamp(MouseSensitivity, 0.1f, 5.0f) * 0.45f;
		const float PitchSign = bInvertY ? -1.0f : 1.0f;
		AddControllerPitchInput(Val * SensitivityScale * PitchSign);
	}
}

void AScavengerCharacter::ToggleHeadlamp()
{
	if (HeadlampLight)
	{
		HeadlampLight->ToggleVisibility();
	}
}

bool AScavengerCharacter::IsHeadlampOn() const
{
	return HeadlampLight && HeadlampLight->IsVisible();
}

bool AScavengerCharacter::AddOwnedTool(TSubclassOf<ABaseTool> ToolClass)
{
	if (!ToolClass || !HasAuthority())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABaseTool* Tool = World->SpawnActor<ABaseTool>(ToolClass, GetActorTransform(), Params);
	if (!Tool)
	{
		return false;
	}

	Tool->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Tool->SetActorRelativeLocation(FVector(35.0f, 28.0f, -15.0f));
	Tool->SetActorHiddenInGame(true);
	OwnedTools.Add(Tool);

	if (OwnedTools.Num() == 1)
	{
		CurrentToolIndex = 0;
		OwnedTools[0]->SetActorHiddenInGame(false);
	}

	return true;
}

void AScavengerCharacter::RechargeCurrentToolBattery(float PercentAmount)
{
	if (ABaseTool* Tool = GetCurrentTool())
	{
		const float MaxBatt = Tool->GetMaxBatteryCharge();
		const float CurrentBatt = Tool->GetBatteryCharge();
		const float AddAmount = (PercentAmount > 1.0f) ? PercentAmount : (PercentAmount * MaxBatt);
		Tool->SetBatteryCharge(FMath::Clamp(CurrentBatt + AddAmount, 0.0f, MaxBatt));
	}
}

void AScavengerCharacter::HealAndRestoreSanity(float HealthAmount, float SanityAmount)
{
	if (HasAuthority())
	{
		CurrentHealth = FMath::Clamp(CurrentHealth + HealthAmount, 0.0f, MaxHealth);
		CurrentSanity = FMath::Clamp(CurrentSanity + SanityAmount, 0.0f, MaxSanity);
		OnRep_CurrentHealth();
		OnRep_CurrentSanity();
		ClientOnSanityRestored(CurrentSanity / MaxSanity);
	}
}

void AScavengerCharacter::AuthSetHealthAndSanity(float AbsoluteHealth, float AbsoluteSanity)
{
	if (HasAuthority())
	{
		CurrentHealth = FMath::Clamp(AbsoluteHealth, 0.0f, MaxHealth);
		CurrentSanity = FMath::Clamp(AbsoluteSanity, 0.0f, MaxSanity);
		bIsDead = (CurrentHealth <= 0.0f);
		bIsDowned = (CurrentHealth > 0.0f && CurrentHealth <= 15.0f);
		OnRep_CurrentHealth();
		OnRep_CurrentSanity();
		ClientOnSanityRestored(CurrentSanity / MaxSanity);
	}
}

void AScavengerCharacter::AuthSetInventoryWeight(float NewWeightKg)
{
	if (HasAuthority())
	{
		CurrentInventoryWeightKg = FMath::Clamp(NewWeightKg, 0.0f, MaxCarryWeightKg);
		OnRep_CurrentInventoryWeightKg();
	}
}

void AScavengerCharacter::EnterStasis()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsInStasis = true;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}
	OnRep_IsInStasis();
}

void AScavengerCharacter::ExitStasis()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsInStasis = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
	OnRep_IsInStasis();
}

void AScavengerCharacter::OnRep_IsInStasis()
{
	if (bIsInStasis)
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->DisableMovement();
			MoveComp->StopMovementImmediately();
		}
	}
	else
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}
}

void AScavengerCharacter::HandleToggleHeadlamp()
{
	ToggleHeadlamp();
}

void AScavengerCharacter::DeployEquippedTool()
{
	ServerDeployTool();
}

void AScavengerCharacter::ServerDeployTool_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	ABaseTool* Tool = GetCurrentTool();
	if (Tool)
	{
		Tool->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		const FVector DropLoc = GetActorLocation() + GetActorForwardVector() * 90.0f;
		Tool->SetActorLocation(DropLoc);
		Tool->SetActorHiddenInGame(false);

		OwnedTools.Remove(Tool);
		if (CurrentToolIndex >= OwnedTools.Num())
		{
			CurrentToolIndex = FMath::Max(0, OwnedTools.Num() - 1);
		}
		OnRep_CurrentToolIndex();
	}
}

void AScavengerCharacter::HandleDeployToolPressed()
{
	DeployEquippedTool();
}

float AScavengerCharacter::GetToolBatteryCharge() const
{
	if (const ABaseTool* Tool = GetCurrentTool())
	{
		return Tool->GetBatteryNormalized();
	}
	return 0.0f;
}

void AScavengerCharacter::HandleGrabPressed()
{
	if (HeldLoot.IsValid())
	{
		InputRelease();
		return;
	}

	// Interaction au regard avec les elements du monde (Terminal et Sas)
	if (FirstPersonCamera)
	{
		const FVector TraceStart = FirstPersonCamera->GetComponentLocation();
		const FVector TraceEnd = TraceStart + FirstPersonCamera->GetForwardVector() * 320.0f;
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			if (ALiminalTerminalActor* Terminal = Cast<ALiminalTerminalActor>(Hit.GetActor()))
			{
				Terminal->Interact(this);
				return;
			}
			if (ALiminalAirlockActor* Airlock = Cast<ALiminalAirlockActor>(Hit.GetActor()))
			{
				Airlock->Interact(this);
				return;
			}
		}
	}

	InputGrab();
}

void AScavengerCharacter::HandleUseToolPressed()
{
	ServerUseTool();
}

void AScavengerCharacter::HandleCycleToolPressed()
{
	ServerCycleTool();
}

ABaseTool* AScavengerCharacter::GetCurrentTool() const
{
	return OwnedTools.IsValidIndex(CurrentToolIndex) ? OwnedTools[CurrentToolIndex].Get() : nullptr;
}

FName AScavengerCharacter::GetCurrentToolName() const
{
	const ABaseTool* Tool = GetCurrentTool();
	return Tool ? FName(*Tool->GetClass()->GetName()) : NAME_None;
}

void AScavengerCharacter::EquipLoadout()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const TSubclassOf<ABaseTool>& ToolClass : DefaultToolClasses)
	{
		if (!ToolClass)
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABaseTool* Tool = World->SpawnActor<ABaseTool>(ToolClass, GetActorTransform(), Params);
		if (!Tool)
		{
			continue;
		}

		Tool->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Tool->SetActorRelativeLocation(FVector(35.0f, 28.0f, -15.0f));
		Tool->SetActorHiddenInGame(true);
		OwnedTools.Add(Tool);
	}

	// Charger les outils achetes et stockes en persistance M.E.G.
	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
	{
		auto SpawnStoredToolIfMissing = [&](TSubclassOf<ABaseTool> ToolClass)
		{
			if (!ToolClass) return;
			for (const TObjectPtr<ABaseTool>& Existing : OwnedTools)
			{
				if (Existing && Existing->IsA(ToolClass))
				{
					return;
				}
			}

			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (ABaseTool* NewTool = World->SpawnActor<ABaseTool>(ToolClass, GetActorTransform(), Params))
			{
				NewTool->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				NewTool->SetActorRelativeLocation(FVector(35.0f, 28.0f, -15.0f));
				NewTool->SetActorHiddenInGame(true);
				OwnedTools.Add(NewTool);
			}
		};

		for (const FName& StoredId : GI->GetSaveData().StoredToolIds)
		{
			if (StoredId == FName("Tool_FlashStrobe")) SpawnStoredToolIfMissing(AFlashStrobeTool::StaticClass());
			else if (StoredId == FName("Tool_SonicMicrowave")) SpawnStoredToolIfMissing(ASonicMicrowaveTool::StaticClass());
			else if (StoredId == FName("Tool_AudioDecoy")) SpawnStoredToolIfMissing(AAudioDecoyTool::StaticClass());
			else if (StoredId == FName("Tool_LidarScanner")) SpawnStoredToolIfMissing(ALidarScannerTool::StaticClass());
			else if (StoredId == FName("Tool_SignalAnalyzer")) SpawnStoredToolIfMissing(ASignalAnalyzerTool::StaticClass());
			else if (StoredId == FName("Tool_RealityAnchor")) SpawnStoredToolIfMissing(ARealityAnchorTool::StaticClass());
			else if (StoredId == FName("Tool_Tether")) SpawnStoredToolIfMissing(ATetherTool::StaticClass());
			else if (StoredId == FName("Tool_ChalkMarker")) SpawnStoredToolIfMissing(AChalkMarkerTool::StaticClass());
			else if (StoredId == FName("Tool_AdrenalineInjector")) SpawnStoredToolIfMissing(AAdrenalineInjectorTool::StaticClass());
		}
	}

	if (OwnedTools.Num() > 0)
	{
		CurrentToolIndex = 0;
		OwnedTools[0]->SetActorHiddenInGame(false);
	}
}

void AScavengerCharacter::ServerUseTool_Implementation()
{
	ABaseTool* Tool = GetCurrentTool();
	if (!Tool || !HasAuthority())
	{
		return;
	}

	if (Tool->IsActive())
	{
		Tool->Deactivate();
	}
	else
	{
		Tool->Activate();
	}
}

void AScavengerCharacter::ServerCycleTool_Implementation()
{
	const int32 Count = OwnedTools.Num();
	if (Count < 2)
	{
		return;
	}

	if (ABaseTool* Current = GetCurrentTool())
	{
		if (Current->IsActive())
		{
			Current->Deactivate();
		}
	}

	CurrentToolIndex = (CurrentToolIndex + 1) % Count;
	OnRep_CurrentToolIndex();
}

void AScavengerCharacter::UpdateSanityPressure(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	if (!World || CurrentSanity <= 0.0f)
	{
		return;
	}

	if (!ULiminalZoneRulesSubsystem::IsSanityPressureActive(this))
	{
		const float RestoreRate = ULiminalZoneRulesSubsystem::GetSafeZoneSanityRestoreRate(this);
		if (RestoreRate > 0.0f && CurrentSanity < MaxSanity && HasAuthority())
		{
			ServerRestoreSanity(RestoreRate * DeltaSeconds);
		}
		return;
	}

	// Reality Anchor et injection d'adrenaline stoppent l'erosion mentale
	if (bInsideRealityAnchor || bHasAdrenalineRush)
	{
		return;
	}

	float Drain = 0.0f;

	bool bAllyNear = false;
	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		if (*It != this &&
			FVector::DistSquared2D(It->GetActorLocation(), GetActorLocation()) < FMath::Square(AllySupportRange))
		{
			bAllyNear = true;
			break;
		}
	}
	if (!bAllyNear)
	{
		Drain += SanityIsolationDrainPerSecond;
	}

	float NearestEntitySq = TNumericLimits<float>::Max();
	for (TActorIterator<ALiminalEntity> EntityIt(World); EntityIt; ++EntityIt)
	{
		NearestEntitySq = FMath::Min(NearestEntitySq,
			static_cast<float>(FVector::DistSquared2D(EntityIt->GetActorLocation(), GetActorLocation())));
	}
	if (NearestEntitySq < FMath::Square(HoundProximityRange))
	{
		Drain += SanityHoundProximityDrainPerSecond;
	}

	// Erosion psychologique en pleine obscurite (lampe torche eteinte)
	if (!IsHeadlampOn())
	{
		Drain += SanityDarknessDrainPerSecond;
	}

	if (Drain > 0.0f)
	{
		ServerDrainSanity(Drain * DeltaSeconds);
	}
}

void AScavengerCharacter::UpdateLocalEffects(float DeltaSeconds)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (DamageFlashIntensity > 0.0f)
	{
		DamageFlashIntensity = FMath::Max(0.0f, DamageFlashIntensity - DeltaSeconds * 2.2f);
	}

	if (SanityPostProcess)
	{
		SanityPostProcess->UpdateSanityState(CurrentSanity, MaxSanity);
	}

	const float Sanity01 = GetSanityPercent();
	const bool bSanityPressureActive = ULiminalZoneRulesSubsystem::IsSanityPressureActive(this);

	if (bSanityPressureActive && Sanity01 < 0.40f)
	{
		FakeAlertTimer += DeltaSeconds;
		if (FakeAlertTimer >= 7.0f)
		{
			FakeAlertTimer = 0.0f;
			static const TArray<FString> GlitchMessages = {
				TEXT("DÉCONNEXION BALISE M.E.G. // LOCALISATION PERDUE"),
				TEXT("ALERTE : ANOMALIE BIOMÉTRIQUE IMMÉDIATE DERRIÈRE VOUS"),
				TEXT("QUOTA RÉVOQUÉ : REPORT DE DETTE IMMINENT"),
				TEXT("AVERTISSEMENT : BRÈCHE DU SAS D'EXTRACTION"),
				TEXT("ERREUR CRITIQUE // PRESSION DE RÉALITÉ DÉFAILLANTE")
			};
			ActiveFakeAlert = GlitchMessages[FMath::RandRange(0, GlitchMessages.Num() - 1)];
		}

		GhostFootstepTimer += DeltaSeconds;
		if (GhostFootstepTimer >= 8.5f)
		{
			GhostFootstepTimer = 0.0f;
			if (FootstepAudio)
			{
				FootstepAudio->PlayFootstep(0.4f);
			}
		}
	}
	else
	{
		ActiveFakeAlert.Empty();
		FakeAlertTimer = 0.0f;
		GhostFootstepTimer = 0.0f;
	}

	if (FirstPersonCamera)
	{
		FPostProcessSettings& PostProcess = FirstPersonCamera->PostProcessSettings;

		const float Intensity = FMath::Clamp((0.6f - Sanity01) / 0.6f, 0.0f, 1.0f);
		const float Health01 = GetHealthPercent();
		const float LowHealthIntensity = FMath::Clamp((0.4f - Health01) / 0.4f, 0.0f, 1.0f);

		PostProcess.bOverride_SceneFringeIntensity = true;
		PostProcess.SceneFringeIntensity = (Intensity + LowHealthIntensity) * 2.0f;
		PostProcess.bOverride_VignetteIntensity = true;
		PostProcess.VignetteIntensity = 0.15f + Intensity * 0.4f + LowHealthIntensity * 0.4f;
		PostProcess.bOverride_FilmGrainIntensity = true;
		PostProcess.FilmGrainIntensity = (Intensity * 0.3f) + (LowHealthIntensity * 0.4f);

		const FVector BaseCamLoc = FVector(0.0f, 0.0f, 65.0f);
		const float Speed2D = GetVelocity().Size2D();
		if (Speed2D > 25.0f && GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
		{
			const float BobFreq = bIsSprinting ? 14.0f : 9.5f;
			HeadBobPhase += BobFreq * DeltaSeconds;
			const float BobZAmp = bIsSprinting ? 2.5f : 1.2f;
			const float BobYAmp = bIsSprinting ? 1.5f : 0.8f;
			const FVector BobOffset = FVector(0.0f, FMath::Cos(HeadBobPhase * 0.5f) * BobYAmp, FMath::Sin(HeadBobPhase) * BobZAmp);
			FirstPersonCamera->SetRelativeLocation(FMath::VInterpTo(FirstPersonCamera->GetRelativeLocation(), BaseCamLoc + BobOffset, DeltaSeconds, 15.0f));
		}
		else
		{
			HeadBobPhase = 0.0f;
			if (!FirstPersonCamera->GetRelativeLocation().Equals(BaseCamLoc, 0.1f))
			{
				FirstPersonCamera->SetRelativeLocation(
					FMath::VInterpTo(FirstPersonCamera->GetRelativeLocation(), BaseCamLoc, DeltaSeconds, 10.0f));
			}
		}
	}

	if (!bSanityPressureActive || Sanity01 >= HallucinationSanityThresholdPercent)
	{
		TimeToNextHallucination = HallucinationMinIntervalSeconds;
		return;
	}

	TimeToNextHallucination -= DeltaSeconds;
	if (TimeToNextHallucination <= 0.0f)
	{
		TimeToNextHallucination =
			FMath::FRandRange(HallucinationMinIntervalSeconds, HallucinationMaxIntervalSeconds);
		SpawnHallucination();
	}
}

void AScavengerCharacter::SpawnHallucination()
{
	if (!ULiminalZoneRulesSubsystem::IsSanityPressureActive(this))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (FootstepAudio)
	{
		FootstepAudio->PlayFootstep(0.75f);
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Ghost = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		GetActorLocation() + FVector(0.0f, 0.0f, 60.0f), FRotator::ZeroRotator, Params);
	if (!Ghost)
	{
		return;
	}

	Ghost->SetOwner(this);
	Ghost->SetMobility(EComponentMobility::Movable);
	Ghost->SetLifeSpan(0.35f);

	UStaticMeshComponent* GhostMesh = Ghost->GetStaticMeshComponent();
	UStaticMesh* SpookyMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"));
	if (!SpookyMesh)
	{
		SpookyMesh = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Game/Meshes/Modular/SM_Door_Leaf.SM_Door_Leaf"));
	}
	if (GhostMesh && SpookyMesh)
	{
		GhostMesh->SetMobility(EComponentMobility::Movable);
		GhostMesh->SetStaticMesh(SpookyMesh);
		GhostMesh->SetWorldScale3D(FVector(0.8f));
		GhostMesh->bOwnerNoSee = false;
		GhostMesh->bOnlyOwnerSee = true;
	}

	const FVector Behind = -GetActorForwardVector() * 250.0f +
		GetActorRightVector() * FMath::FRandRange(-150.0f, 150.0f);
	Ghost->SetActorLocation(GetActorLocation() + Behind + FVector(0.0f, 0.0f, 60.0f));
}

void AScavengerCharacter::HandleSprintStarted()
{
	if (CurrentStamina > 0.0f)
	{
		bIsSprinting = true;
		UpdateMovementFromWeight();
	}
	ServerSetSprinting(true);
}

void AScavengerCharacter::HandleSprintStopped()
{
	bIsSprinting = false;
	UpdateMovementFromWeight();
	ServerSetSprinting(false);
}

void AScavengerCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	if (bNewSprinting && CurrentStamina <= 0.0f)
	{
		return;
	}

	bIsSprinting = bNewSprinting;
	UpdateMovementFromWeight();
}

void AScavengerCharacter::ServerDrainStamina_Implementation(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float WeightFactor = 1.0f + (GetWeightRatio() * 0.5f);
	CurrentStamina = FMath::Max(CurrentStamina - Amount * WeightFactor, 0.0f);
	TimeSinceStaminaDrain = 0.0f;
}

void AScavengerCharacter::ServerAddInventoryWeight(float WeightKg)
{
	if (!HasAuthority() || !FMath::IsFinite(WeightKg) || WeightKg <= 0.0f)
	{
		return;
	}

	CurrentInventoryWeightKg = FMath::Min(CurrentInventoryWeightKg + WeightKg, MaxCarryWeightKg);
}

void AScavengerCharacter::ServerRemoveInventoryWeight(float WeightKg)
{
	if (!HasAuthority() || !FMath::IsFinite(WeightKg) || WeightKg <= 0.0f)
	{
		return;
	}

	CurrentInventoryWeightKg = FMath::Max(CurrentInventoryWeightKg - WeightKg, 0.0f);
}

void AScavengerCharacter::ServerDrainSanity(float Amount)
{
	AuthDrainSanity(Amount);
}

void AScavengerCharacter::AuthDrainSanity(float Amount)
{
	if (!FMath::IsFinite(Amount) || Amount <= 0.0f || !HasAuthority())
	{
		return;
	}

	CurrentSanity = FMath::Max(CurrentSanity - Amount, 0.0f);
}

void AScavengerCharacter::ServerRestoreSanity(float Amount)
{
	if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.0f)
	{
		return;
	}

	CurrentSanity = FMath::Min(CurrentSanity + Amount, MaxSanity);
	ClientOnSanityRestored(CurrentSanity / MaxSanity);
}

float AScavengerCharacter::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

bool AScavengerCharacter::ShouldTakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser) const
{
	if ((GetLocalRole() < ROLE_Authority) || !CanBeDamaged() || Damage <= 0.0f)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// In non-transient game worlds where a GameInstance is present, require AuthGameMode
	if (!World->GetAuthGameMode() && World->GetGameInstance() != nullptr)
	{
		return false;
	}

	return true;
}

float AScavengerCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead || Damage <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.0f);

	ClientOnDamaged(GetHealthPercent());

	if (CurrentHealth <= 0.0f)
	{
		if (!bIsDowned)
		{
			EnterDownedState();
		}
		else
		{
			Die(EventInstigator);
		}
	}

	return ActualDamage;
}

void AScavengerCharacter::EnterDownedState()
{
	if (!HasAuthority() || bIsDowned || bIsDead)
	{
		return;
	}

	bIsDowned = true;
	DownedTimeRemaining = 45.0f;
	CurrentHealth = 0.0f;

	DropCarriedLootOnGround();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = 80.0f; // Ramper au sol
		bIsSprinting = false;
	}

	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 25.0f));
	}

	OnRep_IsDowned();
}

void AScavengerCharacter::OnRep_IsDowned()
{
	if (bIsDowned)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = 80.0f;
		}
		if (FirstPersonCamera)
		{
			FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 25.0f));
		}
	}
	else
	{
		UpdateMovementFromWeight();
		if (FirstPersonCamera)
		{
			FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 65.0f));
		}
	}
}

void AScavengerCharacter::ServerRevivePlayer(AScavengerCharacter* Reviver)
{
	if (HasAuthority() && IsValid(Reviver))
	{
		Reviver->ServerRequestRevive(this);
	}
}

bool AScavengerCharacter::CanReviveTarget(const AScavengerCharacter* Target) const
{
	if (!HasAuthority() || !GetWorld() || !IsValid(Target) || Target == this ||
		bIsDead || bIsDowned || bIsHypnotized || bIsHiddenInSpot || bIsInVent ||
		Target->IsDead() || !Target->IsDowned() ||
		FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) > FMath::Square(250.0f))
	{
		return false;
	}
	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ReviveVisibility), false, this);
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit, EyeLocation, Target->GetActorLocation(), ECC_Visibility, Params);
	return !bBlocked || Hit.GetActor() == Target;
}

void AScavengerCharacter::ServerRequestRevive_Implementation(AScavengerCharacter* Target)
{
	if (!CanReviveTarget(Target)) return;
	if (PendingReviveTarget.Get() != Target)
	{
		PendingReviveTarget = Target;
		ReviveElapsedSeconds = 0.0f;
	}
}

void AScavengerCharacter::UpdateRevive(float DeltaSeconds)
{
	AScavengerCharacter* Target = PendingReviveTarget.Get();
	if (!CanReviveTarget(Target))
	{
		PendingReviveTarget.Reset();
		ReviveElapsedSeconds = 0.0f;
		return;
	}
	ReviveElapsedSeconds += DeltaSeconds;
	if (ReviveElapsedSeconds >= 3.0f)
	{
		Target->Revive(0.4f, 0.5f);
		PendingReviveTarget.Reset();
		ReviveElapsedSeconds = 0.0f;
	}
}

void AScavengerCharacter::DropCarriedLootOnGround()
{
	if (!HasAuthority())
	{
		return;
	}

	if (HeldLoot.IsValid())
	{
		ServerRelease();
	}

	if (CarriedCredits > 0)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			const FVector SpawnLoc = GetActorLocation() + FVector(FMath::RandRange(-40.0f, 40.0f), FMath::RandRange(-40.0f, 40.0f), 25.0f);
			if (ALootActor* Dropped = World->SpawnActor<ALootActor>(ALootActor::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params))
			{
				Dropped->SetRandomizedStats(FMath::Max(1.0f, CurrentInventoryWeightKg), CarriedCredits);
			}
		}
		CarriedCredits = 0;
		CurrentInventoryWeightKg = 0.0f;
	}
}

void AScavengerCharacter::Die(AController* Killer)
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsDowned = false;

	DropCarriedLootOnGround();

	if (HeldLoot.IsValid())
	{
		ServerRelease();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (FirstPersonMesh)
	{
		FirstPersonMesh->SetVisibility(false);
	}
	if (FirstPersonToolMesh)
	{
		FirstPersonToolMesh->SetVisibility(false);
	}
	if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
	{
		ThirdPersonMesh->SetOwnerNoSee(false);
	}

	if (ALiminalGameMode* GameMode = Cast<ALiminalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnPlayerDied(this);
	}
}

void AScavengerCharacter::ClientOnDamaged_Implementation(float HealthPercent)
{
	DamageFlashIntensity = 1.0f;

	// Coup de recul caméra immersif lors de l'impact
	AddControllerPitchInput(FMath::FRandRange(-2.5f, -4.5f));
	AddControllerYawInput(FMath::FRandRange(-3.5f, 3.5f));

	if (FirstPersonCamera)
	{
		const FVector PunchOffset = FVector(-12.0f, FMath::FRandRange(-8.0f, 8.0f), FMath::FRandRange(-6.0f, 6.0f));
		FirstPersonCamera->SetRelativeLocation(FirstPersonCamera->GetRelativeLocation() + PunchOffset);
	}
}

float AScavengerCharacter::GetStaminaPercent() const
{
	return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f;
}

float AScavengerCharacter::GetWeightRatio() const
{
	return MaxCarryWeightKg > 0.0f ? FMath::Clamp(CurrentInventoryWeightKg / MaxCarryWeightKg, 0.0f, 1.0f) : 0.0f;
}

float AScavengerCharacter::GetSanityPercent() const
{
	return MaxSanity > 0.0f ? CurrentSanity / MaxSanity : 0.0f;
}

int32 AScavengerCharacter::GetCarriedCredits() const
{
	return CarriedCredits;
}

void AScavengerCharacter::AddCarriedCredits(int32 Amount)
{
	CarriedCredits = FMath::Max(0, CarriedCredits + Amount);
}

void AScavengerCharacter::SetCarriedCredits(int32 Amount)
{
	CarriedCredits = FMath::Max(0, Amount);
}

void AScavengerCharacter::OnRep_CarriedCredits()
{
}

void AScavengerCharacter::DeliverCarriedLoot()
{
	if (!HasAuthority())
	{
		return;
	}

	if (CarriedCredits > 0)
	{
		if (UQuotaManager* QuotaManager = GetGameInstance()->GetSubsystem<UQuotaManager>())
		{
			QuotaManager->AddDeliveredValue(CarriedCredits);
		}
	}

	CarriedCredits = 0;
	ServerRemoveInventoryWeight(CurrentInventoryWeightKg);

	if (HeldLoot.IsValid())
	{
		ALootActor* Loot = HeldLoot.Get();
		HeldLoot = nullptr;

		if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
		{
			PhysicsHandle->ReleaseComponent();
		}

		Loot->Destroy();
	}
}

void AScavengerCharacter::UpdateStamina(float DeltaSeconds)
{
	if (bHasAdrenalineRush)
	{
		CurrentStamina = MaxStamina;
		return;
	}

	TimeSinceStaminaDrain += DeltaSeconds;

	// Essoufflement critique : respiration haletante audible par les entités dans le noir
	if (CurrentStamina < 20.0f && GetVelocity().SizeSquared2D() > FMath::Square(MinimumNoiseSpeed))
	{
		FootstepNoiseTimer += DeltaSeconds;
		if (FootstepNoiseTimer >= 1.2f)
		{
			FootstepNoiseTimer = 0.0f;
			UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.7f, this);
			MakeNoise(0.7f, this, GetActorLocation());
		}
	}

	if (TimeSinceStaminaDrain < StaminaRegenDelaySeconds || CurrentStamina >= MaxStamina)
	{
		return;
	}

	CurrentStamina = FMath::Min(CurrentStamina + StaminaRegenPerSecond * DeltaSeconds, MaxStamina);
}

void AScavengerCharacter::UpdateMovementFromWeight()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	const float AdrenalineFactor = bHasAdrenalineRush ? 1.35f : 1.0f;
	const float EffectiveWeightRatio = bHasAdrenalineRush ? 0.0f : GetWeightRatio();
	const float WeightFactor = FMath::Lerp(1.0f, WalkSpeedAtMaxWeightFactor, EffectiveWeightRatio);
	const float SprintFactor = bIsSprinting ? SprintSpeedMultiplier * (1.0f - EffectiveWeightRatio * 0.15f) : 1.0f;

	Movement->MaxWalkSpeed = BaseWalkSpeed * WeightFactor * FMath::Max(SprintFactor, 0.0f) * AdrenalineFactor;
}

void AScavengerCharacter::InputGrab()
{
	ServerTryGrab();
}

void AScavengerCharacter::InputRelease()
{
	ServerRelease();
}

void AScavengerCharacter::ServerTryGrab_Implementation()
{
	ALootActor* Candidate = nullptr;
	UPrimitiveComponent* Primitive = nullptr;
	if (!TraceForGrabbable(Candidate, Primitive) || Candidate->IsClaimed())
	{
		return;
	}

	const float Weight = Candidate->GetWeightKg();
	if (CurrentInventoryWeightKg + Weight > MaxCarryWeightKg + KINDA_SMALL_NUMBER)
	{
		return;
	}

	const int32 Value = Candidate->GetCreditsValue();
	ServerAddInventoryWeight(Weight);
	CarriedCredits += Value;

	// Son discret de rangement acoustique dans le sac a dos
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.35f, this);

	// Notification immediate HUD (Toast vert neon + feedback tactile)
	ClientOnLootCollected(Value, Weight);

	// L'objet est range directement dans le sac a dos tactique M.E.G. (les mains restent 100% libres pour les outils)
	Candidate->Destroy();
}

void AScavengerCharacter::ServerRelease_Implementation()
{
	if (!HeldLoot.IsValid())
	{
		return;
	}

	ALootActor* Loot = HeldLoot.Get();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		PhysicsHandle->ReleaseComponent();
	}

	if (UPrimitiveComponent* Primitive = Loot->GetLootPrimitive())
	{
		Primitive->MoveIgnoreActors.Remove(this);
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->MoveIgnoreActors.Remove(Loot);
	}

	ServerRemoveInventoryWeight(Loot->GetWeightKg());
	CarriedCredits = FMath::Max(CarriedCredits - Loot->GetCreditsValue(), 0);
	Loot->SetClaimed(nullptr);
	HeldLoot = nullptr;
}

void AScavengerCharacter::InputThrowLoot()
{
	if (HeldLoot.IsValid())
	{
		ServerThrowLoot(1.0f);
	}
}

void AScavengerCharacter::ServerThrowLoot_Implementation(float ForceMultiplier)
{
	if (!HeldLoot.IsValid())
	{
		return;
	}

	ALootActor* Loot = HeldLoot.Get();

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		PhysicsHandle->ReleaseComponent();
	}

	UPrimitiveComponent* Primitive = Loot->GetLootPrimitive();
	if (Primitive)
	{
		Primitive->MoveIgnoreActors.Remove(this);
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->MoveIgnoreActors.Remove(Loot);
	}

	ServerRemoveInventoryWeight(Loot->GetWeightKg());
	CarriedCredits = FMath::Max(CarriedCredits - Loot->GetCreditsValue(), 0);
	Loot->SetClaimed(nullptr);
	HeldLoot = nullptr;

	if (Primitive)
	{
		Primitive->SetSimulatePhysics(true);

		// Objets legers volent loin et vite, objets lourds sont projetes a courte portee (style R.E.P.O.)
		const float Weight = FMath::Max(Loot->GetWeightKg(), 0.5f);
		const float LaunchSpeed = (1200.0f / FMath::Sqrt(Weight)) * FMath::Clamp(ForceMultiplier, 0.5f, 2.5f);
		const FVector LookDir = GetControlRotation().Vector();
		const FVector LaunchDir = (LookDir + FVector(0.0f, 0.0f, 0.22f)).GetSafeNormal();
		const FVector FinalVelocity = (LaunchDir * LaunchSpeed) + (GetVelocity() * 0.75f);

		Primitive->SetPhysicsLinearVelocity(FinalVelocity);
		Primitive->SetPhysicsAngularVelocityInDegrees(FVector(
			FMath::FRandRange(-180.0f, 180.0f),
			FMath::FRandRange(-180.0f, 180.0f),
			FMath::FRandRange(-180.0f, 180.0f)));

		// Bruit diegetique d'effort lors de la projection
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.6f, this);
		MakeNoise(0.6f, this, GetActorLocation());
	}
}

void AScavengerCharacter::TriggerAdrenalineRush(float DurationSeconds)
{
	bHasAdrenalineRush = true;
	AdrenalineRushTimer = FMath::Max(AdrenalineRushTimer, DurationSeconds);
	CurrentStamina = MaxStamina;
}

void AScavengerCharacter::Revive(float HealthPercent, float SanityPercent)
{
	if (!HasAuthority() || !FMath::IsFinite(HealthPercent) || !FMath::IsFinite(SanityPercent)) return;
	if (!bIsDead && !bIsDowned)
	{
		return;
	}

	bIsDead = false;
	bIsDowned = false;
	DownedTimeRemaining = 45.0f;
	CurrentHealth = FMath::Clamp(MaxHealth * HealthPercent, 15.0f, MaxHealth);
	CurrentSanity = FMath::Clamp(MaxSanity * SanityPercent, 20.0f, MaxSanity);
	CurrentStamina = MaxStamina;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionProfileName(TEXT("Pawn"));
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	OnRep_IsDowned();

	// Reposseder ce corps si le controleur etait en mode spectateur
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (ALiminalSpectatorPawn* Spec = Cast<ALiminalSpectatorPawn>(PC->GetPawn()))
				{
					if (FVector::DistSquared(Spec->GetActorLocation(), GetActorLocation()) < FMath::Square(1200.0f))
					{
						PC->UnPossess();
						PC->Possess(this);
						Spec->Destroy();
						break;
					}
				}
			}
		}
	}

	TriggerAdrenalineRush(8.0f);
	ClientOnSanityRestored(CurrentSanity / MaxSanity);
}

bool AScavengerCharacter::TraceForGrabbable(ALootActor*& OutLoot, UPrimitiveComponent*& OutPrimitive) const
{
	OutLoot = nullptr;
	OutPrimitive = nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector End = Start + GetControlRotation().Vector() * GrabRange;

	FCollisionQueryParams Params(TEXT("ScavengerGrab"), false);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	if (!World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(20.0f), Params))
	{
		return false;
	}

	ALootActor* Loot = Cast<ALootActor>(Hit.GetActor());
	UPrimitiveComponent* Primitive = Hit.GetComponent();
	if (!Loot || !Primitive)
	{
		return false;
	}

	OutLoot = Loot;
	OutPrimitive = Primitive;
	return true;
}

void AScavengerCharacter::UpdateCarriedObjectTarget()
{
	if (!HeldLoot.IsValid() || !PhysicsHandle || !PhysicsHandle->GetGrabbedComponent())
	{
		return;
	}

	const FRotator ControlRotator = GetControlRotation();
	const FVector TargetLocation = GetActorLocation() + ControlRotator.Vector() * CarryDistance;
	PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, ControlRotator);
}

void AScavengerCharacter::UpdateDebugHud()
{
	if (!IsLocallyControlled() || !GEngine)
	{
		return;
	}

	const UQuotaManager* QuotaManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuotaManager>() : nullptr;

	const FVector PlayerPosition = GetActorLocation();
	FName ToolName = GetCurrentToolName();
	const FString Info = FString::Printf(
		TEXT("Vie %.0f%%  |  Sanité %.0f%%  |  Stamina %.0f%%  |  Poids %.0f / %.0f kg%s  |  Poche %d cr  |  Quota %d / %d (dette %d)%s  |  Outil [Q]: %s [F]%s  |  Pos: %.0f, %.0f, %.0f"),
		GetHealthPercent() * 100.0f,
		GetSanityPercent() * 100.0f,
		GetStaminaPercent() * 100.0f,
		CurrentInventoryWeightKg,
		MaxCarryWeightKg,
		bIsSprinting ? TEXT(" [SPRINT]") : TEXT(""),
		CarriedCredits,
		QuotaManager ? QuotaManager->GetDeliveredValue() : 0,
		QuotaManager ? QuotaManager->GetTotalDue() : 0,
		QuotaManager ? QuotaManager->GetOutstandingDebt() : 0,
		HeldLoot.IsValid() ? TEXT("  [TRANSPORTE E]") : TEXT(""),
		ToolName != NAME_None ? *ToolName.ToString() : TEXT("aucun"),
		(GetCurrentTool() && GetCurrentTool()->IsActive()) ? TEXT(" [ACTIF]") : TEXT(""),
		PlayerPosition.X, PlayerPosition.Y, PlayerPosition.Z);

	GEngine->AddOnScreenDebugMessage((int32)(GetUniqueID() & 0x7FFFFFFF), -1.0f, FColor::White, Info);
}

void AScavengerCharacter::EmitFootstepNoise(float DeltaSeconds)
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || Movement->Velocity.SizeSquared2D() < FMath::Square(MinimumNoiseSpeed))
	{
		FootstepNoiseTimer = 0.0f;
		return;
	}

	FootstepNoiseTimer += DeltaSeconds;
	if (FootstepNoiseTimer < FootstepNoiseIntervalSeconds)
	{
		return;
	}
	FootstepNoiseTimer = 0.0f;

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), FootstepLoudness, this);

	if (FootstepAudio)
	{
		FootstepAudio->PlayFootstep(1.0f);
	}
}

void AScavengerCharacter::OnRep_CurrentStamina()
{
}

void AScavengerCharacter::OnRep_CurrentInventoryWeightKg()
{
	UpdateMovementFromWeight();
}

void AScavengerCharacter::OnRep_IsSprinting()
{
	UpdateMovementFromWeight();
}

void AScavengerCharacter::OnRep_CurrentSanity()
{
}

void AScavengerCharacter::OnRep_CurrentHealth()
{
}

void AScavengerCharacter::OnRep_IsDead()
{
	if (bIsDead)
	{
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
		}
	}
}

void AScavengerCharacter::OnRep_CurrentToolIndex()
{
	for (int32 Index = 0; Index < OwnedTools.Num(); ++Index)
	{
		if (OwnedTools[Index])
		{
			OwnedTools[Index]->SetActorHiddenInGame(Index != CurrentToolIndex);
		}
	}

	if (FirstPersonToolMesh)
	{
		const ABaseTool* CurTool = GetCurrentTool();
		FirstPersonToolMesh->SetVisibility(CurTool != nullptr);

		if (CurTool)
		{
			UStaticMesh* DesiredMesh = nullptr;
			if (CurTool->IsA<AFlashStrobeTool>())
			{
				DesiredMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Tools/SM_FlashStrobe.SM_FlashStrobe"));
			}
			else if (CurTool->IsA<AAlmondWaterSprayTool>())
			{
				DesiredMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Tools/SM_AlmondWaterSpray.SM_AlmondWaterSpray"));
			}
			else if (CurTool->IsA<AAudioDecoyTool>())
			{
				DesiredMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Tools/SM_AudioDecoy.SM_AudioDecoy"));
			}
			else if (CurTool->IsA<AWalkieTalkieTool>())
			{
				DesiredMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Tools/SM_WalkieTalkie.SM_WalkieTalkie"));
			}

			if (DesiredMesh)
			{
				FirstPersonToolMesh->SetStaticMesh(DesiredMesh);
			}
		}
	}
}

void AScavengerCharacter::UpdateLootGaze()
{
	if (!IsLocallyControlled() || !FirstPersonCamera)
	{
		return;
	}

	const FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + FirstPersonCamera->GetForwardVector() * 350.0f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	ALootActor* TargetLoot = nullptr;
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		TargetLoot = Cast<ALootActor>(Hit.GetActor());
	}

	if (TargetLoot != HighlightedLoot.Get())
	{
		if (HighlightedLoot.IsValid())
		{
			HighlightedLoot->HighlightLoot(false);
		}

		HighlightedLoot = TargetLoot;

		if (HighlightedLoot.IsValid())
		{
			HighlightedLoot->HighlightLoot(true);
		}
	}
}

void AScavengerCharacter::ServerSetInfected(bool bInfected)
{
	if (!HasAuthority()) return;
	bIsInfectedPartygoer = bInfected;
	OnRep_IsInfectedPartygoer();
}

void AScavengerCharacter::OnRep_IsInfectedPartygoer()
{
	if (bIsInfectedPartygoer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scavenger] %s is now INFECTED by Partygoer!"), *GetName());
	}
}

void AScavengerCharacter::ToggleFieldManual()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ALiminalScavengerHUD* ScavHUD = Cast<ALiminalScavengerHUD>(PC->GetHUD()))
		{
			ScavHUD->ToggleFieldManual();
		}
	}
}

void AScavengerCharacter::ClientOnSanityRestored_Implementation(float SanityPercent)
{
	if (SanityPostProcess)
	{
		SanityPostProcess->UpdateSanityState(CurrentSanity, MaxSanity);
	}
}

void AScavengerCharacter::ClientOnLootCollected_Implementation(int32 Credits, float WeightKg)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ALiminalScavengerHUD* ScavHUD = Cast<ALiminalScavengerHUD>(PC->GetHUD()))
		{
			ScavHUD->ShowLootPickupNotification(Credits, WeightKg);
		}
	}
}

void AScavengerCharacter::StartCrouch()
{
	bWantsToCrouch = true;
	bIsCrouching = true;
	Crouch();
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = BaseWalkSpeed * CrouchSpeedMultiplier;
	}
}

void AScavengerCharacter::StopCrouch()
{
	bWantsToCrouch = false;
	bIsCrouching = false;
	UnCrouch();
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = BaseWalkSpeed;
		UpdateMovementFromWeight();
	}
}

void AScavengerCharacter::StartLeanLeft()
{
	TargetLeanAmount = -1.0f;
}

void AScavengerCharacter::StartLeanRight()
{
	TargetLeanAmount = 1.0f;
}

void AScavengerCharacter::StopLean()
{
	TargetLeanAmount = 0.0f;
}

void AScavengerCharacter::Interact()
{
	if (bIsHiddenInSpot || bIsInVent)
	{
		ServerInteract();
		return;
	}

	if (!FirstPersonCamera)
	{
		return;
	}

	const FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (FirstPersonCamera->GetForwardVector() * 320.0f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionClient), false, this);

	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(30.0f), Params);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (ALiminalTerminalActor* Terminal = Cast<ALiminalTerminalActor>(HitActor))
		{
			Terminal->Interact(this);
			return; // Terminal UI is local
		}
	}

	ServerInteract();
}

bool AScavengerCharacter::ServerInteract_Validate()
{
	return true;
}

void AScavengerCharacter::ServerInteract_Implementation()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World || bIsDead || bIsDowned || bIsHypnotized) return;

	const double Now = World->GetTimeSeconds();
	if (LastDoorInteractionTime >= 0.0 && Now - LastDoorInteractionTime < 0.2) return;
	LastDoorInteractionTime = Now;

	if (CurrentHidingSpot.IsValid() && CurrentHidingSpot->GetOccupant() == this)
	{
		// ForceExit starts a transition. Tick clears the flag once it finishes.
		CurrentHidingSpot->ForceExit();
		return;
	}
	if (CurrentVent.IsValid() && CurrentVent->GetOccupant() == this)
	{
		CurrentVent->ExitVent();
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);
	const FVector TraceEnd = EyeLocation + EyeRotation.Vector() * 320.0f;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionServer), false, this);
	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(HitResults, EyeLocation, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(30.0f), Params);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (ALiminalAirlockActor* Airlock = Cast<ALiminalAirlockActor>(HitActor))
		{
			Airlock->Interact(this);
			return;
		}

		if (ALiminalDoorActor* Door = Cast<ALiminalDoorActor>(HitActor))
		{
			Door->Interact(this);
			return;
		}

		if (AScavengerCharacter* Teammate = Cast<AScavengerCharacter>(HitActor))
		{
			ServerRequestRevive(Teammate);
			return;
		}

		if (ALiminalKeypadActor* Keypad = Cast<ALiminalKeypadActor>(HitActor))
		{
			Keypad->SubmitCode();
			return;
		}

		if (ALiminalHidingSpot* Spot = Cast<ALiminalHidingSpot>(HitActor))
		{
			Spot->TryEnter(this);
			if (Spot->IsOccupied() && Spot->GetOccupant() == this)
			{
				bIsHiddenInSpot = true;
				CurrentHidingSpot = Spot;
			}
			return;
		}

		if (ALiminalVentActor* Vent = Cast<ALiminalVentActor>(HitActor))
		{
			Vent->TryEnter(this);
			if (Vent->GetOccupant() == this)
			{
				bIsInVent = true;
				CurrentVent = Vent;
			}
			return;
		}

		if (ALiminalBreakerActor* Breaker = Cast<ALiminalBreakerActor>(HitActor))
		{
			Breaker->SetBreakerState(!Breaker->IsPowerRestored());
			return;
		}

		if (ALiminalValvePuzzleActor* Valve = Cast<ALiminalValvePuzzleActor>(HitActor))
		{
			Valve->Interact(this);
			return;
		}

		if (ALiminalFuseBoxActor* FuseBox = Cast<ALiminalFuseBoxActor>(HitActor))
		{
			FuseBox->Interact(this);
			return;
		}

		if (ALiminalKeyItemActor* KeyItem = Cast<ALiminalKeyItemActor>(HitActor))
		{
			KeyItem->TryCollectKey(this);
			return;
		}

		if (ALootActor* Loot = Cast<ALootActor>(HitActor))
		{
			ServerTryGrab();
			return;
		}
	}
}

void AScavengerCharacter::ToggleNightVision()
{
	if (BodycamComponent)
	{
		BodycamComponent->ToggleNightVision();
	}
}

bool AScavengerCharacter::ServerDrainStamina_Validate(float Amount)
{
	return Amount >= 0.0f;
}

bool AScavengerCharacter::ServerSetSprinting_Validate(bool bNewSprinting)
{
	return true;
}

bool AScavengerCharacter::ServerUseTool_Validate()
{
	return true;
}

bool AScavengerCharacter::ServerTerminalPurchaseItem_Validate(ALiminalTerminalActor* Terminal, FName ItemId)
{
	return Terminal != nullptr && !ItemId.IsNone();
}

void AScavengerCharacter::ServerTerminalPurchaseItem_Implementation(ALiminalTerminalActor* Terminal, FName ItemId)
{
	if (!HasAuthority() || !Terminal || bIsDead || bIsDowned || bIsHypnotized)
	{
		return;
	}

	const float MaxDistSq = FMath::Square(450.0f);
	if (FVector::DistSquared(GetActorLocation(), Terminal->GetActorLocation()) > MaxDistSq)
	{
		return;
	}

	FVector EyeLoc;
	FRotator EyeRot;
	GetActorEyesViewPoint(EyeLoc, EyeRot);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TerminalPurchaseLOS), false, this);
	for (const TObjectPtr<ABaseTool>& Tool : OwnedTools)
	{
		if (Tool) Params.AddIgnoredActor(Tool);
	}
	if (UWorld* World = GetWorld())
	{
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, EyeLoc, Terminal->GetActorLocation(), ECC_Visibility, Params);
		if (bBlocked && Hit.GetActor() != Terminal)
		{
			return;
		}
	}

	Terminal->PurchaseStoreItem(ItemId, this);
}

bool AScavengerCharacter::ServerTerminalSelectBiome_Validate(ALiminalTerminalActor* Terminal, ELevelBiome Biome)
{
	return Terminal != nullptr;
}

void AScavengerCharacter::ServerTerminalSelectBiome_Implementation(ALiminalTerminalActor* Terminal, ELevelBiome Biome)
{
	if (!HasAuthority() || !Terminal || bIsDead || bIsDowned || bIsHypnotized)
	{
		return;
	}

	const float MaxDistSq = FMath::Square(450.0f);
	if (FVector::DistSquared(GetActorLocation(), Terminal->GetActorLocation()) > MaxDistSq)
	{
		return;
	}

	FVector EyeLoc;
	FRotator EyeRot;
	GetActorEyesViewPoint(EyeLoc, EyeRot);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TerminalBiomeLOS), false, this);
	for (const TObjectPtr<ABaseTool>& Tool : OwnedTools)
	{
		if (Tool) Params.AddIgnoredActor(Tool);
	}
	if (UWorld* World = GetWorld())
	{
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, EyeLoc, Terminal->GetActorLocation(), ECC_Visibility, Params);
		if (bBlocked && Hit.GetActor() != Terminal)
		{
			UE_LOG(LogTemp, Warning, TEXT("[TerminalLOS] Blocked by: %s (HitActor: %s, Terminal: %s)"),
				*GetNameSafe(Hit.GetComponent()), *GetNameSafe(Hit.GetActor()), *GetNameSafe(Terminal));
			return;
		}
	}

	Terminal->SelectBiome(Biome);
}

bool AScavengerCharacter::ServerTerminalLaunchIncursion_Validate(ALiminalTerminalActor* Terminal)
{
	return Terminal != nullptr;
}

void AScavengerCharacter::ServerTerminalLaunchIncursion_Implementation(ALiminalTerminalActor* Terminal)
{
	if (!HasAuthority() || !Terminal || bIsDead || bIsDowned || bIsHypnotized)
	{
		return;
	}

	const float MaxDistSq = FMath::Square(450.0f);
	if (FVector::DistSquared(GetActorLocation(), Terminal->GetActorLocation()) > MaxDistSq)
	{
		return;
	}

	FVector EyeLoc;
	FRotator EyeRot;
	GetActorEyesViewPoint(EyeLoc, EyeRot);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TerminalLaunchLOS), false, this);
	for (const TObjectPtr<ABaseTool>& Tool : OwnedTools)
	{
		if (Tool) Params.AddIgnoredActor(Tool);
	}
	if (UWorld* World = GetWorld())
	{
		const bool bBlocked = World->LineTraceSingleByChannel(Hit, EyeLoc, Terminal->GetActorLocation(), ECC_Visibility, Params);
		if (bBlocked && Hit.GetActor() != Terminal)
		{
			return;
		}
	}

	Terminal->LaunchIncursion();
}
