$UAT = "F:\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat"
$UPROJECT = "F:\MEG_Reclamation\MEG_Reclamation.uproject"
$ARCHIVE_DIR = "F:\MEG_Reclamation\Builds\Windows"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  M.E.G. : RECLAMATION — PACKAGING STANDALONE RELEASE" -ForegroundColor Cyan
Write-Host "  Configuration : Shipping (Win64) — Final Release" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

if (-not (Test-Path $UAT)) {
    Write-Host "[ERREUR] RunUAT.bat introuvable a $UAT" -ForegroundColor Red
    exit 1
}

# Never terminate editor sessions: they can contain unsaved work.
$OpenEditors = @(Get-Process | Where-Object { $_.ProcessName -like '*LiveCoding*' -or $_.ProcessName -like '*UnrealEditor*' })
if ($OpenEditors.Count -gt 0) {
    Write-Error 'Packaging blocked: save and close Unreal Editor/Live Coding before retrying. No process was stopped.'
    exit 1
}
& $UAT BuildCookRun -project="$UPROJECT" -noP4 -platform=Win64 -clientconfig=Shipping -cook -build -stage -pak -archive -archivedirectory="$ARCHIVE_DIR" -unattended -utf8output

$Code = $LASTEXITCODE
if ($Code -eq 0) {
    Write-Host "`n[SUCCES] Le jeu est packagé avec succes dans $ARCHIVE_DIR !" -ForegroundColor Green
    Write-Host "Executable produit (validation de lancement distincte) : $ARCHIVE_DIR\MEG_Reclamation.exe" -ForegroundColor Green
} else {
    Write-Host "`n[ECHEC] Le packaging a echoue avec le code $Code" -ForegroundColor Red
}
exit $Code
