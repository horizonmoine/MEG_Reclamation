$UE_CMD = "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$UPROJECT = "F:\MEG_Reclamation\MEG_Reclamation.uproject"

Write-Host "Lancement des 25+ Tests d'Automatisation MEG..." -ForegroundColor Yellow
& $UE_CMD $UPROJECT -ExecCmds="Automation RunTests Project.Functional Tests.MEG; Quit" -unattended -nopause -nullrhi -testexit="Automation Test Queue Empty"
$Code = $LASTEXITCODE

if ($Code -eq 0) {
    Write-Host "[SUCCES] Tous les tests MEG sont valides (Exit Code 0) !" -ForegroundColor Green
} else {
    Write-Host "[ALERTE] Des tests ont echoue ou le processus a quitte avec le code $Code" -ForegroundColor Red
}
exit $Code
