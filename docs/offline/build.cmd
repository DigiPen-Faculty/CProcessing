@echo off
REM SPDX-License-Identifier: MIT
REM Copyright (c) 2026 DigiPen Institute of Technology and CProcessing contributors

setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1" %*
