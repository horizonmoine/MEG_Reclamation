# Architectural Analysis: LiminalEntity Skeletal Mesh Migration & AnimNotify Damage Sweeps (UE 5.8)

**Milestone**: M1 - Skeletal Animation & Bestiary (Features F03, F05, F06)  
**Agent**: `teamwork_preview_explorer` (`explorer_m1_2`)  
**Date**: 2026-09-06  
**Status**: Completed Analysis & Proposal

---

## 1. Executive Summary

This document presents the complete C++ architectural specification for migrating hostile liminal entities (`ALiminalEntity` and its 9 specialized subclasses) from static mesh representation (`BodyMesh`) to a fully animated skeletal mesh pipeline (`GetMesh()` / `USkeletalMeshComponent`), synchronized with physical melee damage collision sweeps via a custom Unreal Engine 5.8 AnimNotify (`UAnimNotify_LiminalAttackTrace`).

The proposed architecture guarantees:
1. **100% Backwards-Compatibility**: Retains `BodyMesh` (`UStaticMeshComponent`) as a fallback subobject to ensure zero regressions in existing tests (`FMegTacticalPolishAndSpectatorTest`, `FMegSmilerSensoryTest`, `FMegJerryHypnosisGazeTest`) and preserve subclass stability.
2. **Deterministic Server-Authoritative Combat**: Melee collision sweeps (`SweepMultiByChannel` along `ECC_Pawn`) are executed server-side at the exact strike apex of animation montages, with deduplication across multiple bones/capsules.
3. **Multi-Layered Game Feel Feedback**: Delivers oriented point damage (`UGameplayStatics::ApplyPointDamage`), triggers first-person camera punch and recoil, optional camera shake (`UCameraShakeBase`), third-person hit reaction montages, dynamic blood/damage vignette flash, and spatialized impact audio cues.
4. **Headless & Automation Safety**: A robust fallback mechanism guarantees that if animation montages cannot evaluate (e.g. during headless automation tests running with `-nullrhi`), attack damage is resolved instantly without hanging test queues.

---

## 2. Current Codebase Inspection & Problem Statement

### 2.1 Current `ALiminalEntity` Implementation
- **Files**: `Source/MEG_Reclamation/AI/LiminalEntity.h` (126 lines) and `LiminalEntity.cpp` (226 lines).
- **Class Hierarchy**: `ALiminalEntity : public ACharacter`.
- **Mesh Setup**:
  - `BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));` (`LiminalEntity.cpp:35-43`).
  - Attached to `GetCapsuleComponent()`, collision set to `NoCollision`.
  - Assigned `DefaultBodyMesh` (`/Game/Meshes/Hound/SM_Hound.SM_Hound`) in `BeginPlay()` (`LiminalEntity.cpp:79-86`).
  - `ACharacter`'s native `GetMesh()` (`USkeletalMeshComponent`) is currently unconfigured and dormant.
- **Attack Mechanics**:
  - `PerformMeleeAttack(AActor* Target)` (`LiminalEntity.cpp:167-192`):
    - Instant distance check (`DistSq <= FMath::Square(AttackRange + 50.0f)`).
    - Immediate `UGameplayStatics::ApplyDamage(Target, AttackDamage, ...)`.
    - Immediate `UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, ...)`.
    - No physical collision sweep, no animation synchronization, no socket origin.

### 2.2 Subclasses & Scale Customizations
The 9 entity subclasses (`Clump`, `Deathmoth`, `Duller`, `Jerry`, `Partygoer`, `Skinwalker`, `Smiler`, `Watcher`, `Wretch`) customize `BodyMesh`:
- `LiminalEntity_Clump.cpp:52`: `BodyMesh->SetRelativeScale3D(FVector(1.8f, 1.8f, 0.35f)); BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));`
- `LiminalEntity_Deathmoth.cpp:55`: `BodyMesh->SetRelativeScale3D(FVector(1.3f, 1.6f, 0.6f));`
- `LiminalEntity_Duller.cpp:40`: `BodyMesh->SetRelativeScale3D(FVector(1.1f, 1.1f, 1.0f));` and line 104 `BodyMesh->SetVisibility(bIsRevealed, true);`
- `LiminalEntity_Jerry.cpp:45`: `BodyMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.45f));`
- `LiminalEntity_Partygoer.cpp:53`: `BodyMesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 1.05f));`
- `LiminalEntity_Smiler.cpp:51`: `BodyMesh->SetRelativeScale3D(FVector(0.01f));` (Smiler body is scaled near zero; only glowing eyes/mouth visible)
- `LiminalEntity_Watcher.cpp:56`: `BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));`
- `LiminalEntity_Wretch.cpp:44`: `BodyMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 1.15f));`

### 2.3 Existing Test Constraints (`MegReclamationTests.cpp`)
Existing tests in `Source/MEG_Reclamation/Tests/MegReclamationTests.cpp`:
- `FMegSmilerSensoryTest` (lines 453-467): Validates `GetDefault<ALiminalEntity_Smiler>()`.
- `FMegJerryHypnosisGazeTest` (lines 469-483): Validates `GetDefault<ALiminalEntity_Jerry>()`.
- `FMegTacticalPolishAndSpectatorTest` (lines 557-579): Validates `GetDefault<ALiminalEntity>()` (lines 575-576).
- `FMegScavengerCapabilitiesTest` (lines 485-499): Validates `GetDefault<AScavengerCharacter>()`.

Any changes to constructors, subobjects, or property types MUST NOT invalidate CDO initialization or cause null pointer dereferences.

---

## 3. Skeletal Mesh Migration Architecture (Feature F03)

### 3.1 Dual-Mesh Backwards-Compatible Model
To satisfy Feature F03 while avoiding breaking changes:
1. **Primary Mesh**: `GetMesh()` (`USkeletalMeshComponent`, inherited from `ACharacter`).
   - Sockets: `AttackSocket` (default), `hand_rSocket`, `jawSocket`, `headSocket`.
   - Transform: Relative Location `(0.0, 0.0, -90.0)`, Relative Rotation `(0.0, -90.0, 0.0)`.
   - Collision: `CharacterMesh` profile, `ECollisionEnabled::NoCollision` or `QueryOnly`.
2. **Fallback Mesh**: `BodyMesh` (`UStaticMeshComponent`).
   - Retained as a subobject in `ALiminalEntity`.
   - Kept invisible when a valid `DefaultSkeletalMesh` is loaded.
   - If no skeletal mesh is provided, `BodyMesh` continues to display `DefaultBodyMesh`.
3. **Unified Scale / Visibility API**:
   - `void SetEntityVisualScale(const FVector& Scale3D)`: Applies scale simultaneously to both `BodyMesh` and `GetMesh()`.
   - `void SetEntityVisualVisibility(bool bVisible)`: Toggles visibility on both components, fixing cloaking (e.g. `ALiminalEntity_Duller`).

### 3.2 Property Schema Additions in `ALiminalEntity`
```cpp
// --- Skeletal Mesh & Animation Pipeline ---
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Mesh")
TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
TSubclassOf<UAnimInstance> DefaultAnimClass;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
TObjectPtr<UAnimMontage> AttackMontage;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
TObjectPtr<UAnimMontage> HitReactMontage;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
TObjectPtr<UAnimMontage> DeathMontage;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Combat")
FName DefaultAttackSocket = FName(TEXT("AttackSocket"));

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "5.0"))
float DefaultAttackTraceRadius = 45.0f;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "10.0"))
float DefaultAttackTraceDistance = 90.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Audio")
TObjectPtr<USoundBase> AttackImpactSound;
```

---

## 4. AnimNotify Collision Sweep Architecture (Feature F05)

### 4.1 Custom AnimNotify Class: `UAnimNotify_LiminalAttackTrace`
Located in `Source/MEG_Reclamation/AI/AnimNotify_LiminalAttackTrace.h` and `.cpp`.

#### Header Design (`AnimNotify_LiminalAttackTrace.h`):
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_LiminalAttackTrace.generated.h"

/**
 * AnimNotify de balayage d'attaque physique pour entites liminales.
 * Déclenché depuis Persona/Montage au point culminant du coup (griffe, morsure, tentacule).
 * Execute une trace de sphere (SweepMultiByChannel) sur ECC_Pawn.
 */
UCLASS(meta = (DisplayName = "Liminal Entity Attack Trace"))
class MEG_RECLAMATION_API UAnimNotify_LiminalAttackTrace : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_LiminalAttackTrace();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	/** Socket osseux d'origine du coup sur le Skeletal Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	FName SocketName = FName(TEXT("AttackSocket"));

	/** Rayon de la sphere de balayage (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (ClampMin = "5.0", ClampMax = "200.0"))
	float TraceRadius = 45.0f;

	/** Distance projetee vers l'avant (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (ClampMin = "10.0", ClampMax = "400.0"))
	float TraceDistance = 90.0f;

	/** Forcer des dégâts spécifiques au lieu des dégâts de base de l'entité */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	bool bOverrideDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (EditCondition = "bOverrideDamage", ClampMin = "0.0"))
	float CustomDamage = 35.0f;

	/** Classe de DamageType optionnelle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	TSubclassOf<UDamageType> DamageTypeClass;

	/** Afficher la sphere de trace dans l'editeur / debug */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace|Debug")
	bool bDrawDebugTrace = false;
};
```

#### Notify Execution Flow:
```
[AnimMontage Execution]
       │
       ▼
[UAnimNotify_LiminalAttackTrace::Notify()]
       │
       ├─► Check MeshComp && MeshComp->GetOwner()
       ├─► Check Owner->HasAuthority() (Server-authoritative only)
       └─► Cast<ALiminalEntity>(Owner)->OnAttackNotify(SocketName, TraceRadius, TraceDistance, ...)
              │
              ▼
[ALiminalEntity::OnAttackNotify()]
       │
       ├─► Resolve Socket Location & Orientation (or forward fallback)
       ├─► World->SweepMultiByChannel(OutHits, Start, End, ECC_Pawn, Sphere, QueryParams)
       ├─► Deduplicate Hits via TSet<AActor*> DamagedActors
       ├─► For each valid hit AScavengerCharacter:
       │     ├─► UGameplayStatics::ApplyPointDamage(...)
       │     ├─► Play AttackImpactSound at Hit.ImpactPoint
       │     └─► UAISense_Hearing::ReportNoiseEvent(...)
       └─► Return Number of Targets Hit
```

### 4.2 C++ Collision Sweep Logic in `ALiminalEntity::OnAttackNotify`
```cpp
int32 ALiminalEntity::OnAttackNotify(
	FName SocketName,
	float Radius,
	float ForwardDistance,
	float DamageOverride,
	TSubclassOf<UDamageType> DamageType,
	bool bDrawDebug)
{
	if (!HasAuthority())
	{
		return 0;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	const FName TargetSocket = (SocketName != NAME_None) ? SocketName : DefaultAttackSocket;
	const float ActualRadius = (Radius > 0.0f) ? Radius : DefaultAttackTraceRadius;
	const float ActualDistance = (ForwardDistance > 0.0f) ? ForwardDistance : DefaultAttackTraceDistance;
	const float EffectiveDamage = (DamageOverride >= 0.0f) ? DamageOverride : AttackDamage;

	FVector StartPoint;
	FVector EndPoint;

	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh && SkelMesh->DoesSocketExist(TargetSocket))
	{
		StartPoint = SkelMesh->GetSocketLocation(TargetSocket);
		const FRotator SocketRot = SkelMesh->GetSocketRotation(TargetSocket);
		EndPoint = StartPoint + SocketRot.Vector() * ActualDistance;
	}
	else
	{
		// Fallback si pas de socket : utilise la position du monstre a mi-hauteur
		StartPoint = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
		EndPoint = StartPoint + GetActorForwardVector() * ActualDistance;
	}

	FCollisionShape Sphere = FCollisionShape::MakeSphere(ActualRadius);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LiminalAttackSweep), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> OutHits;
	const bool bHitAny = World->SweepMultiByChannel(
		OutHits, StartPoint, EndPoint, FQuat::Identity, ECC_Pawn, Sphere, QueryParams);

#if !UE_BUILD_SHIPPING
	if (bDrawDebug)
	{
		DrawDebugCapsule(World, (StartPoint + EndPoint) * 0.5f,
			ActualDistance * 0.5f + ActualRadius, ActualRadius,
			FRotationMatrix::MakeFromZ(EndPoint - StartPoint).ToQuat(),
			bHitAny ? FColor::Red : FColor::Green, false, 1.5f, 0, 1.5f);
	}
#endif

	if (!bHitAny)
	{
		return 0;
	}

	TSet<AActor*> DamagedActors;
	int32 HitCount = 0;
	const FVector HitDir = (EndPoint - StartPoint).GetSafeNormal();

	for (const FHitResult& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == this || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		DamagedActors.Add(HitActor);

		if (AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(HitActor))
		{
			if (Scavenger->IsDead())
			{
				continue;
			}

			// 1. Application des dégâts orientes Point Damage
			UGameplayStatics::ApplyPointDamage(
				Scavenger,
				EffectiveDamage,
				HitDir,
				Hit,
				GetController(),
				this,
				DamageType ? DamageType : UDamageType::StaticClass()
			);

			// 2. Audio d'impact de chair / coup physique
			USoundBase* ImpactSoundToPlay = AttackImpactSound ? AttackImpactSound.Get() : AttackSound.Get();
			if (ImpactSoundToPlay)
			{
				UGameplayStatics::PlaySoundAtLocation(World, ImpactSoundToPlay, Hit.ImpactPoint,
					1.0f, FMath::FRandRange(0.9f, 1.1f));
			}

			// 3. Propagation du son d'impact a l'ouie de l'IA environnante
			UAISense_Hearing::ReportNoiseEvent(World, Hit.ImpactPoint, 1.0f, this);

			HitCount++;
		}
	}

	return HitCount;
}
```

---

## 5. Damage Delivery, Hit Reactions & Audio (Feature F06)

### 5.1 Damage Delivery Chain
```
ALiminalEntity (Server)
    │ ApplyPointDamage()
    ▼
AScavengerCharacter::TakeDamage() (Server)
    │ Decrements CurrentHealth
    │ Calls ClientOnDamaged() RPC
    ▼
AScavengerCharacter::ClientOnDamaged_Implementation() (Owning Client)
    ├─► DamageFlashIntensity = 1.0f (HUD Red Vignette)
    ├─► AddControllerPitchInput(-3.5f) & AddControllerYawInput(±3.0f) (Camera Recoil)
    ├─► FirstPersonCamera Positional Punch (-12.0f forward punch)
    └─► ClientStartCameraShake(DamageCameraShakeClass) (Camera Shake)
```

### 5.2 Third-Person Hit Reaction Montage
For multiplayer spectators and co-op scavengers observing the victim:
- `AScavengerCharacter` plays `HitReactMontage` (`MM_HitReact_Front_Lgt_01` or `MM_HitReact_Front_Med_01` from `Content/Characters/Mannequins/Anims/Rifle/HitReact/`).
- Triggered on all simulated proxies when health decreases.

### 5.3 Headless Automation Safe Attack Execution
In `ALiminalEntity::PerformMeleeAttack(AActor* Target)`:
```cpp
bool ALiminalEntity::PerformMeleeAttack(AActor* Target)
{
	if (!CanAttack() || !Target || !HasAuthority())
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	if (DistSq > FMath::Square(AttackRange + 50.0f))
	{
		return false;
	}

	AttackCooldownTimer = AttackCooldownSeconds;

	// Son d'attaque (cri / grognement / charge)
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetActorLocation(),
			1.0f, FMath::FRandRange(0.85f, 1.15f));
	}

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, this);

	// Lancement de l'animation d'attaque si disponible
	bool bMontageTriggered = false;
	if (AttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		const float PlayDuration = PlayAnimMontage(AttackMontage);
		bMontageTriggered = (PlayDuration > 0.0f);
	}

	// Fallback immediat si headless / pas d'animation / pas de squelette
	// Garantit que les tests d'automatisation (nullrhi) ne bloquent jamais
	if (!bMontageTriggered)
	{
		OnAttackNotify(DefaultAttackSocket, DefaultAttackTraceRadius, AttackRange);
	}

	return true;
}
```

---

## 6. Detailed Subclass Adaptations

| Subclass | Sizing / Scale Adjustment | Mesh Visibility & Cloak | Combat & Socket Adaptation |
|----------|---------------------------|-------------------------|----------------------------|
| `ALiminalEntity_Hound` (Base) | Default scale (0.9, 0.9, 0.9) | Standard visibility | Socket: `jawSocket` or `AttackSocket`, bite attack |
| `ALiminalEntity_Smiler` | Both meshes scaled to `0.01f` | Invisible body; light eyes/mouth glow active | Socket: `headSocket`, high-speed charge collision |
| `ALiminalEntity_Partygoer` | Scale `(0.9, 0.9, 1.05)` | Standard visibility; `BalloonMesh` bobbing | Socket: `hand_rSocket`, infection on claw swipe |
| `ALiminalEntity_Duller` | Scale `(1.1, 1.1, 1.0)` | `UpdateVisualCloakAppearance()` toggles both meshes | Socket: `hand_rSocket`, ambush lunge |
| `ALiminalEntity_Skinwalker` | Standard scale `(1.0, 1.0, 1.0)` | Flickering headlamp mimicry | Socket: `hand_rSocket`, sprint ambush swipe |
| `ALiminalEntity_Clump` | Scale `(1.8, 1.8, 0.35)` | RelLoc `(0, 0, -60)` applied to both | Trigger sphere constriction + claw sweep |
| `ALiminalEntity_Deathmoth` | Scale `(1.3, 1.6, 0.6)` | MOVE_Flying mode with abdomen glow | Socket: `jawSocket`, aerial dive sweep |
| `ALiminalEntity_Jerry` | Scale `(0.45, 0.45, 0.45)` | Psionic aura light attachment | Hypnotic gaze raycast + psychic peck |
| `ALiminalEntity_Wretch` | Scale `(0.85, 0.85, 1.15)` | Standard visibility | Socket: `hand_rSocket`, frenzy rush swipe |
| `ALiminalEntity_Watcher` | Scale `(0.6, 0.6, 0.6)` | Eye glow point light attachment | Gaze tracking + proximity teleportation |

---

## 7. Verification Plan & Test Strategy

### 7.1 Existing Tests Preservation
The following tests in `Source/MEG_Reclamation/Tests/MegReclamationTests.cpp` MUST pass without modifications or regressions:
1. `FMegTacticalPolishAndSpectatorTest`: Verifies `GetDefault<ALiminalEntity>()` CDO.
2. `FMegSmilerSensoryTest`: Verifies `GetDefault<ALiminalEntity_Smiler>()` CDO and speeds.
3. `FMegJerryHypnosisGazeTest`: Verifies `GetDefault<ALiminalEntity_Jerry>()` CDO and gaze properties.
4. `FMegScavengerCapabilitiesTest`: Verifies `GetDefault<AScavengerCharacter>()` CDO.

### 7.2 New Automation Test: `FMegEntityAnimNotifyTraceTest`
Add a dedicated test verifying:
1. `ALiminalEntity` CDO has valid `GetMesh()` (`USkeletalMeshComponent`).
2. `OnAttackNotify()` executes safely with 0 targets in range (returns 0, no crash).
3. Spawning an entity and player in a test world: executing `OnAttackNotify()` when within trace range applies damage to `AScavengerCharacter`.
4. `UAnimNotify_LiminalAttackTrace` CDO is valid and constructible.

### 7.3 CLI Verification Commands
1. **Compilation**:
   ```powershell
   & "F:\UE_5.8\Engine\Build\BatchFiles\Build.bat" MEG_ReclamationEditor Win64 Development "F:\MEG_Reclamation\MEG_Reclamation.uproject" -waitmutex
   ```
2. **Automation Tests**:
   ```powershell
   & "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "F:\MEG_Reclamation\MEG_Reclamation.uproject" -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty" -log
   ```
3. **Auto-Check Validation**:
   ```powershell
   powershell -File "F:\MEG_Reclamation\Run_Auto_Check.ps1" -SkipLiveExec
   ```
