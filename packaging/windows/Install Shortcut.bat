@echo off
REM Create Start Menu and Desktop shortcuts for ETP.
REM
REM Double-click this file. All the actual work happens in
REM install-shortcut.ps1 next to it - keep the two together.
REM
REM To remove the shortcuts again, open a Command Prompt here and run:
REM     "Install Shortcut.bat" -Remove

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install-shortcut.ps1" %*

REM The pause keeps the window open when double-clicked so the user can read
REM the output. Automated callers set ETP_NO_PAUSE=1 to skip it.
if not "%ETP_NO_PAUSE%"=="1" (
    echo.
    pause
)
