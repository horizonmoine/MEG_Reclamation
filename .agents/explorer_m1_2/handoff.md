# Handoff Report: LiminalEntity & AnimNotify Damage Architecture

**Milestone**: M1 - Skeletal Animation & Bestiary (Features F03, F05, F06)  
**Agent**: `teamwork_preview_explorer` (`explorer_m1_2`)  
**Target Path**: `F:/MEG_Reclamation/.agents/explorer_m1_2/handoff.md`  
**Date**: 2026-09-06  
**Handoff Type**: Hard (Complete Investigation & Architectural Plan)

---

## 1. Observation

1. **`ALiminalEntity` Component Topology** (`Source/MEG_Reclamation/AI/LiminalEntity.h:100-104`, `LiminalEntity.cpp:35-43`):
   ```cpp
   // LiminalEntity.h:100
   UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity", meta = (AllowPrivateAccess = "true"))
   TObjectPtr<UStaticMeshComponent> BodyMesh;
   UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity")
   TSoftObjectPtr<UStaticMesh> DefaultBodyMesh;

   // LiminalEntity.cpp:35-42
   BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
   BodyMesh->SetupAttachment(GetCapsuleComponent());
   BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   DefaultBodyMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Hound/SM_Hound.SM_Hound")));
   BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
   BodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
   BodyMesh->SetRelativeScale3D(FVector(0.9f));
   ```
   `ALiminalEntity` inherits from `ACharacter` (`LiminalEntity.h:35`), which inherently creates `Mesh` (`USkeletalMeshComponent`) accessible via `GetMesh()`. Currently, `Mesh` is neither initialized nor configured with a skeletal asset or animation class in `ALiminalEntity`.

2. **Subclass Manipulations of `BodyMesh`**:
   - `LiminalEntity_Clump.cpp:52`: `BodyMesh->SetRelativeScale3D(FVector(1.8f, 1.8f, 0.35f)); BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));`
   - `LiminalEntity_Deathmoth.cpp:55`: `BodyMesh->SetRelativeScale3D(FVector(1.3f, 1.6f, 0.6f));`
   - `LiminalEntity_Duller.cpp:40`: `BodyMesh->SetRelativeScale3D(FVector(1.1f, 1.1f, 1.0f));` and `LiminalEntity_Duller.cpp:104`: `BodyMesh->SetVisibility(bIsRevealed, true);`
   - `LiminalEntity_Jerry.cpp:45`: `BodyMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.45f));`
   - `LiminalEntity_Partygoer.cpp:53`: `BodyMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.05f));`
   - `LiminalEntity_Smiler.cpp:51`: `BodyMesh->SetRelativeScale3D(FVector(0.01f));`
   - `LiminalEntity_Watcher.cpp:56`: `BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));`
   - `LiminalEntity_Wretch.cpp:44`: `BodyMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 1.15f));`

3. **Current Melee Attack Mechanics** (`Source/MEG_Reclamation/AI/LiminalEntity.cpp:167-192`):
   ```cpp
   bool ALiminalEntity::PerformMeleeAttack(AActor* Target)
   {
       if (!CanAttack() || !Target || !HasAuthority()) return false;
       const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
       if (DistSq > FMath::Square(AttackRange + 50.0f)) return false;
       AttackCooldownTimer = AttackCooldownSeconds;
       UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
       UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, this);
       if (AttackSound) UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetActorLocation(), 1.0f, FMath::FRandRange(0.85f, 1.15f));
       return true;
   }
   ```
   Melee damage is currently an instantaneous sphere-less distance calculation without bone socket tracking or animation notify triggers.

4. **Player Damage Reception & Game Feel** (`Source/MEG_Reclamation/Player/ScavengerCharacter.cpp:1075-1101, 1221-1234`):
   ```cpp
   // Line 1083:
   const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
   CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.0f);
   ClientOnDamaged(GetHealthPercent());

   // Line 1221:
   void AScavengerCharacter::ClientOnDamaged_Implementation(float HealthPercent)
   {
       DamageFlashIntensity = 1.0f;
       AddControllerPitchInput(FMath::FRandRange(-2.5f, -4.5f));
       AddControllerYawInput(FMath::FRandRange(-3.5f, 3.5f));
       if (FirstPersonCamera)
       {
           const FVector PunchOffset = FVector(-12.0f, FMath::FRandRange(-8.0f, 8.0f), FMath::FRandRange(-6.0f, 6.0f));
           FirstPersonCamera->SetRelativeLocation(FirstPersonCamera->GetRelativeLocation() + PunchOffset);
       }
   }
   ```
   `AScavengerCharacter` already processes damage events via `TakeDamage`, triggering controller pitch/yaw impulse, camera punch offset, and red vignette decay.

5. **Existing Automation Test Invariants** (`Source/MEG_Reclamation/Tests/MegReclamationTests.cpp:453-483, 557-579`):
   - `FMegSmilerSensoryTest`: Verifies `GetDefault<ALiminalEntity_Smiler>()`.
   - `FMegJerryHypnosisGazeTest`: Verifies `GetDefault<ALiminalEntity_Jerry>()`.
   - `FMegTacticalPolishAndSpectatorTest`: Verifies `GetDefault<ALiminalEntity>()`.
   Any regression in CDO instantiation, subobject initialization order, or naming breaks these tests.

---

## 2. Logic Chain

1. **Mesh Backwards-Compatibility Reasoning** (from Observation 1, 2, 5):
   - Simply removing `BodyMesh` would break all 9 subclass constructors that call `if (BodyMesh) BodyMesh->SetRelativeScale3D(...)`.
   - Furthermore, `MegReclamationTests.cpp` instantiates the CDO of `ALiminalEntity` and subclasses; altering subobject names or deleting properties would cause serialization discrepancies.
   - Therefore, `ALiminalEntity` must retain `BodyMesh` (`UStaticMeshComponent`), but configure `GetMesh()` (`USkeletalMeshComponent`) as the primary visual component.
   - In `BeginPlay()`, if `DefaultSkeletalMesh` is set, `GetMesh()` is activated and `BodyMesh` is hidden (`SetVisibility(false)`). If not, `BodyMesh` remains the active fallback.
   - Adding `SetEntityVisualScale()` and `SetEntityVisualVisibility()` allows subclasses like `ALiminalEntity_Duller` to toggle cloaking on both meshes cleanly.

2. **AnimNotify Collision Sweep Reasoning** (from Observation 1, 3):
   - To link animations to damage, an AnimNotify (`UAnimNotify_LiminalAttackTrace`) must be authored in C++.
   - When placed on an attack montage (e.g. at 45% through the claw strike or bite), `UAnimNotify_LiminalAttackTrace::Notify()` executes.
   - Server-authoritative check: If `!Owner->HasAuthority()`, exit immediately to prevent client-side cheat injection or desynchronization.
   - The notify invokes `ALiminalEntity::OnAttackNotify(SocketName, TraceRadius, TraceDistance, ...)`.
   - The trace executes `World->SweepMultiByChannel(OutHits, StartLoc, EndLoc, FQuat::Identity, ECC_Pawn, Sphere, QueryParams)`.
   - `StartLoc` is resolved from `GetMesh()->GetSocketLocation(SocketName)` (e.g. `AttackSocket` / `hand_rSocket` / `jawSocket`). If the socket does not exist or mesh is unrigged, fallback to `GetActorLocation() + FVector(0, 0, 40) + GetActorForwardVector() * Distance`.
   - `TSet<AActor*> DamagedActors` deduplicates hits so multi-component pawns (capsule + mesh) are damaged exactly once per sweep.

3. **Damage Delivery & Game Feel Reasoning** (from Observation 4):
   - Rather than untargeted `ApplyDamage`, invoking `UGameplayStatics::ApplyPointDamage()` provides hit normal, hit bone, and impact location inside `FPointDamageEvent`.
   - `AScavengerCharacter::TakeDamage()` handles this event, triggering `ClientOnDamaged()` RPC.
   - First-person view receives pitch/yaw kick, camera punch offset, and red vignette decay.
   - Adding `UPROPERTY(EditDefaultsOnly) TSubclassOf<UCameraShakeBase> DamageCameraShakeClass;` allows client camera shake invocation via `PC->ClientStartCameraShake()`.
   - Third-person view triggers `HitReactMontage` on `GetMesh()` for multiplayer teammates.
   - Audio feedback: An entity-specific `AttackImpactSound` plays at `Hit.ImpactPoint`, and `UAISense_Hearing::ReportNoiseEvent` alerts surrounding AI.

4. **Headless Automation Fallback Reasoning** (from Observation 3, 5):
   - In automated test environments running headless (`UnrealEditor-Cmd.exe ... -nullrhi -unattended`), skeletal mesh evaluation and AnimMontage ticks may be skipped or return 0 duration.
   - In `ALiminalEntity::PerformMeleeAttack(AActor* Target)`: If `PlayAnimMontage(AttackMontage) <= 0.0f` (no montage or failed play), immediately call `OnAttackNotify(DefaultAttackSocket, DefaultAttackTraceRadius, AttackRange)`.
   - This ensures 100% deterministic test execution under all conditions.

---

## 3. Caveats

1. **Skeletal Assets vs. Placeholders**: In the current repository, `SKM_Manny_Simple.uasset` and humanoid animations exist under `Content/Characters/Mannequins/`, while custom monster rigs (e.g. multi-legged Hound or insectoid Deathmoth) may require bespoke skeletal meshes or socket mapping in Persona. The C++ fallback to actor forward vector guarantees safe execution regardless of whether custom sockets exist.
2. **Hit React Montage Availability**: While `MM_HitReact_Front_Lgt_01.uasset` and `MM_HitReact_Front_Med_01.uasset` are present in `Content/Characters/Mannequins/Anims/Rifle/HitReact/`, an unarmed-specific hit reaction montage can be authored or retargeted for the Scavenger.
3. **Read-Only Investigation Mode**: Per instructions, no source files were directly modified in this phase. The full implementation plan is ready for the coder agent.

---

## 4. Conclusion

The architectural migration from static mesh to skeletal mesh with AnimNotify damage sweeps is completely formulated and backwards-compatible:
1. `ALiminalEntity` gains `GetMesh()` (`USkeletalMeshComponent`) initialization alongside legacy `BodyMesh` retention.
2. `UAnimNotify_LiminalAttackTrace` executes server-authoritative `SweepMultiByChannel` along `ECC_Pawn` from socket `AttackSocket`.
3. `ApplyPointDamage` connects to `AScavengerCharacter`'s existing camera recoil, damage flash, optional camera shake, and impact audio.
4. Seamless fallback in `PerformMeleeAttack` prevents any test hangs or regressions in `MegReclamationTests.cpp`.

---

## 5. Verification Method

### Step 1: Code Compilation
Execute the Win64 Development Editor compilation:
```powershell
& "F:\UE_5.8\Engine\Build\BatchFiles\Build.bat" MEG_ReclamationEditor Win64 Development "F:\MEG_Reclamation\MEG_Reclamation.uproject" -waitmutex
```
*Expected Result*: Exit Code 0, zero compilation errors.

### Step 2: Automation Test Suite
Execute the full 30-test automated suite:
```powershell
& "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "F:\MEG_Reclamation\MEG_Reclamation.uproject" -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty" -log
```
*Expected Result*: 100% pass on all tests including `FMegTacticalPolishAndSpectatorTest`, `FMegSmilerSensoryTest`, and `FMegJerryHypnosisGazeTest`.

### Step 3: Auto-Check Audit
Execute the integrity verification script:
```powershell
powershell -File "F:\MEG_Reclamation\Run_Auto_Check.ps1" -SkipLiveExec
```
*Expected Result*: 27/27 checks passed.

### Invalidation Conditions
- If `GetDefault<ALiminalEntity>()` or any subclass CDO crashes during engine initialization.
- If `SweepMultiByChannel` fails to detect `AScavengerCharacter` due to collision channel mismatch.
- If an entity without a skeletal mesh fails to inflict damage due to missing animation tick in headless mode.
