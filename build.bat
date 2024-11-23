@echo off
setlocal
cd /D "%~dp0"

for %%a in (%*) do set "%%a=1"

set debug_build= /Od /Zi /MD  
set release_build= /O2 /MD
set build_type=%debug_build%

if "%release%" == "1" echo [release] && set build_type=%release_build%

if "%clean%" == "1" rmdir /s /q out

if not exist "out" mkdir out

if "%shader%"=="1" (
    for %%f in (src\*.vert src\*.frag) do (
        glslc "%%f" -o "out\%%~nxf.spv"
    )
    echo compiled shaders
)

if "%ext%" == "1" cl %build_type% /I lib\ /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c src\vma_impl.cpp /Fo:out\vma.obj /Fd:out\vma.pdb && echo "compiled vma"

if "%yk%" == "1" cl /wd4477 /wd4047 /wd4005 /wd4113 /wd4133 /TC /d2cgsummary /Zi /FC %build_type% /I lib\ /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c ./src/main.c /Fo:out\yk.obj /Fd:out\yk.pdb && echo "compiled yk" && link out\yk.obj out\vma.obj /OUT:out\yk.exe /DEBUG user32.lib kernel32.lib gdi32.lib shell32.lib lib\GLFW\glfw3.lib && echo "linked yk"