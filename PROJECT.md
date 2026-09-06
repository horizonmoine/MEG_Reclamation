# Project: M.E.G. : Reclamation (Unreal Engine 5.8)

## Architecture
Cooperative procedural horror extraction game (Escape the Backrooms x Lethal Company) in Unreal Engine 5.8.
- **Core Loop**: Base Alpha (Hub) -> Liminal Airlock Incursion -> Procedural Liminal Labyrinths (18 biomes) -> Scrap Collection & Survival -> Airlock Extraction -> M.E.G. Corporate Debriefing & Quota Settlement.
- **Player & Camera**: 1P First Person Camera + 1P Arms (`FirstPersonMesh`, `bOnlyOwnerSee=true`) + 3P Full Body (`GetMesh()`, `bOwnerNoSee=true`) driven by Mannequin Skeleton / Animation Blueprints.
- **Entity System**: `ALiminalEntity` (inheriting `ACharacter`) utilizing Skeletal Meshes with locomotion blendspaces, aggression, attack, and death states. Damage synchronized via AnimNotify C++ trace sweeps (`ECC_Pawn`).
- **Audio Subsystem**: 3D spatial attenuation, dynamic acoustic occlusion (raycasts through walls), reverb volumes, and dedicated UHF radio bandpass submix (`Submix_Radio`, 350-3200 Hz + distortion).
- **Extraction & Store**: `ALiminalAirlockActor` seamless level transitions, `ALiminalFreightLiftActor` pneumatic physical order delivery, `ALiminalDebriefHUD` CRT debt/quota reconciliation.
- **Networking & Online**: `OnlineSubsystemSteam` (AppID 480 fallback to `OnlineSubsystemNull`) for P2P lobby matchmaking, invites, and diegetic Steam achievements.
- **UX & Scalability**: CRT / VHS scanlines aesthetic, 60+ FPS medium preset on GTX 1060 / RTX 2060 via `DefaultScalability.ini`, complete Gamepad navigation.

## Feature Inventory
| # | Feature | Description | Milestone | Source |
|---|---------|-------------|-----------|--------|
| F01 | Scavenger 1P Arms Mesh | Add `USkeletalMeshComponent* FirstPersonMesh` attached to `FirstPersonCamera` with `bOnlyOwnerSee=true` | M1 | Survey (R1) |
| F02 | Scavenger 3P Body & AnimBP | Wire `GetMesh()` to `SKM_Manny_Simple` and `ABP_Unarmed` with `bOwnerNoSee=true` | M1 | Survey (R1) |
| F03 | Entity Skeletal Mesh Migration | Convert `ALiminalEntity` from static mesh to skeletal mesh (`GetMesh()`) | M1 | Survey (R1) |
| F04 | 9 Hostile Entities Animation | Locomotion blendspaces, idle, aggression, attack, and death for Hound, Smiler, Partygoer, Clump, Deathmoth, Duller, Jerry, Skinwalker, Wretch | M1 | Survey (R1) |
| F05 | AnimNotify C++ Damage Sweeps | Synchronize melee damage with physical attack poses using AnimNotifies and C++ collision traces | M1 | Survey (R1) |
| F06 | Attack & Aggression Audio Triggers | Synchronize monster attack sounds and footsteps to animation events | M1 | Survey (R1) |
| F07 | Proximity Voice Polish & Audio Plugins | Enable audio plugins in `DefaultEngine.ini` and bind `ULiminalProximityVoiceComponent` to VoIP attenuation | M2 | Survey (R2) |
| F08 | 3D Spatial Attenuation & Occlusion | Raycast acoustic occlusion and dynamic reverb in rooms | M2 | Survey (R2) |
| F09 | Diegetic Radio Bandpass & Distortion | UHF bandpass filter (350-3200Hz) and static/EM distortion on `WalkieTalkieTool` | M2 | Survey (R2) |
| F10 | Import Unimported Audio Assets | Import 8 WAVs from `RawAudio/` (airlock alarm, decompress, chime, spray, adrenaline, etc.) | M2 | Survey (R2) |
| F11 | Airlock Transitions & Decontamination | Polished `ALiminalAirlockActor` incursion transitions Base Alpha <-> Backrooms with audio and steam FX | M3 | Survey (R3) |
| F12 | Physical Terminal Delivery System | Replace instant inventory granting with `ALiminalFreightLiftActor` pneumatic physical delivery | M3 | Survey (R3) |
| F13 | Debriefing HUD Flow & Corporate Sanctions | Route end of mission through `ALiminalDebriefHUD` with quota accounting and explicit corporate sanctions | M3 | Survey (R3) |
| F14 | OnlineSubsystemSteam Activation | Enable `OnlineSubsystem` and `OnlineSubsystemSteam` in `.uproject`, `Build.cs`, and `DefaultEngine.ini` (AppID 480) | M4 | Survey (R4) |
| F15 | P2P Matchmaking & Session Hosting | Public/private session hosting, browsing, joining, and friend invites | M4 | Survey (R4) |
| F16 | Diegetic Steam Achievements | 10 achievements with CRT monochrome icons and tracking logic | M4 | Survey (R4) |
| F17 | Scalability Profiles (60+ FPS) | Author `Config/DefaultScalability.ini` tuned for GTX 1060 / RTX 2060 | M5 | Survey (R5) |
| F18 | Gamepad Navigation Parity | Implement full gamepad support on Pause Menu, Debrief HUD, and Terminal | M5 | Survey (R5) |
| F19 | Tetris Inventory Visual UI | Visual grid interface for `ULiminalTetrisInventoryComponent` with gamepad navigation | M5 | Survey (R5) |
| F20 | Retro-Analog CRT/VHS Polish | Immersive scanlines, mechanical key sounds, and diegetic HUD styling | M5 | Survey (Directive) |
| F21 | Clean Dev & Shipping Win64 Compilation | Zero errors and zero blocking warnings in Win64 builds | M6 | Survey (Criteria) |
| F22 | 30/30 Automation Tests Validation | 100% pass on `Run_Automation_Tests.ps1` with Exit Code 0 | M6 | Survey (Criteria) |
| F23 | 27/27 Auto-Check Validation | 100% pass on `Run_Auto_Check.ps1` | M6 | Survey (Criteria) |
| F24 | Standalone Shipping Execution | Standalone `Builds/Windows/MEG_Reclamation.exe` launches instantly without crash | M6 | Survey (Criteria) |

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Skeletal Animation & Bestiary | F01, F02, F03, F04, F05, F06 | none | IN_PROGRESS |
| M2 | Proximity Voice & Audio Submixes | F07, F08, F09, F10 | none | PLANNED |
| M3 | Extraction Loop, Airlock & Quota Economy | F11, F12, F13 | M1, M2 | PLANNED |
| M4 | Steam Integration & Matchmaking | F14, F15, F16 | M3 | PLANNED |
| M5 | Visual Polish, Scalability & Gamepad UI | F17, F18, F19, F20 | M1, M3, M4 | PLANNED |
| M6 | Final Acceptance & Release Certification | F21, F22, F23, F24 | M1, M2, M3, M4, M5 | PLANNED |

## Interface Contracts
### Player ↔ Entity
- `AScavengerCharacter` provides `GetMesh()` (3P body) and `FirstPersonMesh` (1P arms).
- `ALiminalEntity::OnAttackNotify()` triggers C++ sphere/box trace sweep along socket `AttackSocket` against `ECC_Pawn`.
- Hit result invokes `UGameplayStatics::ApplyPointDamage` on hit `AScavengerCharacter` with synchronised audio cue.

### Audio ↔ Tools
- `AWalkieTalkieTool::OnTransmitStart()` routes player voice audio through `Submix_Radio` (bandpass filter 350-3200Hz + EM noise) and plays `S_Radio_Click_On`.
- `AWalkieTalkieTool::OnTransmitEnd()` plays `S_Radio_Click_Off` and unroutes submix.

### Terminal ↔ Freight Lift
- `ALiminalTerminalActor::ServerPurchaseStoreItem(ItemId)` checks player credits, deducts price, and calls `ALiminalFreightLiftActor::DeliverItem(ItemId)`.
- `ALiminalFreightLiftActor` plays pneumatic sound, moves freight platform, and spawns the physical `ALootActor` for manual player pickup.

### GameMode ↔ Debrief HUD
- `ALiminalGameMode::HandleMissionExtraction()` saves collected scrap, sets HUD class to `ALiminalDebriefHUD`, and displays corporate performance report before triggering `ReturnToHub()`.

## Code Layout
- `Source/MEG_Reclamation/Player/` : Character classes (`ScavengerCharacter.h/.cpp`)
- `Source/MEG_Reclamation/AI/` : Entities (`LiminalEntity.h/.cpp`, entity subclasses)
- `Source/MEG_Reclamation/Audio/` : Voice and audio (`LiminalProximityVoiceComponent.h/.cpp`)
- `Source/MEG_Reclamation/Objects/` : Interactables (`LiminalAirlockActor.h/.cpp`, `LiminalTerminalActor.h/.cpp`, `LiminalFreightLiftActor.h/.cpp`)
- `Source/MEG_Reclamation/UI/` : Menus and HUD (`LiminalDebriefHUD.h/.cpp`, `LiminalPauseMenuComponent.h/.cpp`, `LiminalTetrisInventoryWidget.h/.cpp`)
- `Source/MEG_Reclamation/Online/` : Sessions and matchmaking (`LiminalSessionManager.h/.cpp`)
- `Content/Characters/` : Skeletal meshes, skeletons, blendspaces, AnimBPs
- `Content/Audio/` : Imported sound waves, sound cues, attenuation assets, submixes
- `Config/` : `DefaultEngine.ini`, `DefaultGame.ini`, `DefaultInput.ini`, `DefaultScalability.ini`
