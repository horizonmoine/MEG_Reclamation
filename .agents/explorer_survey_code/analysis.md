# Technical Survey & Architectural Analysis: C++ Codebase (R1 to R5)
**Project**: M.E.G. : Reclamation (Unreal Engine 5.8)  
**Date**: 2026-09-06  
**Auditor**: Teamwork Explorer (C++ Codebase & Architecture)  
**Scope**: `Source/MEG_Reclamation/`, `Config/`, `Plugins/`, `MEG_Reclamation.uproject`, and related build files.

---

## 1. Executive Summary

A comprehensive architectural audit of the C++ codebase was conducted across 147 source files, 5 configuration files, and build metadata. The foundation of **M.E.G. : Reclamation** is technically sound with robust architectural modularity, server-authoritative replication, 18 compiled maps, procedural generation algorithms, a custom CRT canvas rendering suite, and 30 passing automation tests.

However, relative to the commercial release requirements **R1 through R5** and the user's **Game Design & Global Coherence Directive** (*Escape the Backrooms* x *Lethal Company* retro-analog immersion), critical architectural and implementation gaps exist:
- **R1 (Skeletal Animation & Bestiary)**: All 9 hostile entities and the player character currently rely entirely on static meshes (`UStaticMeshComponent` / `SM_Hound`, etc.) with zero skeletal meshes, animation blueprints, locomotion blendspaces, or attack anim montages. Damage is applied instantaneously by pure distance check rather than synchronized via AnimNotifies and C++ collision traces/sweeps.
- **R2 (Spatial Voice & Diegetic Radio)**: `ULiminalProximityVoiceComponent` and `AWalkieTalkieTool` have sophisticated mathematical propagation and occlusion logic (multi-trace wall attenuation, UHF frequency simulation), but lack any bridge to Unreal Engine's actual VoIP audio capture/submix engine. Audio spatialization, reverb, and occlusion plugins are unassigned in `DefaultEngine.ini`.
- **R3 (Extraction Loop, Airlock & Economy)**: `ALiminalAirlockActor` provides basic timer-based map switching but lacks physical airlock door sequencing and soundscape integration. `ALiminalTerminalActor` instantly awards items directly to inventory without the required physical pneumatic capsule/freight lift delivery system. `ALiminalDebriefHUD` is fully implemented visually with CRT styling but is completely disconnected from `ALiminalGameMode`. Quota failure accumulates debt but applies no tangible corporate sanctions.
- **R4 (Steam & Online Matchmaking)**: Online subsystems (`OnlineSubsystemSteam` and `OnlineSubsystemNull`) are completely absent from `MEG_Reclamation.uproject`, `MEG_Reclamation.Build.cs`, and `DefaultEngine.ini`. No C++ session hosting/joining, friend invites, or Steam achievements logic exists.
- **R5 (Scalability & Gamepad Navigation)**: `DefaultScalability.ini` does not exist while `DefaultEngine.ini` enables heavy Lumen, Hardware Ray Tracing, and Substrate without mid-range fallback configurations. Gamepad support is fragmented: present in `LiminalMainMenuHUD`, but completely absent in `LiminalPauseMenuComponent`, while Shop Terminal and Tetris Inventory lack visual UI interfaces altogether.

---

## 2. Requirement R1: Skeletal Animation, Bestiary & Damage Traces

### 2.1 Player Character (`AScavengerCharacter` vs `AMEG_ReclamationCharacter`)
- **File**: `Source/MEG_Reclamation/Player/ScavengerCharacter.h` & `.cpp`
- **Current Architecture**:
  - `AScavengerCharacter` inherits from `ACharacter`.
  - First-person view relies on `UCameraComponent* FirstPersonCamera` and `UStaticMeshComponent* FirstPersonToolMesh`.
  - **1P Arms Mesh Gap**: There is NO `USkeletalMeshComponent` for first-person arms (such as `Mesh1P` or `FirstPersonMesh` found in the legacy template class `AMEG_ReclamationCharacter`). Tools are rendered as detached floating static meshes attached to the camera (`FirstPersonToolMesh->SetupAttachment(FirstPersonCamera)`).
  - **3P Body Mesh Gap**: The inherited `GetMesh()` (`USkeletalMeshComponent`) is never initialized, assigned a skeletal mesh, or configured with `bOwnerNoSee = true` / shadow-casting rules in `ScavengerCharacter.cpp`. For multiplayer co-op, third-person player bodies are invisible or unposed.

### 2.2 The 9 Hostile AI Entities
The codebase defines 10 entity types in `EMonsterType` (`Standard`, `Smiler`, `Hound`, `Duller`, `Clump`, `Deathmoth`, `Skinwalker`, `Partygoer`, `Jerry`, `Watcher`, `Wretch`). All 9 required hostiles are mapped to C++ classes:

| Hostile Entity | C++ Class | Base Class | Current Visual Representation | Audio Assets Loaded | Special Mechanics Implemented |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Hound** | `ALiminalEntity` | `ACharacter` | `UStaticMeshComponent* BodyMesh` (`SM_Hound.SM_Hound`) | `S_Hound_Bite`, `S_Hound_Snarl` | Auditory perception, sprint pursuit |
| **Smiler** | `ALiminalEntity_Smiler` | `ALiminalEntity` | `BodyMesh` + 2 `UPointLightComponent` (Eyes) + Mouth Light | `S_Hound_Snarl` (fallback) | Photophobia (charges if illuminated), Stare paralysis in dark |
| **Partygoer** | `ALiminalEntity_Partygoer` | `ALiminalEntity` | `BodyMesh` + `BalloonMesh` (`SM_Balloon`) + `BalloonLight` | `S_Skinwalker_Mimic` | Proximity sanity drain, Scavenger infection (`ServerSetInfected`) |
| **Clump** | `ALiminalEntity_Clump` | `ALiminalEntity` | `BodyMesh` + `USphereComponent* GrabTrigger` | `S_Clump_Gurgle`, `S_Clump_Drag` | Ambient ambush, physical constriction damage per second |
| **Deathmoth** | `ALiminalEntity_Deathmoth` | `ALiminalEntity` | `BodyMesh` + `AbdomenBioluminescence` | `S_Deathmoth_Flutter`, `S_Deathmoth_Screech` | Light attraction (22m), dive bomb charge |
| **Duller** | `ALiminalEntity_Duller` | `ALiminalEntity` | `BodyMesh` (hidden via `SetVisibility(!bIsRevealed)`) | `S_Duller_Growl`, `S_Duller_Rush` | Cloaking/Invisibility; revealed by LIDAR scanner or strobe |
| **Jerry** | `ALiminalEntity_Jerry` | `ALiminalEntity` | `BodyMesh` + `PsionicAura` point light | `S_Jerry_Whisper`, `S_Jerry_Laugh` | Psionic gaze hypnosis, freezes player movement, drains sanity |
| **Skinwalker** | `ALiminalEntity_Skinwalker`| `ALiminalEntity` | `BodyMesh` + `GlitchedHeadlamp` | `S_Skinwalker_Mimic`, `S_Skinwalker_Scream` | Circular RAM voice buffer mimicry (`UVoiceMimicryComponent`) |
| **Wretch** | `ALiminalEntity_Wretch` | `ALiminalEntity` | `BodyMesh` | `S_Wretch_Snarl`, `S_Wretch_Lunge` | Extreme noise sensitivity, blind wander, frenzy sprint |

### 2.3 Animation & Damage Pipeline Gaps (R1)
1. **Static Pose vs Skeletal Mesh**: Every hostile inherits `BodyMesh` (`UStaticMeshComponent`) attached to the root capsule. There are no `USkeletalMeshComponent` setups, no animation instances (`UAnimInstance`), and no locomotion blendspaces (Idle/Walk/Run/Sprint/Aggro).
2. **Instant Damage vs C++ Damage Traces**:
   - `ALiminalEntity::PerformMeleeAttack(AActor* Target)` executes an instantaneous mathematical check:
     ```cpp
     const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
     if (DistSq > FMath::Square(AttackRange + 50.0f)) return false;
     UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
     ```
   - There are NO AnimNotifies triggering attack frames, NO socket-based collision sweeps (`SweepMultiByChannel`), and NO directional trace checks.
3. **Attack Sounds**:
   - Attack sounds (`AttackSound`) are triggered immediately on calling `PerformMeleeAttack` via `UGameplayStatics::PlaySoundAtLocation` rather than being driven diegetically through skeletal animation notifies synchronized with physical impacts.

---

## 3. Requirement R2: Spatial Voice Chat & Diegetic Radio

### 3.1 `ULiminalProximityVoiceComponent`
- **File**: `Source/MEG_Reclamation/Audio/LiminalProximityVoiceComponent.h` & `.cpp`
- **Implementation Status**:
  - Attached to `AScavengerCharacter`.
  - Replicates `bIsSpeaking` and `EVoiceTransmissionMode` (`ProximityOnly`, `RadioBroadcast`, `Muted`).
  - Implements acoustic wall occlusion via `CalculateWallOcclusionTo`: executes `World->LineTraceMultiByChannel` along `ECC_Visibility` from speaker head to listener, calculating `WallCount * SingleWallVolumeLoss` (0.45 per wall).
  - Implements `CanBeHeardBy`: computes spherical distance falloff (`ProximityInnerRadius = 300cm`, `ProximityMaxHearingRadius = 2500cm`), attenuation, and dynamic low-pass frequency lerp (`DirectLineFrequencyHz = 20000Hz` down to `LowPassFrequencyPerWallHz = 850Hz`).
  - Reports auditory alerts to AI (`UAISense_Hearing::ReportNoiseEvent`) so speaking alerts Hounds and Wretches.
- **Architectural Gaps**:
  - The component is a pure logic calculator; it does not connect to Unreal's VoIP system (`IVoiceEngine`, `OnlineVoiceInterface`, or `USoundSubmix`).
  - It contains a transient `TObjectPtr<UAudioComponent> VoiceAudioComponent`, but no audio streaming buffer, microphone input capture, or runtime audio generation is hooked to it.

### 3.2 `WalkieTalkieTool` (Diegetic UHF Radio)
- **File**: `Source/MEG_Reclamation/Tools/WalkieTalkieTool.h` & `.cpp`
- **Implementation Status**:
  - Replicated 8-channel selection (`CurrentChannel` 1 to 8).
  - Battery consumption (`ActiveBatteryDrainPerSecond = 2.0f`).
  - `CalculateSignalClarity`: Distance drop-off over 120m plus line-trace wall penalty (15% signal degradation per structural wall).
  - PTT (Push-To-Talk) click generates physical noise (`MakeNoise`), risking attracting entities.
- **Architectural Gaps**:
  - No DSP audio effect submix (bandpass filter 300Hz-3400Hz) or dynamic noise generator (squelch, white noise, electromagnetic distortion) is instantiated or attached to the audio output bus when transmitting.

### 3.3 Audio Engine Configuration (`Config/DefaultEngine.ini`)
- In `Config/DefaultEngine.ini` lines 46-49:
  ```ini
  SpatializationPlugin=
  SourceDataOverridePlugin=
  ReverbPlugin=
  OcclusionPlugin=
  ```
  All modern UE audio plugins (MetaSound, Resonance Audio, or Unreal Audio Engine built-in spatialization/occlusion) are left blank.

---

## 4. Requirement R3: Extraction Loop, M.E.G. Airlock & Quota Economy

### 4.1 `ALiminalAirlockActor` (Base Alpha <-> Backrooms)
- **File**: `Source/MEG_Reclamation/Objects/LiminalAirlockActor.h` & `.cpp`
- **Current Functionality**:
  - Player interacts with lever -> sets `bIsCycleActive = true`, starts 2.5s timer.
  - Strobe light flashes between amber and red (`StatusLight`).
  - On timeout: `GI->TravelToMission()` loads the destination map.
- **Gaps**:
  - Unidirectional: only handles Base Alpha -> Mission transition. Extraction zone in mission (`AExtractionZone`) uses an instant `GI->ReturnToHub()` without playing the airlock sequence.
  - No physical door movement or animation (uses static frame and lever mesh).
  - No diegetic audio cues (no steam depressurization hiss, air pressure release, or warning klaxons).

### 4.2 Physical Delivery System (Pneumatic Capsule / Freight Lift)
- **File**: `Source/MEG_Reclamation/Objects/LiminalTerminalActor.cpp`
- **Current Behavior**:
  ```cpp
  if (GI->SpendCredits(FoundItem->CostCredits)) {
      Buyer->AddOwnedTool(ToolClass);
      GI->AddStoredTool(ItemId);
  }
  ```
  When an item is purchased at the terminal, it is directly teleported into the player's tool belt and game instance save data.
- **Gaps**:
  - The physical delivery system (pneumatic tube capsule or freight elevator delivering purchased crates/tools into the Hub) is **100% missing**.

### 4.3 `ALiminalDebriefHUD` & Mission Transition Disconnect
- **File**: `Source/MEG_Reclamation/UI/LiminalDebriefHUD.h` & `.cpp`
- **Status**:
  - Exceptionally rich implementation: CRT amber/green/red canvas layout, scanlines, typewriter animation, player stats table (credits, damage received, tools used, survival status), and official M.E.G. stamp animation (`QUOTA ATTEINT` vs `DEFICIT - DETTE`).
  - Supports Enter / Gamepad Face Button Bottom to trigger `GI->ReturnToHub()`.
- **Critical Gap**:
  - In `ALiminalGameMode::Tick` (lines 335-344), when extraction completes or squad wipes, the game mode executes:
    ```cpp
    GI->ReturnToHub();
    ```
  - `ALiminalDebriefHUD` is **never instantiated or set as the active HUD class**. Players are kicked directly to the Hub without seeing the debrief report!

### 4.4 Quota Economy & Sanctions
- **Files**: `Source/MEG_Reclamation/Data/QuotaManager.h` & `LiminalGameInstance.h`
- **Status**: Pure mathematical quota logic in `FQuotaLogic` (`AddDelivered`, `TotalDue`, `IsMet`, `ApplyFailure`) with 100% test coverage.
- **Gaps**:
  - `ApplyFailure` simply increments `OutstandingDebt += (TotalDue - Delivered)`.
  - There are NO corporate sanctions implemented: no equipment confiscation, no hub power outage, no increased store markup, and no game-over contract termination sequence.

---

## 5. Requirement R4: OnlineSubsystems & Steam Integration

### 5.1 Project Build & Plugin Configuration
- **`MEG_Reclamation.Build.cs`**:
  ```csharp
  // Uncomment if you are using online features
  // PrivateDependencyModuleNames.Add("OnlineSubsystem");

  // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file
  ```
  `OnlineSubsystem` is commented out.
- **`MEG_Reclamation.uproject`**:
  Neither `OnlineSubsystem` nor `OnlineSubsystemSteam` is present in the `"Plugins"` section.
- **`Config/DefaultEngine.ini`**:
  No `[OnlineSubsystem]` or `[/Script/OnlineSubsystemSteam.SteamNetDriver]` configuration exists. `SteamDevAppId` is not defined.

### 5.2 C++ Networking & Sessions
- **`ULiminalSessionManager`**: Only provides in-memory stasis for disconnected player pawns and an anti-teleport check.
- **Missing Infrastructure**:
  - No `IOnlineSessionPtr` session creation, finding, or joining methods.
  - No friend invite delegates (`OnSessionUserInviteAccepted`).
  - No Steam P2P socket configuration.
  - No Steam Achievement definitions (neither in C++ nor in engine ini).

---

## 6. Requirement R5: Scalability & Gamepad Navigation

### 6.1 Scalability & Hardware Optimization
- **`Config/DefaultScalability.ini`**: **Does not exist.**
- **`Config/DefaultEngine.ini`**:
  Configured with high-end features forced on by default:
  - `r.ReflectionMethod=1` (Lumen)
  - `r.DynamicGlobalIlluminationMethod=1` (Lumen)
  - `r.RayTracing=True` & `r.RayTracing.RayTracingProxies.ProjectEnabled=True`
  - `r.Shadow.Virtual.Enable=1` (Virtual Shadow Maps)
  - `r.Substrate=True`
  - Target Shader Format: `SM6`
- **Impact on GTX 1060 / RTX 2060**:
  Without custom scalability profiles in `DefaultScalability.ini`, lower quality settings in `UGameUserSettings` still execute expensive Lumen hardware ray-tracing and virtual shadow passes, causing major frame rate drops on mid-range GPUs.

### 6.2 Gamepad Navigation & Input Audit

| Interface / Screen | Implemented In | Gamepad Input Support Status | Deficiencies Identified |
| :--- | :--- | :--- | :--- |
| **Main Menu** | `ALiminalMainMenuHUD` | **Supported** | Uses D-Pad and Face Button Bottom/Right. Fully navigable. |
| **In-Game HUD** | `ALiminalScavengerHUD` | **Partial (Display Only)** | Prompts hardcoded to keyboard (`[E] Ramasser`, `[G] Lancer`); no dynamic gamepad glyphs. |
| **Pause Menu** | `ULiminalPauseMenuComponent` | **Completely Missing** | Only binds `EKeys::Up`, `Down`, `Enter`, `Escape`. Gamepad cannot navigate or close pause. |
| **Shop Terminal** | `ALiminalTerminalActor` | **No UI Exists** | Terminal has no screen menu; pressing E only cycles biome index in world. |
| **Tetris Inventory** | `ULiminalTetrisInventoryComponent` | **No UI Exists** | Logic-only component. Zero canvas or UMG UI widget created. |
| **Debrief Screen** | `ALiminalDebriefHUD` | **Supported** | Listens to `EKeys::Gamepad_FaceButton_Bottom` to return to hub. |
| **Input Mapping** | `Config/DefaultInput.ini` | **Keyboard Only** | ActionMappings contain 0 gamepad button bindings (`Jump`, `Sprint`, `Interact`, `Grab`, etc.). |

---

## 7. Synthesis & Strategic Action Plan

To fulfill the requirements R1-R5 while adhering to the User Game Design Directives, the following architectural additions are recommended:

1. **R1 (Skeletal Meshes & Damage Sync)**:
   - Introduce `USkeletalMeshComponent* Mesh1P` on `AScavengerCharacter` for first-person arm animations.
   - Transition `ALiminalEntity` from `UStaticMeshComponent* BodyMesh` to standard `USkeletalMeshComponent* GetMesh()`, configuring AnimBlueprints with Locomotion BlendSpaces.
   - Refactor `PerformMeleeAttack` to trigger an AnimMontage and delegate damage calculation to an `AnimNotify_MeleeDamageTrace` executing a C++ sphere/box sweep on the attack bone socket.
2. **R2 (Voice & Radio Polish)**:
   - Bind `ULiminalProximityVoiceComponent` to a dedicated `USoundSubmix` audio chain.
   - Enable spatialization and occlusion in `DefaultEngine.ini`.
   - Implement a DSP radio filter (bandpass + static distortion sound cue) when transmitting via `AWalkieTalkieTool`.
3. **R3 (Extraction, Delivery & Debrief)**:
   - Create `ALiminalDeliveryCapsuleActor` or freight lift in Base Alpha spawned on `ServerPurchaseStoreItem`.
   - Update `ALiminalGameMode` to transition to `ALiminalDebriefHUD` before returning to hub.
   - Implement quota failure sanctions in `UQuotaManager` (e.g., locking higher-tier store items, reducing hub lighting, debt penalties).
4. **R4 (Online Subsystems & Steam)**:
   - Enable `OnlineSubsystem` and `OnlineSubsystemSteam` in `.uproject` and `Build.cs`.
   - Configure `[OnlineSubsystem]` and `SteamDevAppId=480` in `DefaultEngine.ini`.
   - Expose session creation and friend invite methods in `ULiminalSessionManager`.
5. **R5 (Scalability & Gamepad Navigation)**:
   - Create `Config/DefaultScalability.ini` disabling ray tracing and switching to screen space techniques on Low/Medium settings.
   - Add gamepad key handling in `ULiminalPauseMenuComponent`.
   - Add gamepad mappings to `Config/DefaultInput.ini`.
   - Build a visual CRT/UMG interface for the Tetris Inventory and Shop Terminal with full D-Pad navigation.
