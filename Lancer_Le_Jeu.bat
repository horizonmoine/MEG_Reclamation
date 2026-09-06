@echo off
title M.E.G. Reclamation - Launcher
echo ===================================================================
echo   M.E.G. : RECLAMATION - LANCEMENT DU JEU
echo ===================================================================
echo.
echo Lancement du jeu en mode Standalone avec les binaires a jour...
start "" "F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0MEG_Reclamation.uproject" /Game/Maps/Lvl_MainMenu -game -log -ResX=1920 -ResY=1080
exit
