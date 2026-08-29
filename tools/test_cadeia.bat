@echo off
setlocal
cd /d "%~dp0\.."
if not exist tools\bin mkdir tools\bin

echo [test_cadeia] compila ferramentas...
gcc -O2 -std=c99 -w -Ilib lib\pread_posix.c banco\erg.c -o tools\bin\erg_new.exe || exit /b 1
gcc -O2 -std=c99 -w tools\traduz.c -o tools\bin\traduz.exe || exit /b 1
gcc -O2 -std=c99 -w tools\wasm_erg.c -o tools\bin\wasm_erg.exe || exit /b 1
gcc -O2 -std=c99 -w tools\wasm_sec.c -o tools\bin\wasm_sec.exe || exit /b 1
if not exist tools\bin\test_metal.exe (
  gcc -O2 -std=c99 -w tools\test_metal.c -o tools\bin\test_metal.exe || exit /b 1
)
if not exist tools\bin\traduz_c_asm_node.exe (
  gcc -O2 -std=c99 -w tests\traduz_c_asm_node.c -o tools\bin\traduz_c_asm_node.exe || exit /b 1
)
if not exist tools\bin\traduz_c_asm_shell.exe (
  gcc -O2 -std=c99 -w tests\traduz_c_asm_shell.c -o tools\bin\traduz_c_asm_shell.exe || exit /b 1
)
if not exist tools\bin\traduz_asm_wasm.exe (
  gcc -O2 -std=c99 -w tests\traduz_asm_wasm.c -o tools\bin\traduz_asm_wasm.exe || exit /b 1
)
if not exist tools\bin\celula_wasm_sec.exe (
  gcc -O2 -std=c99 -w tests\celula_wasm_sec.c -o tools\bin\celula_wasm_sec.exe || exit /b 1
)
if not exist tools\bin\corre_fita_metal.exe (
  gcc -O2 -std=c99 -w tests\corre_fita_metal.c -o tools\bin\corre_fita_metal.exe || exit /b 1
)
if not exist tools\bin\cosmologia.exe (
  gcc -O2 -std=c99 -w -Ilib tests\cosmologia.c -o tools\bin\cosmologia.exe || exit /b 1
)

echo [test_cadeia] gera nucleo...
call tools\gera_nucleo.bat all || exit /b 1

echo [test_cadeia] celula_wasm_sec...
tools\bin\celula_wasm_sec.exe || exit /b 1

echo [test_cadeia] corre_fita_metal...
tools\bin\corre_fita_metal.exe || exit /b 1

call tools\env_node.bat >nul 2>&1
if not errorlevel 1 (
  echo [test_cadeia] corre_fita_browser ^(opcional — canonico: corre_fita_metal^)...
  node tests\corre_fita_browser.js || echo [test_cadeia] browser skip — metal OK
) else (
  echo [test_cadeia] corre_fita_browser omitido — call tools\env_node.bat
)

echo [test_cadeia] test_metal...
tools\bin\test_metal.exe || exit /b 1

echo [test_cadeia] traduz_c_asm_node...
tools\bin\traduz_c_asm_node.exe || exit /b 1

echo [test_cadeia] traduz_c_asm_shell...
tools\bin\traduz_c_asm_shell.exe || exit /b 1

echo [test_cadeia] traduz_asm_wasm...
tools\bin\traduz_asm_wasm.exe || exit /b 1

echo [test_cadeia] cosmologia...
tools\bin\cosmologia.exe || exit /b 1

call tools\env_node.bat >nul 2>&1
if not errorlevel 1 (
  echo [test_cadeia] canal_browser...
  node tests\canal_browser.js || exit /b 1
  echo [test_cadeia] canal_chunk...
  node tests\canal_chunk.js || exit /b 1
  echo [test_cadeia] canal_watcher...
  node tests\canal_watcher.js || exit /b 1
  echo [test_cadeia] canal_patria...
  node tests\canal_patria.js || exit /b 1
  echo [test_cadeia] mvp_ponta...
  node tests\mvp_ponta.js || exit /b 1
  echo [test_cadeia] absorve_node...
  node tests\absorve_node.js || exit /b 1
  echo [test_cadeia] duomorf_pipe...
  node tests\duomorf_pipe.js || exit /b 1
  echo [test_cadeia] traduz_c_wasm_shell...
  node tests\traduz_c_wasm_shell.js || exit /b 1
  echo [test_cadeia] traduz_c_wasm_node...
  node tests\traduz_c_wasm_node.js || exit /b 1
  echo [test_cadeia] banco_metal...
  node tests\banco_metal.js || exit /b 1
) else (
  echo [test_cadeia] canal omitido — call tools\env_node.bat
)

echo.
echo === test_cadeia: TUDO OK ===
endlocal
exit /b 0
