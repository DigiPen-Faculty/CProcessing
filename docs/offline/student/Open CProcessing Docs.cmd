@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Start-Offline-Docs.ps1"
if errorlevel 1 pause
