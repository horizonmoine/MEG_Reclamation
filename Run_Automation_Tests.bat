@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0Run_Automation_Tests.ps1"
if /i "%1" neq "/unattended" if /i "%1" neq "-unattended" pause
