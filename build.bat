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
    for %%f in (src\shaders\*.vert src\shaders\*.frag) do (
        glslc "%%f" -o "out\%%~nxf.spv"
    )
    echo compiled shaders
)

if "%vma%" == "1" cl %build_type% /I lib\ /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c lib\vma\vma_impl.cpp /Fo:out\vma.obj /Fd:out\vma.pdb && echo "compiled vma"

if "%ladybird%" == "1" cl /wd4477 /wd4047 /wd4005 /wd4113 /wd4133 /TC /d2cgsummary /Zi /FC %build_type% /I lib\ /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma /I src\ -c ./src/ladybird/main.c /Fo:out\ladybird.obj /Fd:out\ladybird.pdb && echo "compiled ladybird" && link out\ladybird.obj out\vma.obj /OUT:out\ladybird.exe /DEBUG user32.lib kernel32.lib gdi32.lib shell32.lib lib\GLFW\glfw3.lib && echo "linked ladybird"