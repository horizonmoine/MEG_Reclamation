# M.E.G. : Reclamation — Build, Test & Automation Infrastructure Survey

**Date**: 2026-09-06  
**Investigator**: `teamwork_preview_explorer` (Build & Test Specialist)  
**Target Engine**: Unreal Engine 5.8.1 (CL-56057345)  
**Target Project**: `F:/MEG_Reclamation/MEG_Reclamation.uproject`

---

## Executive Summary

A full investigation of the build, test, and packaging automation infrastructure for **M.E.G. : Reclamation** was executed. The project is fully functional, healthy, and all automation tooling is operational:
- **30 / 30 Automation Tests** executed cleanly with **Exit Code 0** via `Run_Automation_Tests.ps1`.
- **27 / 27 Static Integrity Controls** verified passing 100% via `Run_Auto_Check.ps1 -SkipLiveExec`.
- **31 / 31 Full Audit Controls** (including live execution of multi-agent Python, Standalone bootstrap launcher, Standalone Shipping binary, and native Unreal automation suite) verified passing with **Exit Code 0**.
- **Shipping Win64 standalone build** is already fully packaged in `Builds/Windows/` (`MEG_Reclamation.exe` launcher + 174.7 MB `MEG_Reclamation-Win64-Shipping.exe` + 871.9 MB IoStore `.ucas`/`.utoc`/`.pak`).

---

## 1. Automation Test Infrastructure (`Run_Automation_Tests.ps1` & `.bat`)

### 1.1 Invocation Architecture & Command Line
The test suite is triggered through `Run_Automation_Tests.bat`, which invokes `Run_Automation_Tests.ps1` with `-ExecutionPolicy Bypass`.

- **Engine Binary**: `F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe`
- **Target Project**: `F:\MEG_Reclamation\MEG_Reclamation.uproject`
- **Command Line**:
  ```powershell
  & $UE_CMD $UPROJECT -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty"
  ```

### 1.2 Test Execution Flags Explained
| Flag | Purpose | Impact on Execution |
|---|---|---|
| `-ExecCmds="..."` | Enqueues automation test filter `Project.Functional Tests.MEG` and queues `Quit` upon completion. | Ensures only project-specific MEG tests run without running the 10,000+ engine tests. |
| `-unattended` | Suppresses all modal UI message boxes and user interaction prompts. | Crucial for automated headless CI/CD execution. |
| `-nopause` | Disables pause prompts on errors or exit. | Prevents script hang in background execution. |
| `-nullrhi` | Bypasses DirectX 12 / Vulkan swapchain initialization and uses the Null dynamic RHI driver (`NullDrv.dll`). | Enables fast, headless execution without consuming GPU VRAM or requiring a physical display. |
| `-testexit="..."` | Instructs the automation controller to terminate the editor process when the test queue empties, setting the process exit code according to test success (0) or failure (non-zero). | Returns deterministic exit codes to PowerShell/Batch callers. |

### 1.3 Complete Enumeration of the 30 Automation Tests
All tests are implemented in `Source/MEG_Reclamation/Tests/MegReclamationTests.cpp` under the category `Project.Functional Tests.MEG.*`. They use `IMPLEMENT_SIMPLE_AUTOMATION_TEST` with flags `EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter`.

| # | Test Name | Source Line | Subsystem / Target Class | Verification Details & Assertions |
|---|---|---|---|---|
| 1 | `Project.Functional Tests.MEG.QuotaManager` | Line 48 | `FQuotaLogic`, `FQuotaState` | Validates delivery accrual, target clamping (`ClampTarget`), goal completion logic (`IsMet`), negative value rejection, and overdelivery debt immunity. |
| 2 | `Project.Functional Tests.MEG.QuotaDebt` | Line 80 | `FQuotaLogic` Debt Carryover | Validates underdelivery debt calculation (`ApplyFailure`), total due accumulation (`TotalDue`), and debt settlement across consecutive cycles. |
| 3 | `Project.Functional Tests.MEG.ProcGenDeterminism` | Line 108 | `FLiminalLayoutBuilder` | Generates two layouts with seed 1234 (24x24 grid, 8 rooms); asserts identical 64-bit hash, identical cell arrays bit-by-bit; asserts different seed (4321) produces distinct hash. |
| 4 | `Project.Functional Tests.MEG.ProcGenConnectivity` | Line 137 | `FLiminalLayoutBuilder` | Loops across seeds 1 to 10; asserts BFS connectivity (`IsEveryRoomConnected() == true`) and room count `>= 2` to ensure zero isolated rooms. |
| 5 | `Project.Functional Tests.MEG.LootItemDataTable` | Line 153 | `FLootItem` (`Data/ItemData.h`) | Validates loot data structure: ItemId (`Scrap_Electronics`), DisplayName, positive weight (`8.5kg`), and positive credits (`120`). |
| 6 | `Project.Functional Tests.MEG.ProcGenRoomPlacement` | Line 171 | `FLiminalLayoutBuilder` Room Bounds | Generates 32x32 layout (seed 999); verifies every room is strictly within interior boundary (`Origin >= 1` and `Origin + Size < Grid - 1`) and min dimensions `>= 3x3`. |
| 7 | `Project.Functional Tests.MEG.SaveData` | Line 192 | `FLiminalSaveData` | Verifies default state (positive bank credits, cycle 1, debt 0), credit addition (+300), expenditure (-150), and zero-floor clamping. |
| 8 | `Project.Functional Tests.MEG.MultiBiomeLayout` | Line 211 | `FLiminalLayoutBuilder` Multi-seed | Generates layouts for seeds 101, 202, 303; verifies room count `>= 3` and 100% room connectivity across varied biome seeds. |
| 9 | `Project.Functional Tests.MEG.AllBiomesIntegrity` | Line 228 | `ULiminalAudioSubsystem`, Map Assets | Validates 11 official lore biomes in `ELevelBiome`; verifies audio profile validity (ambient loop, positive volume, reverb decay); verifies physical existence of all 18 `.umap` map files on disk. |
| 10 | `Project.Functional Tests.MEG.VoiceMimicry` | Line 298 | `UVoiceMimicryComponent`, `FVoiceSnippet` | Verifies CDO ring buffer capacity `>= 40,000` samples; validates `FVoiceSnippet` struct (SpeakerId, 1.5s duration, 48kHz sample count). |
| 11 | `Project.Functional Tests.MEG.SanityTiers` | Line 322 | `LiminalSanity::GetTierFromPercent` | Validates sanity percentage to tier mapping: 100%/75% -> Stable, 65%/55% -> Uneasy, 35%/28% -> Paranoid, 10%/0% -> Psychotic. |
| 12 | `Project.Functional Tests.MEG.Phase2Tools` | Line 339 | Tool Actor CDOs | Verifies CDO parameters for LIDAR (battery 100, range >= 1000, cone > 0), Signal Analyzer (battery 100), Reality Anchor (weight 30kg, radius >= 600, duration >= 30s), Tether (length >= 1000), Chalk (16 marks). |
| 13 | `Project.Functional Tests.MEG.HubProgression` | Line 391 | `ULiminalHubProgressionComponent` | Verifies CDO initial tier (`MakeshiftCamp`) and non-empty display name. |
| 14 | `Project.Functional Tests.MEG.MissionGameMode` | Line 407 | `ALiminalGameMode` | Verifies CDO `DefaultPawnClass == AScavengerCharacter`, mission duration `>= 300s`, collapse timer `>= 300s`, initial MatchState `InMission`. |
| 15 | `Project.Functional Tests.MEG.TerminalStoreCatalog` | Line 425 | `ALiminalTerminalActor` | Verifies CDO store catalog contains `>= 8` articles and contains critical tools/consumables (FlashStrobe, SonicMicrowave, LIDAR, RealityAnchor, Battery, AlmondWater). |
| 16 | `Project.Functional Tests.MEG.SmilerSensory` | Line 453 | `ALiminalEntity_Smiler` | Verifies CDO movement speeds: ChargeSpeed == 850.0f, NormalStalkSpeed == 260.0f. |
| 17 | `Project.Functional Tests.MEG.JerryHypnosisGaze` | Line 469 | `ALiminalEntity_Jerry` | Verifies CDO hypnosis gaze range `>= 800.0f` and sanity drain `>= 8.0/s`. |
| 18 | `Project.Functional Tests.MEG.ScavengerCapabilities` | Line 485 | `AScavengerCharacter` | Verifies CDO carrying capacity (60 kg) and valid stamina percentage. |
| 19 | `Project.Functional Tests.MEG.AssetPipelineGeneration` | Line 502 | `MEG_FullGameAutomator`, Hound Builder | `#if WITH_EDITOR`: Triggers programmatic asset builders and validates existence of `DT_LootItems.uasset`, `BB_Hound.uasset`, `BT_Hound.uasset`, `IMC_Scavenger.uasset`. |
| 20 | `Project.Functional Tests.MEG.MassiveLabyrinthScaling` | Line 519 | ProcGen Stress, Generator CDO | Validates large labyrinth procgen (48x48: >= 16 rooms, > 300 cells; 64x64: >= 20 rooms, > 600 cells); verifies `ALiminalLevelGenerator` CDO presets; checks 4 dedicated map assets on disk. |
| 21 | `Project.Functional Tests.MEG.TacticalPolishAndSpectator` | Line 557 | Spectator, AudioDecoy, BaseEntity CDOs | Verifies `ALiminalSpectatorPawn` follows player by default; verifies `AAudioDecoyTool` and `ALiminalEntity` CDOs are valid. |
| 22 | `Project.Functional Tests.MEG.FieldManualAndAirlock` | Line 581 | HUD, Airlock, Chalk CDOs | Verifies Scavenger HUD manual closed by default; Airlock cycle inactive by default; Chalk trace and tool CDOs valid. |
| 23 | `Project.Functional Tests.MEG.RepoAndEscapeMechanics` | Line 617 | Keypad, Breaker, Adrenaline, VHS, Blackout | Verifies Keypad locked with `"----"`; Breaker unpowered; Adrenaline tool max doses == 2; VHS HUD overlay active; Blackout inactive. |
| 24 | `Project.Functional Tests.MEG.CoopSurvivalAndDowned` | Line 666 | `AScavengerCharacter` State | Verifies player is neither downed nor dead at start, downed agony timer == 45s, damage flash == 0, fake alerts empty. |
| 25 | `Project.Functional Tests.MEG.ConditionalExtractionAndDeathHUD` | Line 685 | `AExtractionZone`, `ALiminalDeathScreenHUD` | Verifies extraction zone unlocked without constraints by default; death screen HUD CDO valid. |
| 26 | `Project.Functional Tests.MEG.ModularProcGenDungeon` | Line 705 | `ULiminalModularDungeonGenerator` | Validates modular 3D dungeon generator determinism (seed 1337), full BFS connectivity, prop socket generation, and spawn-to-extraction Euclidean distance separation (`> 4.0`). |
| 27 | `Project.Functional Tests.MEG.SteamValvePuzzle` | Line 741 | `ALiminalValvePuzzleActor` | Verifies valve puzzle unsolved at start, initial pressure 180 PSI, and outside safe zone (90-110 PSI). |
| 28 | `Project.Functional Tests.MEG.FuseBoxPuzzle` | Line 757 | `ALiminalFuseBoxActor` | Verifies fuse box unpowered at start, and all three fuse slots (0, 1, 2) empty. |
| 29 | `Project.Functional Tests.MEG.WalkieTalkieRadio` | Line 774 | `AWalkieTalkieTool` | Verifies walkie-talkie channel 1 selected by default, not transmitting, and battery at 100%. |
| 30 | `Project.Functional Tests.MEG.BodycamTelemetry` | Line 790 | `ULiminalBodycamComponent` | Verifies bodycam telemetry timestamp format (`1998-*`), resting BPM == 72, battery == 100%, night vision off, status `"NORMAL"`. |

---

## 2. Integrity Audit Controls (`Run_Auto_Check.ps1`)

`Run_Auto_Check.ps1` implements **27 static integrity controls** across 4 categories, plus **4 live runtime checks** in Section 5 (total 31 controls).

### 2.1 Enumeration of All 27 Static Integrity Controls
| Control ID | Category | Target Component / Path | Exact Pass/Fail Condition | Current Status |
|---|---|---|---|---|
| **C01** | 1. Env & Engine | `F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe` | `Test-Path $UE_CMD` returns `$true` | **PASS** |
| **C02** | 1. Env & Engine | `F:\MEG_Reclamation\MEG_Reclamation.uproject` | `Test-Path $UPROJECT` returns `$true` | **PASS** |
| **C03** | 1. Env & Engine | `F:\UE_5.8\Engine\Binaries\ThirdParty\Python3\Win64\python.exe` | `Test-Path $PYTHON_UE` returns `$true` | **PASS** |
| **C04** | 2. Release Deliverables | `F:\MEG_Reclamation\Builds\Windows\MEG_Reclamation.exe` | `Test-Path $RELEASE_EXE` returns `$true` | **PASS** |
| **C05** | 2. Release Deliverables | `F:\MEG_Reclamation\Builds\Windows\MEG_Reclamation\Binaries\Win64\MEG_Reclamation-Win64-Shipping.exe` | `Test-Path $SHIPPING_EXE` returns `$true` | **PASS** |
| **C06** | 3. Automation Scripts | `F:\MEG_Reclamation\orchestrate_meg_team.py` | `Test-Path "F:\MEG_Reclamation\orchestrate_meg_team.py"` | **PASS** |
| **C07** | 3. Automation Scripts | `F:\MEG_Reclamation\Launch_MultiAgent_Orchestrator.bat` | `Test-Path "F:\MEG_Reclamation\Launch_MultiAgent_Orchestrator.bat"` | **PASS** |
| **C08** | 3. Automation Scripts | `F:\MEG_Reclamation\Run_Automation_Tests.ps1` | `Test-Path "F:\MEG_Reclamation\Run_Automation_Tests.ps1"` | **PASS** |
| **C09** | 3. Automation Scripts | `F:\MEG_Reclamation\Package_Shipping_Build.ps1` | `Test-Path "F:\MEG_Reclamation\Package_Shipping_Build.ps1"` | **PASS** |
| **C10** | 4. Full Cartography | `Content\Maps\Lvl_MainMenu.umap` | File exists on disk | **PASS** |
| **C11** | 4. Full Cartography | `Content\Maps\Lvl_Hub_BaseAlpha.umap` | File exists on disk | **PASS** |
| **C12** | 4. Full Cartography | `Content\Maps\Lvl_00_Lobby.umap` | File exists on disk | **PASS** |
| **C13** | 4. Full Cartography | `Content\Maps\Lvl_01_HabitableZone.umap` | File exists on disk | **PASS** |
| **C14** | 4. Full Cartography | `Content\Maps\Lvl_02_PipeDreams.umap` | File exists on disk | **PASS** |
| **C15** | 4. Full Cartography | `Content\Maps\Lvl_03_ElectricalStation.umap` | File exists on disk | **PASS** |
| **C16** | 4. Full Cartography | `Content\Maps\Lvl_04_AbandonedOffice.umap` | File exists on disk | **PASS** |
| **C17** | 4. Full Cartography | `Content\Maps\Lvl_06_LightsOut.umap` | File exists on disk | **PASS** |
| **C18** | 4. Full Cartography | `Content\Maps\Lvl_08_CaveSystem.umap` | File exists on disk | **PASS** |
| **C19** | 4. Full Cartography | `Content\Maps\Lvl_09_DarkSuburbs.umap` | File exists on disk | **PASS** |
| **C20** | 4. Full Cartography | `Content\Maps\Lvl_10_WheatFields.umap` | File exists on disk | **PASS** |
| **C21** | 4. Full Cartography | `Content\Maps\Lvl_37_Poolrooms.umap` | File exists on disk | **PASS** |
| **C22** | 4. Full Cartography | `Content\Maps\Lvl_99_RunForYourLife.umap` | File exists on disk | **PASS** |
| **C23** | 4. Full Cartography | `Content\Maps\Lvl_Loop.umap` | File exists on disk | **PASS** |
| **C24** | 4. Full Cartography | `Content\Maps\Lvl_ProcGen.umap` | File exists on disk | **PASS** |
| **C25** | 4. Full Cartography | `Content\Maps\Lvl_Level0_Massive.umap` | File exists on disk | **PASS** |
| **C26** | 4. Full Cartography | `Content\Maps\Lvl_Level37_Poolrooms.umap` | File exists on disk | **PASS** |
| **C27** | 4. Full Cartography | `Content\Maps\Lvl_LevelRun_Gauntlet.umap` | File exists on disk | **PASS** |

### 2.2 Live Runtime Controls (Section 5)
When run without `-SkipLiveExec`, `Run_Auto_Check.ps1` executes:
- **C28 (5.1)**: `& $PYTHON_UE "F:\MEG_Reclamation\orchestrate_meg_team.py"` -> Returns exit code **0** (**PASS**).
- **C29 (5.2)**: `& $RELEASE_EXE -nullrhi -unattended -benchmark -seconds=2 -log` -> Standalone launcher boots and exits with code **0** (**PASS**).
- **C30 (5.3)**: `& $SHIPPING_EXE -nullrhi -unattended -benchmark -seconds=2 -log` -> Shipping native binary boots and exits with code **0** (**PASS**).
- **C31 (5.4)**: `& $UE_CMD $UPROJECT -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty"` -> Executes all 30 tests, returns exit code **0** (**PASS**).

**Audit Result**: `31 / 31 CONTROLES VALIDES` (100% Operational Certification).

---

## 3. Packaging Pipeline (`Package_Shipping_Build.ps1` & `.bat`)

### 3.1 Script Pipeline Breakdown
`Package_Shipping_Build.bat` calls `Package_Shipping_Build.ps1`.
1. **Prerequisite Check**: Validates `F:\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat`.
2. **Preventive Process Termination**:
   ```powershell
   Get-Process | Where-Object { $_.ProcessName -like "*LiveCoding*" -or $_.ProcessName -like "*UnrealEditor*" } | Stop-Process -Force -ErrorAction SilentlyContinue
   Start-Sleep -Seconds 2
   ```
   *Rationale*: Eliminates file-locking hazards (`MSB3027` / sharing violations) on project DLLs and DDC cache before initiating UBT and cooking.
3. **Packaging Invocation via RunUAT BuildCookRun**:
   ```powershell
   & $UAT BuildCookRun -project="$UPROJECT" -noP4 -platform=Win64 -clientconfig=Shipping -cook -build -stage -pak -archive -archivedirectory="$ARCHIVE_DIR" -unattended -utf8output
   ```

### 3.2 Output Artifacts in `Builds/Windows/`
- `MEG_Reclamation.exe` (171 KB): Lightweight standalone bootstrap launcher.
- `MEG_Reclamation/Binaries/Win64/MEG_Reclamation-Win64-Shipping.exe` (174.7 MB): Monolithic compiled game binary.
- `MEG_Reclamation/Binaries/Win64/MEG_Reclamation-Win64-Shipping.pdb` (243.8 MB): Symbols for post-mortem debugging.
- `MEG_Reclamation/Content/Paks/MEG_Reclamation-Windows.ucas` (871.9 MB): IoStore Zen container holding cooked assets.
- `MEG_Reclamation/Content/Paks/MEG_Reclamation-Windows.utoc` (421 KB): IoStore TOC index.
- `MEG_Reclamation/Content/Paks/MEG_Reclamation-Windows.pak` (11.5 MB): Pak header.
- `MEG_Reclamation/Content/Paks/global.ucas` & `global.utoc`: Engine shared shader and script metadata.
- Third-party runtime dependencies staged: D3D12Core, DirectML, tbb, msquic, NVaftermath, Ogg, Vorbis.

---

## 4. Unreal Engine 5.8 Environment & Tooling Verification

| Component | Path / Detail | Verified Status |
|---|---|---|
| **Engine Root** | `F:\UE_5.8\Engine\` | Fully installed and accessible |
| **Engine Version** | Unreal Engine 5.8.1-56057345 (`++UE5+Release-5.8`) | Verified via log telemetry |
| **UnrealBuildTool** | `F:\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll` | Operational via bundled .NET 10.0 win-x64 SDK |
| **UnrealAutomationTool (UAT)** | `F:\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat` | Operational |
| **Editor Commandlet** | `F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe` | Operational |
| **Editor Executable** | `F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe` | Operational |
| **Python 3 Runtime** | `F:\UE_5.8\Engine\Binaries\ThirdParty\Python3\Win64\python.exe` | Operational |
| **Compiler Toolchain** | MSVC 14.44.35207 (`F:\vs2022\Nouveau dossier\VC\Tools\MSVC\14.44.35207`) | Verified via UBT log |
| **Windows SDK** | Windows SDK 10.0.26100.0 (`C:\Program Files (x86)\Windows Kits\10`) | Verified via UBT log |
| **ISPC Compiler** | Intel SPMD Compiler 1.24.0 (`F:\UE_5.8\Engine\Source\ThirdParty\Intel\ISPC\...`) | Verified via UBT log |
| **Zen Storage Server** | `F:\UE_5.8\Engine\Binaries\Win64\zenserver.exe` (Port 8558) | Active, managing DDC cache |

---

## 5. Potential Bottlenecks & Requirements for 100% Pass

### 5.1 Memory Constraints During Compilation
- **Observation**: During compilation (`shipping_build_test.log`), UBT requested 1.5 GB memory per action against 4.3 GB available physical RAM (31 GB committed out of 36 GB commit limit), which restricted parallel actions to 2 processes (despite 6 physical / 12 logical CPU cores). Total build time was 420.18s (~7 minutes).
- **Requirement**: Keep system memory pressure low during full rebuilds to prevent thrashing or UBA executor throttling.

### 5.2 Process Mutex & Lock Contention
- **Observation**: Running UBT or UAT while `UnrealEditor.exe` or `LiveCodingConsole.exe` is open triggers wait-mutex locks (`Build.bat is already running, waiting for existing script to terminate...`).
- **Mitigation**: Always ensure any active editor or commandlet processes are stopped before executing compilation or packaging scripts (as implemented in `Package_Shipping_Build.ps1`).

### 5.3 Asset Integrity in Tests
- **Observation**: Tests `FMegAllBiomesIntegrityTest` and `FMegMassiveLabyrinthScalingTest` contain explicit hardcoded disk checks for the 18 `.umap` map files in `Content/Maps/`.
- **Requirement**: If any new map is created or an existing map renamed, both `DefaultGame.ini` (`+MapsToCook`), `Run_Auto_Check.ps1` (`$MapNames`), and `MegReclamationTests.cpp` (`All18Maps`) must be synchronized to maintain 100% test pass.

### 5.4 Test Execution Speed
- **Observation**: Once the editor initializes modules (taking ~90-110s on warm start), all 30 tests execute in **0.7 seconds**.
- **Requirement**: Use `-nullrhi -unattended -nopause -testexit="Automation Test Queue Empty"` for fast headless verification.
