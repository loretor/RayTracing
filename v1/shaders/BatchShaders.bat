@echo off
setlocal enabledelayedexpansion

REM Directory contenente i file .frag e .vert (la stessa in cui si trova lo script)
set SHADER_DIR=%~dp0

REM Vai alla directory degli shader
cd /d "%SHADER_DIR%"

REM Elimina tutti i file .spv nella directory
del /q *.spv

REM Itera attraverso tutti i file .frag e .vert nella directory degli shader
for %%f in (*.frag *.vert) do (
    set "filename=%%~nf"
    set "extension=%%~xf"

    REM Determina il tipo di shader e imposta il nome del file di output
    if "!extension!" == ".frag" (
        set "output=%SHADER_DIR%!filename:~0,-6!Frag.spv"
    ) else if "!extension!" == ".vert" (
        set "output=%SHADER_DIR%!filename:~0,-6!Vert.spv"
    )

    REM Compila il file shader in .spv
    glslc "%%f" -o "!output!"
    
    echo Converted %%f to !output!
)

endlocal
pause

