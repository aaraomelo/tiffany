@echo off
REM auditoria_rg6.bat — bateria RG6 / combinação C0–L7 + M1–M2 + E_∂
REM Label: C0-C8 S0-S4 R0-R4 D0-D3 K0-K7 L0-L7 M1-M2 E_∂
REM Esperado: tests\redes_combinacao.js → #TOTAL 274 0
REM redes.tex §Combinação; lexMax/lexMin não promovidos; C é tipo, Hebb é corolário.
setlocal
call "%~dp0env_node.bat" || exit /b 1
echo === auditoria_rg6: redes_combinacao C0-L7 M1-M2 E_d ===
node "%~dp0..\tests\redes_combinacao.js"
exit /b %ERRORLEVEL%
