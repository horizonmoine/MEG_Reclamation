# Technical Analysis: AScavengerCharacter Skeletal Mesh & Animation Architecture
**Milestone**: M1 - Skeletal Animation & Bestiary (Features F01, F02: Scavenger 1P arms & 3P body)  
**Agent**: teamwork_preview_explorer (explorer_m1_1)  
**Date**: 2026-09-06  

---

## 1. Executive Summary
This analysis formulates the complete, production-ready C++ implementation plan for `AScavengerCharacter` in **M.E.G. : Reclamation** (Unreal Engine 5.8).  
Currently, `AScavengerCharacter` is instantiated directly as the `DefaultPawnClass` by `ALiminalGameMode`, but possesses no first-person skeletal mesh component (`FirstPersonMesh`), while its inherited third-person mesh (`GetMesh()`) is unconfigured (null mesh and default ungrounded transform).  
Existing mannequins (`SKM_Manny_Simple`), blendspaces (`BS_Idle_Walk_Run`), and animation blueprints (`ABP_Unarmed` and `ABP_FP_Copy`) are already fully functional in `Content/Characters/Mannequins/` and `Content/FirstPerson/`.  
The proposed C++ architecture integrates a dedicated 1P arms mesh and a 3P multiplayer body without altering any gameplay properties tested by `MegReclamationTests.cpp`, preserving 100% test compatibility.

---

## 2. Codebase & Asset Audit

### 2.1 Inspection of `ScavengerCharacter.h` & `ScavengerCharacter.cpp`
- **Class Hierarchy**: Inherits from `ACharacter` (`GameFramework/Character.h`).
- **Instantiation**: Configured directly in `Source/MEG_Reclamation/GameModes/LiminalGameMode.cpp`:
  ```cpp
  DefaultPawnClass = AScavengerCharacter::StaticClass();
  ```
  *Key Insight*: There is no Blueprint wrapper class (`BP_ScavengerCharacter`). All components, asset references, and initial transforms MUST be configured in C++ (`AScavengerCharacter::AScavengerCharacter()`).
- **Camera Configuration**:
  - `FirstPersonCamera` is created as `UCameraComponent`, attached to `GetCapsuleComponent()` at `(0.0f, 0.0f, 60.0f)` with `bUsePawnControlRotation = true`.
  - In `Tick()`, camera relative location transitions dynamically:
    - Crouch: interpolates between `StandingCameraZ` (60.0f) and `CrouchedCameraZ` (20.0f).
    - Lean: applies lateral offset along Y-axis (`MaxLeanOffset = 35.0f`).
    - Downed: drops to `(0.0f, 0.0f, 25.0f)`.
- **Current Tool Mesh**:
  - `FirstPersonToolMesh` is a `UStaticMeshComponent` attached to `FirstPersonCamera` at `FVector(35.0f, 20.0f, -22.0f)`, rotation `FRotator(0.0f, -15.0f, 5.0f)`, scale `0.35f`.
  - Collision profile: `NoCollision`, `CastShadow = false`.
  - Mesh asset updated dynamically in `OnRep_CurrentToolIndex()` based on active `ABaseTool`.
- **Missing Architecture**:
  - No `FirstPersonMesh` (1P arms) declared in `ScavengerCharacter.h`.
  - Inherited `GetMesh()` (3P full body) is neither assigned a skeletal mesh nor an animation blueprint in `ScavengerCharacter.cpp`.

### 2.2 Asset Verification in `Content/Characters/Mannequins/` & `Content/FirstPerson/`
1. **`SKM_Manny_Simple.uasset`**:
   - Location: `/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple`
   - Skeleton: `/Game/Characters/Mannequins/Meshes/SK_Mannequin`
   - Key bones: `root`, `pelvis`, `spine_01..05`, `neck_01..02`, `head`, `clavicle_l/r`, `upperarm_l/r`, `lowerarm_l/r`, `hand_l/r`, `ik_hand_gun`, `ik_hand_l`, `ik_hand_r`, `ik_hand_root`.
   - Attachment sockets: Any bone name (`hand_r`, `ik_hand_gun`, etc.) can be used directly as an attachment socket in Unreal Engine.
2. **`ABP_Unarmed.uasset`**:
   - Location: `/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C`
   - Logic: Casts `TryGetPawnOwner()` to `ACharacter`, accesses `GetCharacterMovement()`, extracts `Velocity` to compute `GroundSpeed` and `Direction`.
   - State Machine: Features states for Idle/Walk/Run via `BS_Idle_Walk_Run`, jumping transitions (`MM_Jump`, `MM_Fall_Loop`, `MM_Land`), and idle poses.
3. **`BS_Idle_Walk_Run.uasset`**:
   - Location: `/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run.BS_Idle_Walk_Run`
   - 2D BlendSpace with axes: `Direction` [-180, 180] and `Speed` [0, 600].
   - Contains blend samples for 8-directional walking and jogging.
4. **`ABP_FP_Copy.uasset`**:
   - Location: `/Game/FirstPerson/Anims/ABP_FP_Copy.ABP_FP_Copy_C`
   - Logic: Implements `AnimNode_CopyPoseFromMesh` referencing `SourceMeshComponent` (the 3P mesh) and applies `CtrlRig_FPWarp` to prevent 1P camera near-plane clipping.

---

## 3. Detailed C++ Specification

### 3.1 Feature F01: FirstPersonMesh (1P Arms)
- **Class**: `USkeletalMeshComponent`
- **Header Declaration (`Source/MEG_Reclamation/Player/ScavengerCharacter.h`)**:
  ```cpp
  // Add in includes if needed: class USkeletalMeshComponent;
  
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Mesh", meta = (AllowPrivateAccess = "true"))
  TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

  UFUNCTION(BlueprintPure, Category = "Scavenger|Mesh")
  USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
  ```
- **Constructor Implementation (`Source/MEG_Reclamation/Player/ScavengerCharacter.cpp`)**:
  ```cpp
  FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
  FirstPersonMesh->SetupAttachment(FirstPersonCamera);
  FirstPersonMesh->SetOnlyOwnerSee(true);
  FirstPersonMesh->SetCastShadow(false);
  FirstPersonMesh->bCastHiddenShadow = false;
  FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));
  FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;

  // Relative Transform: Aligns Manny mannequin shoulder/eye height to FirstPersonCamera
  FirstPersonMesh->SetRelativeLocation(FVector(-10.0f, 0.0f, -155.0f));
  FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

  // Asset Binding
  static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshFinder(
      TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
  if (MannyMeshFinder.Succeeded())
  {
      FirstPersonMesh->SetSkeletalMesh(MannyMeshFinder.Object);
  }

  static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimBPFinder(
      TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));
  if (UnarmedAnimBPFinder.Succeeded())
  {
      FirstPersonMesh->SetAnimInstanceClass(UnarmedAnimBPFinder.Class);
  }
  ```

### 3.2 Feature F02: GetMesh() (3P Full Body)
- **Component**: Inherited `Mesh` from `ACharacter`, accessed via `GetMesh()`.
- **Constructor Implementation (`Source/MEG_Reclamation/Player/ScavengerCharacter.cpp`)**:
  ```cpp
  if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
  {
      ThirdPersonMesh->SetOwnerNoSee(true);
      ThirdPersonMesh->SetCastShadow(true);
      ThirdPersonMesh->bCastHiddenShadow = true; // Crucial for horror: player sees their own shadow cast on walls and floors
      ThirdPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
      ThirdPersonMesh->SetCollisionProfileName(TEXT("CharacterMesh"));

      // Grounding transform: Manny origin at feet, facing +X (Yaw = -90)
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
  ```

### 3.3 FirstPersonToolMesh Attachment & Synchronization
Two viable architectures exist for attaching `FirstPersonToolMesh`:

#### Architecture A: Socket Attachment to `FirstPersonMesh` (Recommended for AAA Game Feel)
- **Attachment**: `FirstPersonToolMesh->SetupAttachment(FirstPersonMesh, TEXT("hand_r"));`
- **Socket Transform**:
  - `RelativeLocation`: `FVector(4.0f, 2.0f, -1.0f)`
  - `RelativeRotation`: `FRotator(0.0f, 90.0f, 0.0f)`
  - `RelativeScale3D`: `FVector(0.35f)`
- **Behavior**:
  - The tool naturally moves, breathes, and sways with the 1P arms during locomotion blendspace execution.
  - When the player stops or sprints, the tool motion matches the hand grip realistically.
- **Replication**: In `EquipLoadout()`, spawned `ABaseTool` actors for 3P can simultaneously be attached to `GetMesh()` socket `TEXT("hand_r")`, granting complete visual synchronization in multiplayer lobbies.

#### Architecture B: Camera Attachment (Baseline Fallback)
- **Attachment**: `FirstPersonToolMesh->SetupAttachment(FirstPersonCamera);`
- **Camera Transform**:
  - `RelativeLocation`: `FVector(35.0f, 20.0f, -22.0f)`
  - `RelativeRotation`: `FRotator(0.0f, -15.0f, 5.0f)`
  - `RelativeScale3D`: `FVector(0.35f)`
- **Behavior**: Tool remains locked to camera viewport regardless of arms animation pose.
- **Recommendation**: Provide clean socket attachment to `FirstPersonMesh` as default, with fallback to `FirstPersonCamera` if `FirstPersonMesh` is unassigned.

---

## 4. Systems Compatibility Matrix

| Gameplay System | Mechanism | Compatibility Analysis |
|---|---|---|
| **Sprinting & Stamina** | `bIsSprinting` increases `MaxWalkSpeed` from 600 to 960 cm/s; drains stamina. | **100% Compatible**. `ABP_Unarmed` dynamically reads velocity from `CharacterMovementComponent`. When sprinting, `GroundSpeed` smoothly moves from Walk (200-300) into Run (600+), driving `BS_Idle_Walk_Run` jog/run samples. |
| **Tool Holding & Cycling** | `CurrentToolIndex` replication updates `FirstPersonToolMesh->SetStaticMesh(DesiredMesh)`. | **100% Compatible**. Socket attachment to `FirstPersonMesh` (`hand_r`) retains full mesh swapping and visibility logic in `OnRep_CurrentToolIndex()`. |
| **Crouch & Lean** | `StartCrouch()` calls `ACharacter::Crouch()`. Camera interpolates Z from 60.0f to 20.0f; Lean shifts camera in Y. | **100% Compatible**. Because `FirstPersonMesh` is attached to `FirstPersonCamera`, 1P arms naturally drop and tilt with the camera. 3P `GetMesh()` is adjusted by `ACharacter::Crouch()` to maintain capsule alignment. |
| **Downed State** | `EnterDownedState()` drops camera Z to 25.0f and sets speed to 80.0f. | **100% Compatible**. 1P arms descend with camera. Crawling speed (80 cm/s) drives slow crawl in `BS_Idle_Walk_Run`. |
| **Death System** | `Die()` and `OnRep_IsDead()` disable capsule collision and movement. | **Enhanced Compatibility**. On death: `FirstPersonMesh->SetVisibility(false)` hides arms; `GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"))` and `SetSimulatePhysics(true)` trigger ragdoll for 3P; `GetMesh()->SetOwnerNoSee(false)` lets dead player observe their corpse in spectator mode. |
| **MegReclamationTests.cpp** | Validates CDO defaults (`MaxCarryWeightKg == 60`, `StaminaPercent >= 0`, `IsDowned == false`, `IsDead == false`, `DownedTimeRemaining == 45`, `DamageFlashIntensity == 0`, `ActiveFakeAlert.IsEmpty() == true`, `DefaultPawnClass == AScavengerCharacter`). | **100% Compatible**. None of the CDO checks inspect skeletal mesh pointers or modify tested floats/booleans. All tests will pass cleanly. |

---

## 5. Proposed Concrete Code Modifications

### 5.1 `Source/MEG_Reclamation/Player/ScavengerCharacter.h`
```cpp
// --- In includes / forward declarations ---
class USkeletalMeshComponent;

// --- Under public section (around line 188) ---
	UFUNCTION(BlueprintPure, Category = "Scavenger|Mesh")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

// --- Under protected properties (around line 382) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
```

### 5.2 `Source/MEG_Reclamation/Player/ScavengerCharacter.cpp`
```cpp
// --- In includes ---
#include "Components/SkeletalMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// --- In constructor AScavengerCharacter::AScavengerCharacter() ---
	// 1. Asset finders for Mannequin mesh and Unarmed AnimBP
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> UnarmedAnimBPFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C"));

	// 2. Configure 3P full body (GetMesh())
	if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
	{
		ThirdPersonMesh->SetOwnerNoSee(true);
		ThirdPersonMesh->SetCastShadow(true);
		ThirdPersonMesh->bCastHiddenShadow = true;
		ThirdPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
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

	// 3. Configure 1P arms (FirstPersonMesh)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetCastShadow(false);
	FirstPersonMesh->bCastHiddenShadow = false;
	FirstPersonMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
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

	// 4. Attach FirstPersonToolMesh to FirstPersonMesh socket hand_r
	FirstPersonToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonToolMesh"));
	FirstPersonToolMesh->SetupAttachment(FirstPersonMesh, TEXT("hand_r"));
	FirstPersonToolMesh->SetRelativeLocation(FVector(4.0f, 2.0f, -1.0f));
	FirstPersonToolMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	FirstPersonToolMesh->SetRelativeScale3D(FVector(0.35f));
	FirstPersonToolMesh->SetCollisionProfileName(TEXT("NoCollision"));
	FirstPersonToolMesh->SetCastShadow(false);
	FirstPersonToolMesh->SetOnlyOwnerSee(true);
```

### 5.3 Death Ragdoll Handling in `OnRep_IsDead()` & `Die()`
```cpp
// In Die() and OnRep_IsDead():
if (bIsDead)
{
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
        ThirdPersonMesh->SetCollisionProfileName(TEXT("Ragdoll"));
        ThirdPersonMesh->SetSimulatePhysics(true);
    }
}
```

---

## 6. Independent Verification Plan
1. **Compilation Check**:
   - Execute `UnrealBuildTool` Win64 Development:
     `dotnet "F:\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MEG_ReclamationEditor Win64 Development -Project="F:\MEG_Reclamation\MEG_Reclamation.uproject" -WaitMutex`
2. **Automated Test Suite**:
   - Execute `Run_Automation_Tests.ps1`.
   - Verify `FMegScavengerCapabilitiesTest` and `FMegCoopSurvivalAndDownedTest` execute with Exit Code 0.
3. **Audit Script**:
   - Run `Run_Auto_Check.ps1 -SkipLiveExec` (27/27 OK).
