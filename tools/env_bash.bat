@echo off
rem env_bash.bat — ingere bash no PATH e em TIFFANY_BASH (pleno do motor).
if defined BASH_BIN if exist "%BASH_BIN%\bash.exe" (
  set "PATH=%BASH_BIN%;%PATH%"
  set "TIFFANY_BASH=%BASH_BIN%\bash.exe"
  exit /b 0
)
if defined TIFFANY_BASH if exist "%TIFFANY_BASH%" (
  exit /b 0
)
if exist "%ProgramFiles%\Git\usr\bin\bash.exe" (
  set "PATH=%ProgramFiles%\Git\usr\bin;%PATH%"
  set "TIFFANY_BASH=%ProgramFiles%\Git\usr\bin\bash.exe"
  exit /b 0
)
if exist "%ProgramFiles%\Git\bin\bash.exe" (
  set "PATH=%ProgramFiles%\Git\bin;%PATH%"
  set "TIFFANY_BASH=%ProgramFiles%\Git\bin\bash.exe"
  exit /b 0
)
if exist "%ProgramFiles(x86)%\Git\usr\bin\bash.exe" (
  set "PATH=%ProgramFiles(x86)%\Git\usr\bin;%PATH%"
  set "TIFFANY_BASH=%ProgramFiles(x86)%\Git\usr\bin\bash.exe"
  exit /b 0
)
if exist "%LOCALAPPDATA%\Programs\Git\usr\bin\bash.exe" (
  set "PATH=%LOCALAPPDATA%\Programs\Git\usr\bin;%PATH%"
  set "TIFFANY_BASH=%LOCALAPPDATA%\Programs\Git\usr\bin\bash.exe"
  exit /b 0
)
if exist "C:\msys64\usr\bin\bash.exe" (
  set "PATH=C:\msys64\usr\bin;%PATH%"
  set "TIFFANY_BASH=C:\msys64\usr\bin\bash.exe"
  exit /b 0
)
where bash >nul 2>&1 && (
  for /f "delims=" %%I in ('where bash') do (
    set "TIFFANY_BASH=%%I"
    exit /b 0
  )
)
exit /b 1
