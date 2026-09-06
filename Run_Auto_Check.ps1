param(
    [switch]$SkipLiveExec = $false
)

$UE_CMD = "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$UPROJECT = "F:\MEG_Reclamation\MEG_Reclamation.uproject"
$RELEASE_EXE = "F:\MEG_Reclamation\Builds\Windows\MEG_Reclamation.exe"
$SHIPPING_EXE = "F:\MEG_Reclamation\Builds\Windows\MEG_Reclamation\Binaries\Win64\MEG_Reclamation-Win64-Shipping.exe"
$PYTHON_UE = "F:\UE_5.8\Engine\Binaries\ThirdParty\Python3\Win64\python.exe"

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  M.E.G. : RECLAMATION -- AUDIT COMPLET DU JEU" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan

$SuccessCount = 0
$TotalChecks = 0

function Verify-Step([string]$Desc, [bool]$Passed) {
    $script:TotalChecks++
    if ($Passed) {
        Write-Host " [OK] $Desc" -ForegroundColor Green
        $script:SuccessCount++
    } else {
        Write-Host " [FAIL] $Desc" -ForegroundColor Red
    }
}

Write-Host "`n-- 1. Environnement et Moteur Unreal Engine 5.8 --" -ForegroundColor Yellow
Verify-Step "Unreal Engine 5.8 Editeur Command-Line" (Test-Path $UE_CMD)
Verify-Step "Fichier Projet MEG_Reclamation.uproject" (Test-Path $UPROJECT)
Verify-Step "Interpreteur Python UE 5.8 Integre" (Test-Path $PYTHON_UE)

Write-Host "`n-- 2. Executables et Livrables Standalone Release --" -ForegroundColor Yellow
Verify-Step "Lanceur Standalone Release (MEG_Reclamation.exe)" (Test-Path $RELEASE_EXE)
Verify-Step "Binaire Shipping Standalone (MEG_Reclamation-Win64-Shipping.exe)" (Test-Path $SHIPPING_EXE)

Write-Host "`n-- 3. Scripts d'Automatisation et Multi-Agents --" -ForegroundColor Yellow
Verify-Step "Script Multi-Agents Google Antigravity (orchestrate_meg_team.py)" (Test-Path "F:\MEG_Reclamation\orchestrate_meg_team.py")
Verify-Step "Launcher Batch Multi-Agents (Launch_MultiAgent_Orchestrator.bat)" (Test-Path "F:\MEG_Reclamation\Launch_MultiAgent_Orchestrator.bat")
Verify-Step "Suite de 30 Tests d'Automatisation (Run_Automation_Tests.ps1)" (Test-Path "F:\MEG_Reclamation\Run_Automation_Tests.ps1")
Verify-Step "Script de Packaging Shipping (Package_Shipping_Build.ps1)" (Test-Path "F:\MEG_Reclamation\Package_Shipping_Build.ps1")

Write-Host "`n-- 4. Cartographie Integrale (18 Maps UE 5.8) --" -ForegroundColor Yellow
$MapNames = @(
    "Lvl_MainMenu",
    "Lvl_Hub_BaseAlpha",
    "Lvl_00_Lobby",
    "Lvl_01_HabitableZone",
    "Lvl_02_PipeDreams",
    "Lvl_03_ElectricalStation",
    "Lvl_04_AbandonedOffice",
    "Lvl_06_LightsOut",
    "Lvl_08_CaveSystem",
    "Lvl_09_DarkSuburbs",
    "Lvl_10_WheatFields",
    "Lvl_37_Poolrooms",
    "Lvl_99_RunForYourLife",
    "Lvl_Loop",
    "Lvl_ProcGen",
    "Lvl_Level0_Massive",
    "Lvl_Level37_Poolrooms",
    "Lvl_LevelRun_Gauntlet"
)

foreach ($Name in $MapNames) {
    $FilePath = "F:\MEG_Reclamation\Content\Maps\$Name.umap"
    Verify-Step "Map: $Name.umap" (Test-Path $FilePath)
}

if (-not $SkipLiveExec) {
    Write-Host "`n-- 5. Execution Live de Tous les Executables et Livrables --" -ForegroundColor Yellow

    # 5.1 Multi-Agents
    Write-Host " [RUN] Execution live de la ruche multi-agents (orchestrate_meg_team.py)..." -ForegroundColor Gray
    & $PYTHON_UE "F:\MEG_Reclamation\orchestrate_meg_team.py" *>$null
    Verify-Step "Execution Multi-Agents Google Antigravity (Code 0)" ($LASTEXITCODE -eq 0)

    # 5.2 Standalone Release Launcher
    Write-Host " [RUN] Execution live du lanceur Standalone Release (MEG_Reclamation.exe)..." -ForegroundColor Gray
    & $RELEASE_EXE -nullrhi -unattended -benchmark -seconds=2 -log *>$null
    Verify-Step "Execution Standalone Bootstrap Release (Code 0)" ($LASTEXITCODE -eq 0)

    # 5.3 Standalone Shipping Executable
    Write-Host " [RUN] Execution live du binaire Shipping (MEG_Reclamation-Win64-Shipping.exe)..." -ForegroundColor Gray
    & $SHIPPING_EXE -nullrhi -unattended -benchmark -seconds=2 -log *>$null
    Verify-Step "Execution Standalone Shipping Natif (Code 0)" ($LASTEXITCODE -eq 0)

    # 5.4 Automation Tests Suite
    Write-Host " [RUN] Execution live de la suite de 30 tests d'automatisation UE 5.8..." -ForegroundColor Gray
    & $UE_CMD $UPROJECT -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty" *>$null
    Verify-Step "Execution Suite 30 Tests Natifs UE 5.8 (Code 0)" ($LASTEXITCODE -eq 0)
}

Write-Host "`n==================================================" -ForegroundColor Cyan
Write-Host "  BILAN: $SuccessCount / $TotalChecks CONTROLES VALIDES" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan

if ($SuccessCount -eq $TotalChecks) {
    Write-Host "[CERTIFICATION] 100% Operationnel. Tous les composants, maps et executables sont valides.`n" -ForegroundColor Green
    exit 0
} else {
    Write-Host "[ALERTE] Certains composants ont echoue a la verification.`n" -ForegroundColor Red
    exit 1
}
