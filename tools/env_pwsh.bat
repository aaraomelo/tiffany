@echo off
rem env_pwsh.bat — ingere PowerShell no PATH e em TIFFANY_PWSH (pleno do motor).
if defined TIFFANY_PWSH if exist "%TIFFANY_PWSH%" exit /b 0
if exist "%ProgramFiles%\PowerShell\7\pwsh.exe" (
  set "PATH=%ProgramFiles%\PowerShell\7;%PATH%"
  set "TIFFANY_PWSH=%ProgramFiles%\PowerShell\7\pwsh.exe"
  exit /b 0
)
if exist "%ProgramFiles%\PowerShell\pwsh.exe" (
  set "PATH=%ProgramFiles%\PowerShell;%PATH%"
  set "TIFFANY_PWSH=%ProgramFiles%\PowerShell\pwsh.exe"
  exit /b 0
)
if exist "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" (
  set "PATH=%SystemRoot%\System32\WindowsPowerShell\v1.0;%PATH%"
  set "TIFFANY_PWSH=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
  exit /b 0
)
where pwsh >nul 2>&1 && (
  for /f "delims=" %%I in ('where pwsh') do (
    set "TIFFANY_PWSH=%%I"
    exit /b 0
  )
)
where powershell >nul 2>&1 && (
  for /f "delims=" %%I in ('where powershell') do (
    set "TIFFANY_PWSH=%%I"
    exit /b 0
  )
)
exit /b 1
