param(
    [switch]$SkipLiveExec = $false,
    [ValidateSet('Structural', 'Automation', 'ReleaseSmoke')][string]$Mode = 'Structural',
    [ValidateRange(1, 7200)][int]$TimeoutSeconds = 600
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
Verify-Step "Suite de Tests d'Automatisation (Run_Automation_Tests.ps1)" (Test-Path "F:\MEG_Reclamation\Run_Automation_Tests.ps1")
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

if ($SkipLiveExec) { $Mode = 'Structural' }
if ($Mode -eq 'Automation') {
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'Run_Automation_Tests.ps1') -TimeoutSeconds $TimeoutSeconds
    Verify-Step 'Automation nominative report gate' ($LASTEXITCODE -eq 0)
}
if ($Mode -eq 'ReleaseSmoke') {
    # Process smoke only, not rendered gameplay or release certification.
    foreach ($Executable in @($RELEASE_EXE, $SHIPPING_EXE)) {
        $SmokeProcess = $null
        $Passed = $false
        try {
            $SmokeProcess = Start-Process -FilePath $Executable -ArgumentList '-nullrhi -unattended -benchmark -seconds=2 -log' -WindowStyle Hidden -PassThru
            $null = $SmokeProcess.Handle
            if ($SmokeProcess.WaitForExit($TimeoutSeconds * 1000)) {
                $SmokeProcess.Refresh()
                $Passed = $SmokeProcess.ExitCode -eq 0
            }
        } catch { Write-Warning $_.Exception.Message }
        finally {
            if ($SmokeProcess -and -not $SmokeProcess.HasExited) {
                # Only terminate the process tree created by this smoke run.
                & taskkill.exe /PID $SmokeProcess.Id /T /F 2>$null | Out-Null
            }
        }
        Verify-Step "Process smoke: $Executable" $Passed
    }
}
Write-Host "`n==================================================" -ForegroundColor Cyan
Write-Host "  BILAN: $SuccessCount / $TotalChecks CONTROLES VALIDES" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan

if ($SuccessCount -eq $TotalChecks) {
    Write-Host "[SUCCES] Controles demandes valides. Presence/processus uniquement : aucune certification de jouabilite ou release.`n" -ForegroundColor Green
    exit 0
} else {
    Write-Host "[ALERTE] Certains composants ont echoue a la verification.`n" -ForegroundColor Red
    exit 1
}
