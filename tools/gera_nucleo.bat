@echo off
setlocal
cd /d "%~dp0\.."
if not exist tools\bin mkdir tools\bin
if not exist assets\figuras\wasm mkdir assets\figuras\wasm
if not exist tools\bin\erg_new.exe (
  gcc -O2 -std=c99 -w -Ilib lib\pread_posix.c banco\erg.c -o tools\bin\erg_new.exe || exit /b 1
)
set ERG=tools\bin\erg_new.exe
if not exist %ERG% (
  if exist tools\bin\erg.exe set ERG=tools\bin\erg.exe
  if not exist %ERG% exit /b 1
)
if not exist tools\bin\traduz.exe (
  gcc -O2 -std=c99 -w tools\traduz.c -o tools\bin\traduz.exe || exit /b 1
)
if not exist tools\bin\wasm_erg.exe (
  gcc -O2 -std=c99 -w tools\wasm_erg.c -o tools\bin\wasm_erg.exe || exit /b 1
)
if not exist tools\bin\wasm_sec.exe (
  gcc -O2 -std=c99 -w tools\wasm_sec.c -o tools\bin\wasm_sec.exe || exit /b 1
)
if not exist assets\figuras\wasm\isa.wasm (
  tools\bin\traduz.exe tools\isa.c -o assets\figuras\wasm\isa.wasm || exit /b 1
)
set BACK=%1
if "%BACK%"=="" set BACK=all
if /i "%BACK%"=="all" (
  call :gera node || exit /b 1
  call :gera bash || exit /b 1
  call :gera powershell || exit /b 1
) else (
  call :gera %BACK% || exit /b 1
)
endlocal
exit /b 0

:gera
echo [%~1] traduz...
if not exist conecthus\backends\%~1 mkdir conecthus\backends\%~1
tools\bin\traduz.exe conecthus\backends\%~1\interpretar.c -o assets\figuras\wasm\%~1.wasm || exit /b 1
echo [%~1] wasm_erg...
tools\bin\wasm_erg.exe assets\figuras\wasm\%~1.wasm %~1_corre conecthus\backends\%~1\%~1_corre.erg || exit /b 1
echo [%~1] celula.erg (--all)...
tools\bin\wasm_erg.exe assets\figuras\wasm\%~1.wasm --all conecthus\backends\%~1\celula.erg || exit /b 1
echo [%~1] embute celula no wasm...
tools\bin\wasm_sec.exe cadeia %~1 || exit /b 1
echo [%~1] monta...
%ERG% monta conecthus\backends\%~1\%~1_corre.erg conecthus\backends\%~1\%~1_corre.fita.bin || exit /b 1
echo [%~1] ok: conecthus\backends\%~1\%~1_corre.fita.bin
exit /b 0
