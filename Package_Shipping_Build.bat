@echo off
echo ==========================================================
echo  M.E.G. : RECLAMATION ? PACKAGING STANDALONE RELEASE (WIN64)
echo ==========================================================
powershell -ExecutionPolicy Bypass -File "%~dp0Package_Shipping_Build.ps1"
if /i "%1" neq "/unattended" if /i "%1" neq "-unattended" pause
