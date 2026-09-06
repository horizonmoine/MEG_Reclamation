@echo off
echo ==========================================================
echo  LANCEMENT DU SYSTEME MULTI-AGENTS GOOGLE ANTIGRAVITY
echo  Projet: M.E.G. : RECLAMATION
echo ==========================================================
set "PYTHON_CMD=F:\UE_5.8\Engine\Binaries\ThirdParty\Python3\Win64\python.exe"
if not exist "%PYTHON_CMD%" (
    set PYTHON_CMD=python
)
"%PYTHON_CMD%" "%~dp0orchestrate_meg_team.py"
if /i "%1" neq "/unattended" if /i "%1" neq "-unattended" pause

