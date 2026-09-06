# Detailed Technical Analysis: Assets, Audio, Content & Content Pipelines

**Project**: M.E.G. : Reclamation (Unreal Engine 5.8)  
**Agent**: teamwork_preview_explorer (Assets & Content Pipelines)  
**Date**: 2026-09-06  
**Working Directory**: `F:/MEG_Reclamation/.agents/explorer_survey_assets_2`  

---

## Executive Summary

A comprehensive architectural and content audit of `M.E.G. : Reclamation` was conducted across all asset folders (`Content/`, `RawAssets/`, `RawAudio/`, `SourceArt/`, `models/`), C++ source code (`Source/MEG_Reclamation/`), project configuration (`Config/`), and automation/import scripts.

The project possesses solid foundations: 18 levels, 42 high-resolution PBR textures, 40 sound assets, comprehensive C++ systems (AI, inventory, sanity, tools, audio subsystem, debrief, terminal), and 30 passing automated tests. However, critical gaps exist between current static asset implementations and the AAA indie game design specifications (R1 to R5) defined in `ORIGINAL_REQUEST.md`:
1. **Bestiary (R1)**: Hostile entities currently use a static mesh (`SM_Hound`) deformed with scales and lights; no animated skeletal meshes, blendspaces, or AnimBPs currently exist for the 9 entities.
2. **Player Character (R1)**: UE Mannequin assets (`SKM_Manny_Simple`, `SK_Mannequin`, `ABP_Unarmed`, `BS_Idle_Walk_Run`) exist in `Content/Characters/Mannequins/`, but `AScavengerCharacter` has not wired separate 1P arms vs 3P replicated body meshes or AnimNotifies in C++.
3. **Audio & Radio (R2)**: 39 SoundWaves and 1 SoundAttenuation exist, but 8 critical gameplay sounds (airlock alarm/decompress/chime, door kicks, spray, adrenaline, heavy lift) sit unimported in `RawAudio/`. Walkie-talkie UHF radio squelch, static noise loop, and bandpass distortion submixes are absent.
4. **Extraction & Hub (R3)**: `LiminalAirlockActor` uses placeholder geometry without steam/decontamination FX or synchronized audio. Store purchases on `LiminalTerminalActor` instantly grant items without the required diegetic physical delivery mechanism (pneumatic capsule / freight lift).
5. **Steam Subsystem & Achievements (R4)**: `OnlineSubsystemSteam` is commented out in `Build.cs`, missing in `DefaultEngine.ini` and `MEG_Reclamation.uproject`; no achievement icons or registrations exist.
6. **UI, Gamepad & Scalability (R5)**: Main Menu has gamepad navigation, but Pause, Debrief, and Terminal lack gamepad parity. No UI exists for the 8x6 Tetris inventory grid. `DefaultScalability.ini` is completely missing despite Lumen/VSM/Substrate being enabled by default.

---

## 1. R1 — Skeletal Meshes, Skeletons, Animations & Animation Blueprints

### 1.1 Scavenger (Player Character)
- **Observed Assets in `Content/Characters/Mannequins/`**:
  - Skeletal Mesh: `SKM_Manny_Simple.uasset` (15.8 MB), `SKM_Quinn_Simple.uasset` (16.2 MB).
  - Skeleton: `SK_Mannequin.uasset` (191 KB).
  - AnimBP: `ABP_Unarmed.uasset` (385 KB).
  - BlendSpace: `BS_Idle_Walk_Run.uasset` (52 KB).
  - Locomotion / Action Animations:
    - Idle: `MM_Idle.uasset` (1.43 MB).
    - Walk (8 directional): `MF_Unarmed_Walk_Fwd`, `Fwd_Left`, `Fwd_Right`, `Left`, `Right`, `Bwd`, `Bwd_Left`, `Bwd_Right`.
    - Jog (8 directional): `MF_Unarmed_Jog_Fwd`, `Fwd_Left`, `Fwd_Right`, `Left`, `Right`, `Bwd`, `Bwd_Left`, `Bwd_Right`.
    - Attacks: `MM_Attack_01`, `MM_Attack_02`, `MM_Attack_03`, `MM_ChargedAttack`.
    - Deaths: `MM_Death_Front_01/02/03`, `MM_Death_Back_01`, `MM_Death_Left_01`, `MM_Death_Right_01`.
  - In `Content/FirstPerson/Anims/`: `ABP_FP_Copy.uasset` (57 KB), `CtrlRig_FPWarp.uasset` (248 KB).
- **C++ Analysis (`Source/MEG_Reclamation/Player/ScavengerCharacter.h/.cpp`)**:
  - `AScavengerCharacter` inherits from `ACharacter` directly (not `AMEG_ReclamationCharacter`).
  - `FirstPersonToolMesh` (`UStaticMeshComponent`) is attached to `FirstPersonCamera`.
  - **Gap**: There is no dedicated `USkeletalMeshComponent* FirstPersonMesh` (arms) attached to `FirstPersonCamera` with `bOnlyOwnerSee = true` and `CastShadow = false`.
  - **Gap**: The inherited `ACharacter::GetMesh()` (3P mesh) does not have `SKM_Manny_Simple` assigned, is not positioned at relative `(0, 0, -90)` / `(0, -90, 0)`, does not have `bOwnerNoSee = true`, and has no `AnimClass` assigned.
  - **Requirement for Implementation**:
    1. Declare `USkeletalMeshComponent* FirstPersonMesh` in `AScavengerCharacter`. Attach to `FirstPersonCamera`. Assign Manny arms mesh and `ABP_FP_Copy` (or `ABP_Unarmed`).
    2. Configure 3P mesh (`GetMesh()`): Assign `SKM_Manny_Simple` and `ABP_Unarmed`. Replicate locomotion velocity, crouching, and death states.
    3. Connect AnimNotifies on `MM_Attack_*` and `MM_Death_*` to play attack sounds and ragdoll/death callbacks.

### 1.2 The 9 Hostile Entities
The 9 entities in `M.E.G. : Reclamation` are:
1. `Hound`
2. `Smiler`
3. `Partygoer`
4. `Clump`
5. `Deathmoth`
6. `Duller`
7. `Jerry`
8. `Skinwalker`
9. `Wretch`
(Additionally, `Watcher` exists in C++ as a 10th entity).

- **Current State of Model & Mesh Assets**:
  - `models/hound.fbx`: Byte-for-byte identical copy of `models/hound.zip` (4,331,376 bytes); parsing via Blender reports `OSError: Invalid header`.
  - `models/hound_extracted/.../Meshy_AI_Backroom_Creature_Enc_0824101528_texture.fbx`: Valid FBX, but Blender inspection confirms:
    - Objects: `[('Mesh_0', 'MESH')]`
    - Armatures: `[]` (0 bones)
    - Actions: `[]` (0 animations)
    - It is a purely static mesh generated by Meshy AI.
  - `Content/Meshes/Hound/SM_Hound.uasset`: Static mesh imported from the above Meshy FBX.
  - `models/import_hound.py` & `models/wire_hound.py`: Explicitly imports as Static Mesh (`SM_Hound`) and sets it on `BodyMesh` (`UStaticMeshComponent`).
  - **Zero FBX files exist** in the repository for Smiler, Partygoer, Clump, Deathmoth, Duller, Jerry, Skinwalker, Wretch.
  - In C++ (`LiminalEntity.h/.cpp` and subclasses):
    - `ALiminalEntity` inherits `ACharacter`, but completely ignores `GetMesh()` (`USkeletalMeshComponent`).
    - Instead, it creates `BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>()` and defaults to `SM_Hound`.
    - `LiminalEntity_Smiler`: Sets `BodyMesh` scale to `0.01` and uses 3 `UPointLightComponent`s (`EyeGlowLeft`, `EyeGlowRight`, `SmileMouthGlow`).
    - `LiminalEntity_Partygoer`: Attaches `SM_Partygoer_Balloon` and scales `SM_Hound`.
    - `LiminalEntity_Clump`: Flattens `SM_Hound` (`Scale3D = (1.8, 1.8, 0.35)`).
    - `LiminalEntity_Deathmoth`: Scales `SM_Hound` (`Scale3D = (1.3, 1.6, 0.6)`) and sets movement mode to Flying.
    - All other entities use default `SM_Hound` with color or scale variations.
- **Pipeline & Architecture Solution to achieve Animated Bestiary**:
  - Blender 5.2.1 is installed and fully functional at `F:\blender\blender.exe`.
  - Two coordinated pipelines should be used:
    1. **Bipedal Entities (Partygoer, Skinwalker, Wretch, Duller, Smiler)**:
       - Leverage `SK_Mannequin` and `SKM_Manny_Simple` with distinct procedural/PBR monster materials and attached thematic props (Partygoer balloon `SM_Partygoer_Balloon`, Smiler void/emissive face mask, Wretch decayed suit).
       - Utilize existing locomotion animations (`BS_Idle_Walk_Run`), attack swipes (`MM_Attack_01/02/03`), and death collapses (`MM_Death_*`).
    2. **Specialized Non-Humanoid Entities (Hound, Deathmoth, Clump, Jerry)**:
       - Rigging & Animation Pipeline via Blender headless script (`F:\blender\blender.exe -b --python pipeline_rig_entities.py`):
         - **Hound**: Quadruped rig (root, pelvis, spine_01/02, neck, head, jaw, 4x shoulder/thigh/knee/ankle/paw). Generate 4 animations: `A_Hound_Idle`, `A_Hound_Run` (gallop cycle), `A_Hound_Bite` (lunge attack), `A_Hound_Death`.
         - **Deathmoth**: Winged insectoid rig (thorax, abdomen, head, 2x antennae, 4x wing bones). Generate: `A_Deathmoth_Hover`, `A_Deathmoth_Dive`, `A_Deathmoth_Screech`, `A_Deathmoth_Death`.
         - **Clump**: Multi-limbed amorphous mass rig (central hub, 8 tentacle/limb chains). Generate: `A_Clump_Writhe`, `A_Clump_Lunge`, `A_Clump_Death`.
         - **Jerry**: Avian rig (body, wings, beak, talons). Generate: `A_Jerry_Perch`, `A_Jerry_Fly`, `A_Jerry_Peck`, `A_Jerry_Death`.
       - Import script via Unreal Engine Python (`UnrealEditor-Cmd.exe`):
         - Import FBX with `options.import_as_skeletal = True`.
         - Generate Skeletons, AnimSequences, and AnimMontages.
    3. **C++ Refactor in `ALiminalEntity`**:
       - Transition from `UStaticMeshComponent* BodyMesh` to `GetMesh()` (`USkeletalMeshComponent`).
       - Add UPROPERTIES:
         ```cpp
         UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Mesh")
         TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh;

         UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
         TSubclassOf<UAnimInstance> DefaultAnimClass;

         UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
         TObjectPtr<UAnimMontage> AttackMontage;

         UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
         TObjectPtr<UAnimMontage> AggroMontage;

         UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
         TObjectPtr<UAnimMontage> DeathMontage;
         ```
       - In `PerformMeleeAttack`: Trigger `PlayAnimMontage(AttackMontage)` with damage triggered via C++ AnimNotify (`UAnimNotify_LiminalAttackDamage`).
       - In `TakeDamage`: Trigger `PlayAnimMontage(DeathMontage)` or activate ragdoll simulation (`GetMesh()->SetSimulatePhysics(true)`).

---

## 2. R2 — Audio Assets, Spatialization, Submixes & Diagetic Radio

### 2.1 Current Audio Inventory
- **`Content/Audio/`**:
  - `SA_LiminalDefault.uasset`: Sound Attenuation asset (Inner Radius: 300cm, Falloff: 2000cm, Natural Sound algorithm).
  - 10 Biome Ambient Loops: `S_Ambient_Lobby`, `HabitableZone`, `PipeDreams`, `Electrical`, `Office`, `Cave`, `DarkSuburbs`, `WheatFields`, `Poolrooms`, `Run`.
  - 18 Monster Cues: `S_Hound_Snarl`, `S_Hound_Bite`, `S_Smiler_Distortion`, `S_Partygoer_Chime`, `S_Clump_Drag/Gurgle`, `S_Deathmoth_Flutter/Screech`, `S_Duller_Growl/Rush`, `S_Jerry_Laugh/Whisper`, `S_Skinwalker_Mimic/Scream`, `S_Watcher_Alert/Hum`, `S_Wretch_Lunge/Snarl`.
  - 11 Gameplay & Physical SFX: `S_Fluorescent_Hum`, `S_Footstep_Carpet_01/02`, `S_Footstep_Concrete_01/02`, `S_Geiger_Click`, `S_Heartbeat_Panic`, `S_Loot_Impact_Heavy/Metal/Plastic`, `S_Terminal_Beep`.
  - **Total in Content/Audio**: 40 assets.
- **Unimported Assets in `RawAudio/`**:
  - `S_Door_Slow_Creak.wav` (1.2s hinge creak)
  - `S_Door_Kick_Breach.wav` (0.85s explosive breach)
  - `S_Spray_AlmondWater.wav` (0.9s pressurized mist hiss)
  - `S_Adrenaline_Inject.wav` (0.85s mechanical snap + surge)
  - `S_Airlock_Alarm.wav` (1.6s 2-tone oscillating horn)
  - `S_Airlock_Decompress.wav` (2.4s pneumatic steam purge + rumble)
  - `S_Airlock_Chime.wav` (1.3s electronic clearance chime)
  - `S_Loot_Heavy_Pickup.wav` (0.65s heavy lift friction + thump)

### 2.2 Missing Audio Assets & Submix Architecture
To fulfill R2 and user design directives:
1. **Missing Audio Wave Generation**:
   - `S_Radio_Squelch_Start.wav`: 0.15s PTT burst click and carrier engage chirp.
   - `S_Radio_Squelch_End.wav`: 0.20s PTT release tail squelch.
   - `S_Radio_Static_Loop.wav`: Seamless 2.0s white noise + electromagnetic modulation loop.
   - `S_CRT_Flyback_Hum.wav`: 60Hz hum with subtle 15.734 kHz flyback resonance.
   - `S_Tape_Rewind_Click.wav`: Mechanical tape motor and latch sounds.
2. **Submix & Effect Hierarchy**:
   - `MasterAudioSubmix`
     - `Submix_World`: Environmental and physical sounds.
       - Connected to `SubmixEffectReverbPreset` controlled by `ULiminalAudioSubsystem::UpdateRoomAcoustics`.
     - `Submix_Voice`: Proximity chat.
     - `Submix_Radio`: Walkie-talkie UHF communication:
       - Bandpass Filter: High-pass at 350 Hz, Low-pass at 3200 Hz.
       - Overdrive / Distortion: Subtle non-linear distortion effect for diégétique analog radio quality.
3. **Sound Attenuation Assets Needed**:
   - `SA_Voice_Proximity`: Custom occlusion curve (low-pass filtering through walls using `ULiminalProximityVoiceComponent::CalculateWallOcclusionTo`).
   - `SA_Monster_Roar`: 3500cm hearing range, high priority.
   - `SA_Airlock_Industrial`: 4000cm hearing range with cone attenuation.

---

## 3. R3 — Airlock Extraction, Pneumatic Delivery & Debriefing UI

### 3.1 Airlock (`LiminalAirlockActor`)
- **Current Observation**:
  - C++ class `ALiminalAirlockActor` uses `SM_Door_Frame` and `SM_Breaker` as placeholder visuals.
  - Cycle lasts 2.5s, flashing amber/red point light before calling `GI->TravelToMission()`.
  - It does not instantiate audio components for `S_Airlock_Alarm`, `S_Airlock_Decompress`, or `S_Airlock_Chime`.
  - It lacks particle / smoke decontamination FX.
- **Requirements**:
  1. Assemble / assign dedicated airlock enclosure mesh or modular frame (`SM_Door_Frame` + left/right animated door leaves `SM_Door_Leaf`).
  2. Implement downward steam spray: Niagara system `NS_Decontamination_Steam` or procedural mist particle component triggered during `bIsCycleActive`.
  3. Audio playback: Play `S_Airlock_Alarm` on cycle start, `S_Airlock_Decompress` during steam venting, and `S_Airlock_Chime` on cycle completion.

### 3.2 Diegetic Delivery System (`LiminalTerminalActor`)
- **Current Observation**:
  - `ALiminalTerminalActor` handles purchasing 9 tools and 2 consumables.
  - `ServerPurchaseStoreItem_Implementation` gives items directly to the player character or charges batteries instantaneously with zero physical animation or actor spawning.
- **Requirements**:
  1. Create a physical delivery actor: `ALiminalFreightLiftActor` or `ALiminalDeliveryPneumaticActor`.
     - Mesh components: Delivery hatch/chute using `SM_SupplyCrate_MEG` or pneumatic delivery cylinder.
     - Spawns in Base Alpha near the terminal.
  2. Purchase Flow:
     - On store purchase, terminal triggers `ALiminalFreightLiftActor::DeliverItem(ItemId, ItemClass)`.
     - Hatch opens with pneumatic hiss audio (`S_Airlock_Decompress`).
     - The tool or consumable (`ABaseTool` or `ALootActor`) appears in the tray for the player to physically pick up.

### 3.3 Debriefing Screen (`LiminalDebriefHUD`)
- **Current Observation**:
  - `ALiminalDebriefHUD` is an existing C++ Canvas HUD with an amber CRT terminal aesthetic, typewriter effect, mission duration, credits vs quota, debt tracking, player stats, and rubber stamp verdict ("QUOTA ATTEINT" vs "DEFICIT DE LA CORPORATION").
- **Requirements**:
  1. Add audio synchronization: Typewriter click (`S_Terminal_Beep` variant) during text reveal, heavy thud impact (`S_Loot_Impact_Heavy`) on stamp appearance, success chime (`S_Airlock_Chime`) on quota pass, buzzer on deficit.
  2. Diegetic Sanctions Display: Explicit text for corporate penalties on failure (e.g. "SANCTION M.E.G. NIVEAU 3 : RETENUE SALARIALE 100% - RISQUE DE DECLASSIFICATION D-CLASS").
  3. Gamepad input support: Allow `Gamepad_FaceButton_Bottom` (A / Cross) to acknowledge report and return to Hub.

---

## 4. R4 — Steam Subsystems, Matchmaking & Diegetic Achievements

### 4.1 Steam Subsystem Configuration
- **Current Observation**:
  - `MEG_Reclamation.Build.cs`: Lines 34-37 have `OnlineSubsystem` commented out.
  - `MEG_Reclamation.uproject`: No `OnlineSubsystemSteam` plugin enabled.
  - `Config/DefaultEngine.ini`: No Steam net driver, platform service, or AppID configured.
- **Required Configuration**:
  1. In `MEG_Reclamation.uproject`:
     ```json
     { "Name": "OnlineSubsystem", "Enabled": true },
     { "Name": "OnlineSubsystemSteam", "Enabled": true },
     { "Name": "OnlineSubsystemUtils", "Enabled": true }
     ```
  2. In `MEG_Reclamation.Build.cs`:
     Add `"OnlineSubsystem"`, `"OnlineSubsystemSteam"`, `"OnlineSubsystemUtils"` to `PublicDependencyModuleNames`.
  3. In `Config/DefaultEngine.ini`:
     ```ini
     [/Script/Engine.GameEngine]
     +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

     [OnlineSubsystem]
     DefaultPlatformService=Steam

     [OnlineSubsystemSteam]
     bEnabled=true
     SteamDevAppId=480
     bVACEnabled=0
     GameServerQueryPort=27015
     bAllowP2PPresence=true
     ```

### 4.2 Diegetic Steam Achievements
10 corporate-themed achievements designed for M.E.G. Reclamation:
| ID | Display Title | Description (FR) | Trigger Condition |
|---|---|---|---|
| `ACH_FIRST_EXTRACTION` | Recrue M.E.G. | Réaliser sa première extraction réussie depuis les Backrooms. | ExtractionZone delivery + Return to Hub |
| `ACH_QUOTA_SURPASSED` | Employé du Mois | Dépasser le quota corporatif de plus de 200%. | QuotaManager quota evaluation |
| `ACH_HOUND_TAMER` | Dompteur Acoustique | Survivre à la traque d'un Hound sans subir de morsure. | Hound aggro ended with 0 damage |
| `ACH_STARE_ABYSS` | Regard dans l'Abysse | Survivre à la charge d'un Smiler en éteignant sa lampe torche. | Smiler charge neutralized by darkness |
| `ACH_PARTY_CRASHER` | Fête Gâchée | Neutraliser un Partygoer avant qu'il n'infecte un membre de l'escouade. | Partygoer killed before infection |
| `ACH_SURVIVE_RUN` | Sprint Mortel | S'échapper vivant du Niveau ! (Run for your life). | Level 99 extraction completed |
| `ACH_SUB_ZERO_SANITY` | Délire Lucide | Réussir une extraction avec moins de 10% de santé mentale. | Extracted with CurrentSanity < 10.0f |
| `ACH_DEEP_EXPLORER` | Cartographe des Limbes | Survivre dans chacun des 11 biomes liminaux. | GameInstance tracks 11 visited biomes |
| `ACH_NO_MAN_LEFT_BEHIND` | Solidarité M.E.G. | Réanimer 3 coéquipiers au cours d'une seule incursion. | ScavengerCharacter RevivePlayer count >= 3 |
| `ACH_DEBT_FREE` | Dette Remboursée | Rembourser intégralement sa dette corporative M.E.G. (10 000 cr). | QuotaManager debt <= 0 |

- **Asset Generation Spec**:
  - 10 monochrome/amber CRT vector icons (256x256 PNG) stored in `SourceArt/Achievements/` and imported to `/Game/Textures/Achievements/`.

---

## 5. R5 — UI Assets, Tetris Inventory, Gamepad Focus & Scalability

### 5.1 Tetris Spatial Inventory UI
- `ULiminalTetrisInventoryComponent` is fully implemented in C++ (8 columns, 6 rows, rotation, weight calculation, cell collision checks), but has **no UI renderer**.
- **Implementation Spec**:
  - Integrate a dedicated UMG widget (`UWBP_TetrisInventory`) or Canvas HUD renderer in `LiminalScavengerHUD`:
    - Grid cell size: 48x48 px.
    - Amber CRT grid lines (`CRT_AmberDim`).
    - Color-coded item footprints:
      - Tools: Cyan (`0.2, 0.8, 1.0`)
      - Valuables/Scrap: Gold/Amber (`1.0, 0.75, 0.2`)
      - Medical/Consumable: Green (`0.2, 1.0, 0.3`)
    - Dynamic Weight & Value summary at the bottom.
    - Full Gamepad navigation:
      - Left Thumbstick / DPad: Move cursor across slots (X: 0..7, Y: 0..5).
      - `Gamepad_FaceButton_Bottom` (A): Pick up / Place item.
      - `Gamepad_FaceButton_Left` (X): Rotate item 90°.
      - `Gamepad_FaceButton_Right` (B): Drop item on ground (`InputActionDropLoot`).

### 5.2 Universal Gamepad Parity across All Menus
| Menu | Current Input | Required Gamepad Implementation |
|---|---|---|
| **Main Menu** (`LiminalMainMenuHUD`) | Mouse & Gamepad DPad / A / B | Fully functional; add analog stick navigation support. |
| **Pause Menu** (`LiminalPauseMenuComponent`) | Keyboard only (W/S/Up/Down/Enter) | Add `Gamepad_DPad_Up/Down`, `Gamepad_FaceButton_Bottom` (Confirm), `Gamepad_Special_Right` (Start / Resume). |
| **Debrief Screen** (`LiminalDebriefHUD`) | Mouse & Space/Enter | Add `Gamepad_FaceButton_Bottom` to return to Hub. |
| **HUD / Field Manual** (`LiminalScavengerHUD`) | Keyboard M/J/Tab | Bind to `Gamepad_Special_Left` (Back/Select/View). |
| **Vending / Store Terminal** | Overlap & Key E | Add Gamepad cursor / button prompts (A to Buy, B to Exit). |

### 5.3 Scalability Profiles (`Config/DefaultScalability.ini`)
Currently, `Config/DefaultScalability.ini` does NOT exist. Unreal Engine defaults to maximum settings (Substrate + Lumen + VSM + Hardware Ray Tracing), which causes severe framerate drops on mid-tier GPUs (GTX 1060 / RTX 2060).

**Required `DefaultScalability.ini` Profile**:
```ini
[PostProcessQuality@1]
r.MotionBlurQuality=0
r.AmbientOcclusionLevels=1
r.DepthOfFieldQuality=1
r.BloomQuality=1

[ShadowQuality@1]
r.LightFunctionQuality=1
r.ShadowQuality=2
r.Shadow.CSM.MaxCascades=2
r.Shadow.Virtual.Enable=0
r.DistanceFieldShadowing=0

[GlobalIlluminationQuality@1]
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.ScreenProbeGather.DownsampleFactor=16
r.Lumen.HardwareRayTracing=0
r.DynamicGlobalIlluminationMethod=1

[ReflectionQuality@1]
r.Lumen.Reflections.Allow=1
r.Lumen.Reflections.HardwareRayTracing=0
r.ReflectionMethod=1

[AntiAliasingQuality@1]
r.AntiAliasingMethod=4
r.TemporalAA.Quality=1
r.TSR.History.ScreenPercentage=67
```
*(With full @0 Low, @1 Medium, @2 High, @3 Epic presets targeting guaranteed 60+ FPS on GTX 1060/RTX 2060).*

---

## 6. Retro-Analog CRT / VHS / M.E.G. Terminal Game Feel Integration

To comply with the User Game Design Directive:
1. **Visual Language**:
   - Amber (P3 phosphor, 590nm) and Green (P1 phosphor, 525nm) monochrome palettes for terminals and HUD readouts.
   - Dynamic VHS bodycam overlay: timestamp (`REC [00:14:28:12]`), red blinking indicator, battery icon with level bars, subtle scanline drift.
   - Low-pass screen curvature / chromatic aberration post-process.
2. **Audio Language**:
   - Mechanical keyboard click / relay switch SFX on every button press.
   - Soft 60Hz transformer hum + 15.734 kHz horizontal deflection coil whine.
   - Diegetic radio filter: 300Hz–3200Hz bandpass with static burst squelches.
3. **Diegetic Immersion**:
   - No floating health numbers or modern fantasy markers; all readouts are physical gauges (analogue pressure needles, cathode ray traces, audible heartbeat and breathing rates).
