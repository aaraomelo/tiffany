@echo off
rem env_node.bat — node no PATH: instalacao, Cursor helper, ou NODE_BIN.
if defined NODE_BIN if exist "%NODE_BIN%\node.exe" (
  set "PATH=%NODE_BIN%;%PATH%"
  exit /b 0
)
where node >nul 2>&1 && exit /b 0
if exist "%ProgramFiles%\nodejs\node.exe" (
  set "PATH=%ProgramFiles%\nodejs;%PATH%"
  exit /b 0
)
if exist "%LOCALAPPDATA%\Programs\cursor\resources\app\resources\helpers\node.exe" (
  set "PATH=%LOCALAPPDATA%\Programs\cursor\resources\app\resources\helpers;%PATH%"
  exit /b 0
)
exit /b 1
