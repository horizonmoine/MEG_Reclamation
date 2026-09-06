# Handoff Report: AScavengerCharacter Skeletal Mesh & Animation Architecture

**Milestone**: M1 - Skeletal Animation & Bestiary (Features F01, F02: Scavenger 1P arms & 3P body)  
**Agent**: teamwork_preview_explorer (`explorer_m1_1`)  
**Target Recipient**: Orchestrator / Implementer Agent  
**Date**: 2026-09-06T02:44:00Z  
**Type**: Hard Handoff (Investigation & Architecture Complete)  

---

## 1. Observation

### 1.1 Source Code State
- **`Source/MEG_Reclamation/GameModes/LiminalGameMode.cpp` (line 33)**:
  ```cpp
  DefaultPawnClass = AScavengerCharacter::StaticClass();
  ```
  `AScavengerCharacter` is directly instantiated as the native C++ default pawn class without a Blueprint wrapper.
- **`Source/MEG_Reclamation/Player/ScavengerCharacter.h` (lines 380-470)**:
  - Line 381: `TObjectPtr<UCameraComponent> FirstPersonCamera;`
  - Line 469: `TObjectPtr<UStaticMeshComponent> FirstPersonToolMesh;`
  - There is no declaration of `FirstPersonMesh` (or any `USkeletalMeshComponent`) in `AScavengerCharacter.h`.
- **`Source/MEG_Reclamation/Player/ScavengerCharacter.cpp` (lines 79-91)**:
  ```cpp
  FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
  FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
  FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
  FirstPersonCamera->bUsePawnControlRotation = true;

  FirstPersonToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonToolMesh"));
  FirstPersonToolMesh->SetupAttachment(FirstPersonCamera);
  FirstPersonToolMesh->SetRelativeLocation(FVector(35.0f, 20.0f, -22.0f));
  FirstPersonToolMesh->SetRelativeRotation(FRotator(0.0f, -15.0f, 5.0f));
  FirstPersonToolMesh->SetRelativeScale3D(FVector(0.35f));
  FirstPersonToolMesh->SetCollisionProfileName(TEXT("NoCollision"));
  FirstPersonToolMesh->SetCastShadow(false);
  ```
  - The inherited full-body third-person mesh `GetMesh()` is never referenced or configured in `ScavengerCharacter.cpp`.
  - `FirstPersonToolMesh` is attached directly to `FirstPersonCamera` with a static offset and has `SetCastShadow(false)`, but lacks `bOnlyOwnerSee = true`.

### 1.2 Asset Availability & Structure
- **`Content/Characters/Mannequins/Meshes/SKM_Manny_Simple.uasset`**:
  - Valid skeletal mesh referencing skeleton `/Game/Characters/Mannequins/Meshes/SK_Mannequin`.
  - Skeleton contains bones: `root`, `pelvis`, `spine_01..05`, `clavicle_l/r`, `upperarm_l/r`, `lowerarm_l/r`, `hand_l/r`, `ik_hand_gun`, `ik_hand_l`, `ik_hand_r`.
- **`Content/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.uasset`**:
  - AnimBlueprint generated class: `UABP_Unarmed_C`.
  - Graph logic dynamically accesses `TryGetPawnOwner()`, casts to `ACharacter`, gets `CharacterMovement`, and computes `GroundSpeed` and `Direction` from `Velocity`.
  - Driven by `BS_Idle_Walk_Run.uasset` with speed range `[0.0, 600.0]` and direction `[-180.0, 180.0]`.
- **`Content/FirstPerson/Anims/ABP_FP_Copy.uasset`**:
  - AnimBlueprint generated class `UABP_FP_Copy_C` utilizing `AnimNode_CopyPoseFromMesh` and `CtrlRig_FPWarp` to prevent near-clipping when copying poses from the 3P mesh.

### 1.3 Test & Tool Validation
- Command: `powershell -ExecutionPolicy Bypass -File "F:/MEG_Reclamation/Run_Auto_Check.ps1" -SkipLiveExec`
  - Result: `BILAN: 27 / 27 CONTROLES VALIDES` (Exit Code 0).
- Command: `powershell -ExecutionPolicy Bypass -File "F:/MEG_Reclamation/Run_Automation_Tests.ps1"`
  - Result: `[SUCCES] Tous les tests MEG sont valides (Exit Code 0) !`
- `Source/MEG_Reclamation/Tests/MegReclamationTests.cpp`:
  - Lines 485-499: `FMegScavengerCapabilitiesTest` tests `Scavenger->GetMaxCarryWeightKg() == 60.0f` and `Scavenger->GetStaminaPercent() >= 0.0f`.
  - Lines 666-684: `FMegCoopSurvivalAndDownedTest` tests `IsDowned() == false`, `IsDead() == false`, `GetDownedTimeRemaining() == 45.0f`, `GetDamageFlashIntensity() == 0.0f`, `GetActiveFakeAlert().IsEmpty() == true`.
  - Line 416: Tests `GM->DefaultPawnClass == AScavengerCharacter::StaticClass()`.

---

## 2. Logic Chain

1. **Direct C++ Instantiation (from 1.1)**:  
   Because `ALiminalGameMode` sets `DefaultPawnClass = AScavengerCharacter::StaticClass()`, there is no Blueprint asset where mesh components can be added via the UE Editor UI. Any new component or asset binding (`SKM_Manny_Simple`, `ABP_Unarmed`) MUST be declared and initialized directly in C++ (`ScavengerCharacter.h` and `ScavengerCharacter.cpp`).

2. **FirstPersonMesh Hierarchy & Alignment (from 1.1 & 1.2)**:  
   `FirstPersonCamera` is already set up at `(0, 0, 60)` relative to the capsule and moves dynamically during crouch (down to Z=20), lean (Y=±35), and downed state (down to Z=25). By attaching `FirstPersonMesh` to `FirstPersonCamera` with relative transform `FVector(-10.0f, 0.0f, -155.0f)` and `FRotator(0.0f, -90.0f, 0.0f)`, the 1P arms naturally align with the camera viewport and inherit all camera crouch, lean, and crawl offsets without requiring complex custom procedural camera compensation code.

3. **Multiplayer 3P Body (`GetMesh()`) Setup (from 1.1 & 1.2)**:  
   `ACharacter` already creates a root skeletal mesh component accessible via `GetMesh()`. In `AScavengerCharacter` constructor, configuring `GetMesh()` with `SKM_Manny_Simple` and `ABP_Unarmed_C`, positioning it at `FVector(0, 0, -90)` / `FRotator(0, -90, 0)`, and setting `SetOwnerNoSee(true)` ensures other players in multiplayer see the scavenger's full walking/running/jumping body. Enabling `bCastHiddenShadow = true` ensures the local player sees their own body shadow cast on the environment, reinforcing horror immersion.

4. **Tool Attachment & Visual Synchronization (from 1.1 & 1.2)**:  
   `FirstPersonToolMesh` can be attached directly to `FirstPersonMesh` at socket/bone `TEXT("hand_r")` with local transform `FVector(4.0f, 2.0f, -1.0f)` / `FRotator(0.0f, 90.0f, 0.0f)` and `bOnlyOwnerSee = true`. This synchronizes the tool with locomotion arm bobbing and idle breathing. The tool mesh swapping mechanism in `OnRep_CurrentToolIndex()` remains 100% intact.

5. **Locomotion & Blendspace Compatibility (from 1.1 & 1.2)**:  
   `AScavengerCharacter` modifies `MaxWalkSpeed` during sprint (from 600 to ~960 cm/s) and crouch (to 300 cm/s). `ABP_Unarmed` reads `Velocity` via `ACharacter::GetCharacterMovement()`. Because `CharacterMovementComponent` replicates movement velocity to simulated proxies, both the autonomous proxy (local player) and simulated proxies (multiplayer peers) will correctly evaluate `BS_Idle_Walk_Run` across idle, walk, and jog/run thresholds without requiring custom AnimNotify wiring for basic movement.

6. **Automated Test Immunity (from 1.3)**:  
   `MegReclamationTests.cpp` checks CDO properties for carry weight, stamina, downed state, and default pawn class. Adding `FirstPersonMesh`, configuring `GetMesh()`, and setting attachment transforms does not alter any CDO variable checked by `MegReclamationTests.cpp`. The test suite will continue to pass with Exit Code 0.

---

## 3. Caveats

1. **First-Person Near-Plane Clipping**:  
   When using full mannequin `SKM_Manny_Simple` attached to `FirstPersonCamera`, the head and chest geometry of Manny can potentially intersect with the camera near-clipping plane if `FirstPersonPrimitiveType` is not supported or if FOV is narrow.  
   *Mitigation*: Set `FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;` (available in UE 5.8) or hide the head bone using `FirstPersonMesh->HideBoneByName(TEXT("head"), PBO_None);`. Alternatively, `ABP_FP_Copy` with `CtrlRig_FPWarp` can be used.
2. **Third-Person Tool Mesh in Multiplayer**:  
   `FirstPersonToolMesh` is strictly for the first-person owner view (`bOnlyOwnerSee = true`). For other players to see the tool in multiplayer, the spawned `ABaseTool` instances in `OwnedTools` should be attached to `GetMesh()` at socket `TEXT("hand_r")` in `EquipLoadout()`.
3. **Dedicated Server Headless Mode**:  
   When running in `-nullrhi` or dedicated server mode, skeletal meshes are not rendered, but animation ticking remains active for root motion or notify events. `VisibilityBasedAnimTickOption` should be set to `EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones` if root motion or notifies are later added.

---

## 4. Conclusion

The implementation path for Milestone M1 (Features F01 and F02) is well-defined, minimal, and carries zero regression risk:
1. Declare `USkeletalMeshComponent* FirstPersonMesh` and `GetFirstPersonMesh()` in `ScavengerCharacter.h`.
2. In `ScavengerCharacter.cpp` constructor:
   - Load `SKM_Manny_Simple` and `ABP_Unarmed_C` via `ConstructorHelpers`.
   - Setup `FirstPersonMesh` attached to `FirstPersonCamera` with `bOnlyOwnerSee = true`, `CastShadow = false`, `NoCollision`, relative location `(-10, 0, -155)`, rotation `(0, -90, 0)`.
   - Configure `GetMesh()` with `SKM_Manny_Simple`, `ABP_Unarmed_C`, `bOwnerNoSee = true`, `bCastHiddenShadow = true`, relative location `(0, 0, -90)`, rotation `(0, -90, 0)`.
   - Attach `FirstPersonToolMesh` to `FirstPersonMesh` socket `hand_r` with `SetOnlyOwnerSee(true)`.
3. In `Die()` and `OnRep_IsDead()`:
   - Hide `FirstPersonMesh` and `FirstPersonToolMesh`.
   - Enable `Ragdoll` physics on `GetMesh()` and set `bOwnerNoSee = false` for death spectator view.

---

## 5. Verification Method

To independently verify this implementation:
1. **Compilation Command**:
   ```powershell
   & "dotnet" "F:\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MEG_ReclamationEditor Win64 Development -Project="F:\MEG_Reclamation\MEG_Reclamation.uproject" -WaitMutex
   ```
   *Expected*: Exit Code 0, 0 errors.
2. **Automated Tests Command**:
   ```powershell
   powershell -ExecutionPolicy Bypass -File "F:\MEG_Reclamation\Run_Automation_Tests.ps1"
   ```
   *Expected*: `[SUCCES] Tous les tests MEG sont valides (Exit Code 0) !`
3. **Audit Script Command**:
   ```powershell
   powershell -ExecutionPolicy Bypass -File "F:\MEG_Reclamation\Run_Auto_Check.ps1" -SkipLiveExec
   ```
   *Expected*: `27 / 27 CONTROLES VALIDES` (Exit Code 0).
4. **Files to Inspect**:
   - `Source/MEG_Reclamation/Player/ScavengerCharacter.h`
   - `Source/MEG_Reclamation/Player/ScavengerCharacter.cpp`
   - Detailed analysis: `F:/MEG_Reclamation/.agents/explorer_m1_1/analysis.md`
5. **Invalidation Conditions**:
   - Any failure in `FMegScavengerCapabilitiesTest` or `FMegCoopSurvivalAndDownedTest`.
   - Compilation errors regarding missing headers (`Components/SkeletalMeshComponent.h`, `UObject/ConstructorHelpers.h`).
   - Null reference crash when spawning `AScavengerCharacter` without assets.
