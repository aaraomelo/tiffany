@echo off
rem env_node.bat — ingere node no PATH e em TIFFANY_NODE (pleno do motor).
rem Ordem: NODE_BIN, instalacao, helper Cursor. Nao e o PATH do utilizador a autoridade.
if defined NODE_BIN if exist "%NODE_BIN%\node.exe" (
  set "PATH=%NODE_BIN%;%PATH%"
  set "TIFFANY_NODE=%NODE_BIN%\node.exe"
  exit /b 0
)
where node >nul 2>&1 && (
  for /f "delims=" %%I in ('where node') do (
    set "TIFFANY_NODE=%%I"
    exit /b 0
  )
)
if exist "%ProgramFiles%\nodejs\node.exe" (
  set "PATH=%ProgramFiles%\nodejs;%PATH%"
  set "TIFFANY_NODE=%ProgramFiles%\nodejs\node.exe"
  exit /b 0
)
if exist "%LOCALAPPDATA%\Programs\cursor\resources\app\resources\helpers\node.exe" (
  set "PATH=%LOCALAPPDATA%\Programs\cursor\resources\app\resources\helpers;%PATH%"
  set "TIFFANY_NODE=%LOCALAPPDATA%\Programs\cursor\resources\app\resources\helpers\node.exe"
  exit /b 0
)
exit /b 1
