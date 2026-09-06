# Technical Analysis: Bestiary Rigging, Locomotion & Audio Wiring (M1 / F04, F06)
**Project**: M.E.G. : Reclamation (Unreal Engine 5.8)  
**Milestone**: M1 - Skeletal Animation & Bestiary (Features F04, F06)  
**Author**: teamwork_preview_explorer (Bestiary Rigging, Locomotion & Audio Wiring Specialist)  
**Date**: 2026-09-06  

---

## Executive Summary

This document formulates the comprehensive asset generation, procedural rigging, animation blendspace, and audio wiring plan for all **9 hostile entities** in *M.E.G. : Reclamation*:
- **Humanoid Entities (5)**: **Partygoer**, **Skinwalker**, **Wretch**, **Duller**, **Smiler**.
- **Non-Humanoid Entities (4)**: **Hound** (quadruped predator), **Deathmoth** (aerial insectoid flier), **Clump** (amorphous multi-limbed ambush mass), **Jerry** (anomalous psionic avian).

Currently, all 9 entities inherit from `ALiminalEntity : public ACharacter` but render as static meshes attached to a dummy `UStaticMeshComponent* BodyMesh` with no skeletal deformation, no locomotion blendspaces, and instant mathematical distance-check damage in C++. This analysis provides the technical blueprint to transition all 9 entities to fully animated `USkeletalMeshComponent` assets (`GetMesh()`), with locomotion blendspaces, physical attack montages synchronized to C++ `AnimNotify` damage sweeps on `AttackSocket` (`ECC_Pawn`), and 3D spatialized audio cues wired to animation keyframes.

---

## 1. Asset & Codebase Audit

### 1.1 Existing 3D Models & Raw Assets
- **`models/hound_extracted/`**:
  - File: `Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx/Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx`
  - Texture: `Meshy_AI_Backroom_Creature_Enc_0824101528_texture.png` (diffuse/albedo map)
  - Mesh details: `Mesh_0`, 4,998 vertices, 9,999 triangles.
  - Bounding Box: `[-0.226, -1.002, -0.474]` to `[0.223, 1.004, 0.470]` meters (Length: 200.6 cm, Width: 44.9 cm, Height: 94.4 cm).
  - Current status: Static mesh without skeleton.
- **`models/hound.fbx`**: Binary duplicate of `hound.zip` (invalid FBX header); the usable FBX is the extracted creature mesh above.
- **`models/wire_hound.py`**: Legacy script that imported `Meshy_...fbx` as static mesh `SM_Hound` and bound it to `BP_Hound.BodyMesh`.
- **`RawAssets/FBX/Props/SM_Partygoer_Balloon.fbx`**: Prop mesh used by Partygoer entity.
- **`Content/Characters/Mannequins/`**:
  - Full Unreal Engine 5.8 Mannequin suite:
    - Skeletal Mesh: `SKM_Manny_Simple.uasset`, `SKM_Quinn_Simple.uasset`
    - Skeleton: `SK_Mannequin.uasset`
    - Animation Blueprint: `ABP_Unarmed.uasset`
    - Locomotion BlendSpace: `BS_Idle_Walk_Run.uasset`
    - Idle: `MM_Idle.uasset`
    - Walks: `MF_Unarmed_Walk_Fwd.uasset`, `MF_Unarmed_Walk_Bwd.uasset`, etc.
    - Jogs: `MF_Unarmed_Jog_Fwd.uasset`, `MF_Unarmed_Jog_Bwd.uasset`, etc.
    - Attacks: `MM_Attack_01.uasset`, `MM_Attack_02.uasset`, `MM_Attack_03.uasset`, `MM_ChargedAttack.uasset`
    - Deaths: `MM_Death_Front_01/02/03.uasset`, `MM_Death_Back_01.uasset`, `MM_Death_Left_01.uasset`, `MM_Death_Right_01.uasset`

### 1.2 Audio Assets & Sound Inventory
All required sound waves are imported as `.uasset` in `Content/Audio/` and backed by WAVs in `RawAudio/` and `RawAssets/Audio/`:
- **Hound**: `S_Hound_Snarl.uasset` (1.2s guttural snarl), `S_Hound_Bite.uasset` (0.35s crunch/snap)
- **Smiler**: `S_Smiler_Distortion.uasset` (1.5s resonant dissonance & static)
- **Partygoer**: `S_Partygoer_Chime.uasset` (1.6s eerie music box notes C6-B6)
- **Clump**: `S_Clump_Gurgle.uasset` (wet bubbling), `S_Clump_Drag.uasset` (visceral dragging)
- **Deathmoth**: `S_Deathmoth_Flutter.uasset` (rapid wing beating), `S_Deathmoth_Screech.uasset` (piercing insect scream)
- **Duller**: `S_Duller_Growl.uasset` (low guttural growl), `S_Duller_Rush.uasset` (whoosh/rush)
- **Jerry**: `S_Jerry_Whisper.uasset` (psionic whisper drone), `S_Jerry_Laugh.uasset` (mocking parrot chatter)
- **Skinwalker**: `S_Skinwalker_Mimic.uasset` (distorted radio voice), `S_Skinwalker_Scream.uasset` (demonic shriek)
- **Wretch**: `S_Wretch_Snarl.uasset` (wheezing feral growl), `S_Wretch_Lunge.uasset` (biting lunge)
- **Footsteps & Impact**: `S_Footstep_Carpet_01/02.uasset`, `S_Footstep_Concrete_01/02.uasset`, `S_Loot_Impact_Heavy.uasset`
- **Attenuation Asset**: `SA_LiminalDefault.uasset` in `Content/Audio/`

### 1.3 C++ AI & Combat Current State
- `ALiminalEntity` inherits `ACharacter`. While `ACharacter` provides `USkeletalMeshComponent* Mesh` (`GetMesh()`), `ALiminalEntity` currently creates an extra `UStaticMeshComponent* BodyMesh` (`LiminalEntity.cpp:35-37`) and attaches it to `GetCapsuleComponent()`.
- Combat currently executes in `ALiminalEntity::PerformMeleeAttack(AActor* Target)` (`LiminalEntity.cpp:167-192`):
  - Checks Euclidean distance `DistSq > FMath::Square(AttackRange + 50.0f)`.
  - Directly invokes `UGameplayStatics::ApplyDamage(Target, AttackDamage, ...)`.
  - Plays `AttackSound` immediately at actor location.
- **Deficiency against PROJECT.md Interface Contract**:
  - `ALiminalEntity::OnAttackNotify()` does NOT exist yet.
  - `AttackSocket` is not queried.
  - Damage is not synchronized with physical weapon/limb swing animations.
  - No `AnimNotify` classes or notifies exist in the animation sequences.

---

## 2. Humanoid Entities Plan (Partygoer, Skinwalker, Wretch, Duller, Smiler)

The 5 humanoid entities share the standard Unreal Engine Mannequin skeleton (`SK_Mannequin`) located at `/Game/Characters/Mannequins/Meshes/SK_Mannequin`. This provides immediate access to high-fidelity mocap animation assets (`MM_Idle`, `BS_Idle_Walk_Run`, `MM_Attack_01/02/03`, `MM_Death_Front/Back`), eliminating performance overhead and asset bloat while maximizing visual fidelity.

Each humanoid entity is differentiated via **proportional scaling**, **dynamic PBR materials**, **attached accessories/sockets**, and **behavioral blendspace parameterization**:

| Entity | Base Mesh | Scale (X, Y, Z) | Material / Appearance | Attached Accessories & Sockets | Locomotion & Anim Montages |
|---|---|---|---|---|---|
| **Partygoer** | `SKM_Manny_Simple` | `(0.90, 0.90, 1.05)` (slender, unnaturally tall) | `M_Partygoer` (Smooth yellow leathery hide, drawn red smile, hollow black eye sockets) | `BalloonMesh` (`SM_Partygoer_Balloon`) attached to `hand_rSocket`; `BalloonLight` (Red PointLight 180 cd) | Locomotion: `BS_Partygoer_Locomotion` (cheerful, upright stride, 520 cm/s); Attack: `MM_Attack_01` (two-handed reaching grab); Death: `MM_Death_Front_01` |
| **Skinwalker** | `SKM_Manny_Simple` (or `Quinn`) | `(1.00, 1.00, 1.00)` (exact human proportion) | `M_Skinwalker_Scavenger` (Bloodstained, ripped MEG Hazmat suit; dynamically mimics teammate skin) | `GlitchedHeadlamp` (SpotLight 300 cd, cone 18 deg / 38 deg) on `headSocket`; fake walkie-talkie on `pelvisSocket` | Locomotion: `BS_Idle_Walk_Run` (indistinguishable from human scavenger); Attack: `MM_ChargedAttack` (sudden violent claw thrust); Death: `MM_Death_Back_01` |
| **Wretch** | `SKM_Manny_Simple` | `(0.85, 0.85, 1.15)` (emaciated, hunched, gaunt) | `M_Wretch_Skin` (Pale necrotic bruised flesh, visible ribs, dark veins, peeling skin) | Claws on fingers; sunken eye sockets; twitching posture | Locomotion: `BS_Wretch_Locomotion` (shivering idle, erratic dragging walk 380 cm/s, sprint 650 cm/s); Attack: `MM_Attack_03` (flailing claw swipe); Death: `MM_Death_Front_02` |
| **Duller** | `SKM_Manny_Simple` | `(1.10, 1.10, 1.00)` (broad, imposing silhouette) | `M_Duller_Shadow` (Master Cloak: index-matching refractive refraction + fresnel in unrevealed state; transitions to matte void black when revealed) | Shimmering outline in unrevealed state; revealed via LIDAR/Strobe (`bIsRevealed`) | Locomotion: `BS_Duller_Locomotion` (silent, low stalking crawl 390 cm/s); Attack: `MM_Attack_01` (sudden forward leap); Death: `MM_Death_Left_01` with smoke dissolve |
| **Smiler** | `SKM_Manny_Simple` | `(1.00, 1.00, 1.00)` | `M_Smiler_Void` (100% light-absorbing unlit pitch black material for body) + `M_Smiler_Emissive` on face | Floating wide grinning teeth & circular eyes (`SM_Smiler_Face`) on `headSocket` + 3 PointLights (`EyeGlowL`, `EyeGlowR`, `SmileGlow`) | Locomotion: `BS_Smiler_Charge` (lurks motionless at 0 cm/s; charges at 850 cm/s when illuminated); Attack: `MM_Attack_01` (snapping jaw thrust); Death: Lights flicker and extinguish |

### 2.1 Humanoid Animation Blueprint Architecture (`ABP_Entity_Humanoid`)
- **Parent Skeleton**: `/Game/Characters/Mannequins/Meshes/SK_Mannequin`
- **State Machine**:
  1. **Idle/Walk/Run**: Driven by `Speed` (computed in `NativeUpdateAnimation` from `TryGetPawnOwner()->GetVelocity().Size2D()`). Evaluates `BS_Idle_Walk_Run` (or entity child blendspace).
  2. **Attack State**: Plays `Montage_Play(AttackMontage)`. Contains notify `AnimNotify_EntityAttack` on the apex damage frame and `AnimNotify_PlaySound` for attack vocalization.
  3. **Stunned State**: Evaluates an additive hit react / stunned loop (`MM_HitReact_Front_Hvy_01`) when `ALiminalEntity::IsStunned() == true`.
  4. **Death State**: Plays `DeathMontage` upon `CurrentHealth <= 0.0f`, followed by ragdoll activation or freezing the final frame.

---

## 3. Non-Humanoid Procedural Rigging & Blender 5.2.1 Pipeline

For the 4 non-humanoid entities, procedural Python scripts for **Blender 5.2.1 LTS** (`F:\blender\blender.exe`) generate the anatomical mesh geometry (or bind existing FBX mesh for Hound), build the bone hierarchy, compute vertex skinning weights, author 4 distinct keyframed animation actions (Idle, Walk/Fly, Attack, Death), and export game-ready Skeletal FBXs with `add_leaf_bones=False` and bake animations enabled.

### 3.1 Hound (Quadruped Predator)
- **Geometry Source**: `models/hound_extracted/Meshy_AI_Backroom_Creature_Enc_0824101528_texture_fbx/Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx`
- **Armature Hierarchy** (`SK_Hound_Skeleton`):
  - `root` (0, 0, 0)
    - `pelvis` (0, -0.45, 0.70)
      - `spine_01` (0, -0.20, 0.72) -> `spine_02` (0, 0.10, 0.74) -> `chest` (0, 0.35, 0.75)
        - `neck` (0, 0.55, 0.78) -> `head` (0, 0.75, 0.82)
          - `jaw` (0, 0.78, 0.75) [Socket: `AttackSocket`]
          - `snout` (0, 0.92, 0.80)
        - `clavicle_l` (-0.12, 0.40, 0.70) -> `upperarm_l` -> `forearm_l` -> `paw_fl`
        - `clavicle_r` (0.12, 0.40, 0.70) -> `upperarm_r` -> `forearm_r` -> `paw_fr`
      - `hip_l` (-0.12, -0.45, 0.68) -> `thigh_l` -> `calf_l` -> `paw_bl`
      - `hip_r` (0.12, -0.45, 0.68) -> `thigh_r` -> `calf_r` -> `paw_br`
- **Keyframed Actions (30 fps)**:
  1. `Hound_Idle` (60 frames / 2.0s loop): Quadruped crouched posture; chest undulates vertically ±2.5 cm (1.5s period breathing); head scans subtly left/right (±8 deg); jaw pants slightly.
  2. `Hound_Walk` / `Hound_Run` (24 frames / 0.8s loop): Diagonal trot gait (FL + BR synchronize, alternating with FR + BL); spine flexes sinusoidally; paws hit ground at frames 4, 10, 16, 22.
  3. `Hound_Attack` (30 frames / 1.0s):
     - Frames 1-8: Anticipation crouch onto hind legs.
     - Frames 9-14: Explosive forward lunge (+45 cm forward), jaw snaps shut at frame 12 (Notifies: `AnimNotify_PlaySound` [S_Hound_Bite] at f=11, `AnimNotify_EntityAttack` at f=12).
     - Frames 15-30: Landing recovery back onto all fours.
  4. `Hound_Death` (35 frames / 1.16s): Forelegs buckle forward, chest impacts ground at f=14, hindquarters collapse to right, jaw drops agape.

### 3.2 Deathmoth (Aerial Insectoid Flier)
- **Procedural Mesh**: Head with compound eye hemispheres and arched feathery antennae; segmented furry thorax; elongated bulbous abdomen (3 segments) with bioluminescent ventral spots; 4 distinct wings (forewings 120 cm span, hindwings 75 cm span); 6 hooked insect legs.
- **Armature Hierarchy** (`SK_Deathmoth_Skeleton`):
  - `root` (0, 0, 0)
    - `thorax` (0, 0, 1.20)
      - `abdomen_01` (0, -0.25, 1.15) -> `abdomen_02` (0, -0.55, 1.08) -> `abdomen_03` (0, -0.85, 1.00) [Socket: `AbdomenSocket`]
      - `head` (0, 0.25, 1.22) [Socket: `AttackSocket`] -> `antenna_l`, `antenna_r`, `mandibles`
      - `wing_fore_l`, `wing_fore_r` (spans 120 cm)
      - `wing_hind_l`, `wing_hind_r` (spans 75 cm)
      - `leg_f_l/r`, `leg_m_l/r`, `leg_b_l/r` (6 legs)
- **Keyframed Actions (30 fps)**:
  1. `Deathmoth_Idle` / `Hover` (30 frames / 1.0s loop): High-frequency wing flapping (4 complete flap cycles, ±35 deg pitch/roll oscillation, frames 0-7, 8-15, 16-22, 23-30); vertical body hover bobbing (±4 cm); antennae drift.
  2. `Deathmoth_Fly` (20 frames / 0.66s loop): Fast flight; wings swept backward at 25 deg angle; abdomen tilted down 18 deg; rapid 6-cycle flap rate.
  3. `Deathmoth_Attack` (25 frames / 0.83s): Steep dive lunge; forelegs thrust outward; mandibles snap forward at frame 10 (Notifies: `AnimNotify_PlaySound` [S_Deathmoth_Screech] at f=8, `AnimNotify_EntityAttack` at f=10).
  4. `Deathmoth_Death` (40 frames / 1.33s): Wings shudder and fold limp against thorax; legs curl inward into spasm; body rotates inverted into freefall descent.

### 3.3 Clump (Amorphous Mass of Limbs)
- **Procedural Mesh**: Irregular organic mound base (radius 140 cm, height 40 cm) with blisters and flesh folds; 8 human appendages (4 arms, 4 legs) protruding and grasping outward in a 360 deg perimeter.
- **Armature Hierarchy** (`SK_Clump_Skeleton`):
  - `root` (0, 0, 0)
    - `mound_core` (0, 0, 0.20) [Socket: `AttackSocket`]
      - `arm_01` (shoulder -> elbow -> hand) at 45 deg
      - `arm_02` (shoulder -> elbow -> hand) at 135 deg
      - `arm_03` (shoulder -> elbow -> hand) at 225 deg
      - `arm_04` (shoulder -> elbow -> hand) at 315 deg
      - `leg_01` (thigh -> knee -> foot) at 0 deg
      - `leg_02` (thigh -> knee -> foot) at 90 deg
      - `leg_03` (thigh -> knee -> foot) at 180 deg
      - `leg_04` (thigh -> knee -> foot) at 270 deg
- **Keyframed Actions (30 fps)**:
  1. `Clump_Idle` (60 frames / 2.0s loop): Grotesque respiration pulse; mound core expands/contracts by 8%; sprawling hands twitch and scrape carpet irregularly.
  2. `Clump_Slither` (40 frames / 1.33s loop): Crawling contraction; perimeter arms reach forward, plant hands, pull central mound forward 40 cm, legs follow.
  3. `Clump_Attack` (30 frames / 1.0s): Upward eruptive grab; all 4 arm chains shoot upward and converge into an enclosing cage, snapping shut at frame 12 (Notifies: `AnimNotify_PlaySound` [S_Clump_Drag] at f=10, `AnimNotify_EntityAttack` at f=12).
  4. `Clump_Death` (45 frames / 1.5s): Core collapses flat to ground level (Z = 5 cm); arms and legs spasm violently for 15 frames, then sprawl flaccid.

### 3.4 Jerry (Anomalous Psionic Avian)
- **Procedural Mesh**: Macaw avian body (scale ~45 cm); curved predator beak; iridescent scarlet/cyan feathers; fanned tail feathers; perching clawed feet.
- **Armature Hierarchy** (`SK_Jerry_Skeleton`):
  - `root` (0, 0, 0)
    - `body` (0, 0, 0.25)
      - `chest` (0, 0.05, 0.32) -> `neck` -> `head` [Socket: `AuraSocket`]
        - `beak_upper` (0, 0.24, 0.45) [Socket: `AttackSocket`]
        - `beak_lower` (0, 0.22, 0.41)
      - `wing_l` (upper -> forearm -> hand -> feathers)
      - `wing_r` (upper -> forearm -> hand -> feathers)
      - `tail` (0, -0.18, 0.20)
      - `leg_l` (thigh -> calf -> foot)
      - `leg_r` (thigh -> calf -> foot)
- **Keyframed Actions (30 fps)**:
  1. `Jerry_Idle` (60 frames / 2.0s loop): Perched idle; sharp avian head twitches at f=15 (45 deg left) and f=35 (60 deg right); subtle feather shuffle.
  2. `Jerry_Hop` (16 frames / 0.53s loop): Classic avian double hop; crouches f=0-3, launches f=4-8, touches down with slight wing flare f=9-16.
  3. `Jerry_Hypnosis` / `Attack` (40 frames / 1.33s loop): Psionic channeling pose; wings flare wide horizontally (180 deg span); beak gapes open in silent cackle; head shakes with high-frequency micro-tremor (Notifies: `AnimNotify_PlaySound` [S_Jerry_Laugh] at f=5, `AnimNotify_EntityAttack` at f=10).
  4. `Jerry_Death` (30 frames / 1.0s): Stiffens abruptly; wings crumple haphazardly; topples backward off perch; lies motionless upside down.

---

## 4. Audio Cue Wiring & AnimNotify Specification

### 4.1 Audio Routing Architecture
Each entity has:
1. **Sound Cue Wrappers (`SC_<Entity>_Aggro`, `SC_<Entity>_Attack`, `SC_<Entity>_Footstep`)**:
   - Wrap raw sound waves with `SoundNodeModulator` (Random Pitch: 0.88-1.12, Random Volume: 0.90-1.05) to eliminate repetitive audio fatigue.
   - Assign `AttenuationSettings = /Game/Audio/SA_LiminalDefault.SA_LiminalDefault` for full 3D inverse-distance falloff.
   - Route to `SoundClass = /Game/Audio/SC_Monsters` or `SC_SFX`.
2. **Keyframe-Synchronized AnimNotifies**:
   - `AnimNotify_PlaySound`: Triggers attack vocalization/screech or footstep on exact contact frames.
   - `AnimNotify_EntityAttack`: Custom C++ notify executing damage trace sweep on `AttackSocket`.
   - `AnimNotify_EntityFootstep`: Custom notify reporting hearing noise (`UAISense_Hearing::ReportNoiseEvent`) and playing surface-aware footstep audio.

### 4.2 Entity Audio Wiring Matrix

| Entity | Aggro Audio (`AggroSound`) | Attack Audio (`AttackSound`) | Attack Anim & Trigger Frame | Footstep / Movement Audio & Frequency |
|---|---|---|---|---|
| **Hound** | `S_Hound_Snarl` (1.2s guttural growl) | `S_Hound_Bite` (0.35s snap & crush) | `Hound_Attack`: Notify at **f=11** (sound), **f=12** (damage sweep) | 4-beat quadruped trot (`S_Footstep_Concrete_01/02`) at f=4, 10, 16, 22 |
| **Smiler** | `S_Smiler_Distortion` (1.5s dissonance & static) | `S_Smiler_Distortion` (pitch shifted +1.3x) | `MM_Attack_01`: Notify at **f=6** (distortion), **f=12** (damage sweep) | Silent stalking; faint static hum loop at high speed (850 cm/s) |
| **Partygoer** | `S_Partygoer_Chime` (1.6s music box C6-B6) | `S_Partygoer_Chime` (reversed/distorted burst) | `MM_Attack_01`: Notify at **f=14** (chime strike), **f=18** (infection sweep) | Light rhythmic shoe steps (`S_Footstep_Carpet_01/02`) at f=12, 24 |
| **Clump** | `S_Clump_Gurgle` (wet visceral bubbling) | `S_Clump_Drag` (wet dragging impact) | `Clump_Attack`: Notify at **f=10** (drag sound), **f=12** (grab sweep) | Fleshy squelch/slide loop; periodic wet slap at f=20 on slither |
| **Deathmoth** | `S_Deathmoth_Flutter` (rapid wing buzzing) | `S_Deathmoth_Screech` (high chitinous shriek) | `Deathmoth_Attack`: Notify at **f=8** (screech), **f=10** (dive slash) | Continuous wing flutter loop attached to flap cycle (12-16 Hz) |
| **Duller** | `S_Duller_Growl` (low resonant drone) | `S_Duller_Rush` (violent sudden whoosh) | `MM_Attack_01`: Notify at **f=8** (rush sound), **f=14** (claw sweep) | Muffled, stealth padded steps (50% volume of standard footstep) |
| **Jerry** | `S_Jerry_Whisper` (psionic whispering) | `S_Jerry_Laugh` (mocking parrot chatter) | `Jerry_Hypnosis`: Notify at **f=5** (laugh/channel), **f=10** (hypnosis tick) | Claw tap/scratch on concrete at f=8 on each hop |
| **Skinwalker** | `S_Skinwalker_Mimic` (garbled radio voice) | `S_Skinwalker_Scream` (unhinged beast shriek) | `MM_ChargedAttack`: Notify at **f=10** (scream), **f=15** (lethal claw) | Heavy scavenger boot steps (`S_Footstep_Concrete_01/02`) identical to players |
| **Wretch** | `S_Wretch_Snarl` (wheezing feral snarl) | `S_Wretch_Lunge` (violent biting lunge) | `MM_Attack_03`: Notify at **f=12** (lunge vocal), **f=16** (bite sweep) | Irregular dragging scuff steps (asymmetric timing: f=8, f=22) |

---

## 5. C++ Architecture Implementation Plan

### 5.1 Migration from `UStaticMeshComponent* BodyMesh` to `GetMesh()`
In `LiminalEntity.h` / `.cpp`:
- Deprecate standalone `BodyMesh` and configure base `ACharacter::GetMesh()` directly in `ALiminalEntity::ALiminalEntity()`:
```cpp
GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
```

### 5.2 `ALiminalEntity::OnAttackNotify()` & Sphere Sweep Specification
```cpp
void ALiminalEntity::OnAttackNotify()
{
    if (!HasAuthority() || CurrentHealth <= 0.0f)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    const FVector SocketLocation = (MeshComp && MeshComp->DoesSocketExist(TEXT("AttackSocket"))) 
        ? MeshComp->GetSocketLocation(TEXT("AttackSocket")) 
        : (GetActorLocation() + GetActorForwardVector() * 70.0f);

    const float SweepRadius = AttackRange * 0.45f; // ~60-70 cm coverage
    const FVector SweepStart = SocketLocation;
    const FVector SweepEnd = SocketLocation + GetActorForwardVector() * (AttackRange * 0.5f);

    TArray<FHitResult> HitResults;
    FCollisionShape SweepSphere = FCollisionShape::MakeSphere(SweepRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    const bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults, SweepStart, SweepEnd, FQuat::Identity, ECC_Pawn, SweepSphere, QueryParams);

    for (const FHitResult& Hit : HitResults)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            if (AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(HitActor))
            {
                const FVector HitDir = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                UGameplayStatics::ApplyPointDamage(
                    Scavenger, AttackDamage, HitDir, Hit, GetController(), this, UDamageType::StaticClass());

                if (AttackSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        GetWorld(), AttackSound, Hit.ImpactPoint, 1.0f, FMath::FRandRange(0.92f, 1.08f));
                }

                UAISense_Hearing::ReportNoiseEvent(GetWorld(), Hit.ImpactPoint, 1.0f, this);
                break; // Melee swing impacts first valid player target
            }
        }
    }
}
```

### 5.3 C++ AnimNotify Classes
In `Source/MEG_Reclamation/Animation/LiminalAnimNotifies.h/.cpp`:
- `UAnimNotify_EntityAttack : public UAnimNotify`:
  - `virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;`
  - Casts `MeshComp->GetOwner()` to `ALiminalEntity` and calls `Entity->OnAttackNotify()`.
- `UAnimNotify_EntityFootstep : public UAnimNotify`:
  - `UPROPERTY(EditAnywhere) FName FootSocketName;`
  - `UPROPERTY(EditAnywhere) float Loudness = 0.6f;`
  - Traces down to surface, plays appropriate footstep sound cue, and reports noise event via `UAISense_Hearing::ReportNoiseEvent`.

---

## 6. Concrete Asset Generation & Import Scripts

### 6.1 Blender 5.2.1 Batch Generation Script (`generate_bestiary_blender.py`)
To be placed in `F:/MEG_Reclamation/scripts/generate_bestiary_blender.py` and executed via:
`& "F:\blender\blender.exe" -b --python "F:\MEG_Reclamation\scripts\generate_bestiary_blender.py"`

**Script Features**:
1. **Hound**:
   - Imports `models/hound_extracted/.../Meshy_...texture.fbx`.
   - Generates 19-bone quadruped armature.
   - Computes automatic vertex weights.
   - Bakes `Hound_Idle`, `Hound_Walk`, `Hound_Attack`, `Hound_Death` actions.
   - Exports `RawAssets/FBX/Bestiary/SK_Hound.fbx`.
2. **Deathmoth**:
   - Generates procedural multi-segmented insect mesh (head, compound eyes, antennae, thorax, abdomen with bioluminescent pods, 4 wings, 6 legs).
   - Generates 16-bone flight armature.
   - Bakes `Deathmoth_Hover`, `Deathmoth_Fly`, `Deathmoth_Attack`, `Deathmoth_Death` actions.
   - Exports `RawAssets/FBX/Bestiary/SK_Deathmoth.fbx`.
3. **Clump**:
   - Generates visceral mound geometry with 8 branching human limbs.
   - Generates 17-bone multi-limb armature.
   - Bakes `Clump_Idle`, `Clump_Slither`, `Clump_Attack`, `Clump_Death` actions.
   - Exports `RawAssets/FBX/Bestiary/SK_Clump.fbx`.
4. **Jerry**:
   - Generates avian macaw geometry (head, beak, wings, tail, clawed perching feet).
   - Generates 14-bone avian armature.
   - Bakes `Jerry_Idle`, `Jerry_Hop`, `Jerry_Hypnosis`, `Jerry_Death` actions.
   - Exports `RawAssets/FBX/Bestiary/SK_Jerry.fbx`.

### 6.2 Unreal Engine Import & Blueprint Wiring Script (`import_bestiary_ue.py`)
To be executed headless in Unreal Engine 5.8:
`& "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "F:\MEG_Reclamation\MEG_Reclamation.uproject" -ExecutePythonScript="F:\MEG_Reclamation\scripts\import_bestiary_ue.py" -unattended -nopause -nullrhi`

**Script Actions**:
1. Creates destination folders: `/Game/Characters/Bestiary/{Hound, Deathmoth, Clump, Jerry, Humanoids}`.
2. Executes `AssetImportTask` with `FbxImportUI.import_as_skeletal = True` to import the 4 non-humanoid FBXs into `USkeletalMesh`, `USkeleton`, and `UAnimSequence`.
3. Creates SoundCue assets wrapping all 9 entity sound waves with pitch modulation and `SA_LiminalDefault` attenuation.
4. Generates or configures the Animation Blueprints (`ABP_Hound`, `ABP_Deathmoth`, `ABP_Clump`, `ABP_Jerry`, `ABP_Entity_Humanoid`).
5. Updates the Blueprint CDOs (`BP_Hound`, etc.) and C++ entity classes to assign the imported Skeletal Meshes and Animation Blueprints.

---

## 7. Verification & Testing Strategy

### 7.1 Automated Suite Verification
Execute the project automated test command:
`& "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "F:\MEG_Reclamation\MEG_Reclamation.uproject" -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty"`
Ensure all 30 tests (including `FMegAssetPipelineGenerationTest` and `FMegTacticalPolishAndSpectatorTest`) pass with Exit Code 0.

### 7.2 Integrity Audit Verification
Execute the project health audit:
`powershell -ExecutionPolicy Bypass -File "F:\MEG_Reclamation\Run_Auto_Check.ps1"`
Ensure 27/27 checks pass, including maps, executables, and multi-agent scripts.

### 7.3 Invalidation Conditions
- Any entity skeletal mesh failing to import or resulting in 0-vertex count.
- Bone socket `AttackSocket` missing from any skeleton, causing fallback traces.
- `OnAttackNotify()` failing to detect player capsule during active attack animation.
- Headless Blender script throwing exceptions on bone generation or action keyframing.
