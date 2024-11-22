@echo off
setlocal
cd /D "%~dp0"

for %%a in (%*) do set "%%a=1"

set debug_build= /Od /Zi /MD  
set release_build= /O2 /MD
set build_type=%debug_build%

if "%release%" == "1" echo [release] && set build_type=%release_build%

:: Deletes compiler artifacts and quits
if "%clean%" == "1" (
rm *.exp
rm *.lib
rm *.obj
rm *.pdb
rm *.exe
rm *.rdi
rm *.o
rm *.spv
exit /b
)

if "%shader%"=="1" (
    for %%f in (gpu\*.vert gpu\*.frag) do (
        glslc "%%f" -o "%%~nxf.spv"
    )
    echo compiled shaders
)

if "%ext%" == "1" cl %build_type% /I . /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c src/vma_impl.cpp /Fo:vma.obj && echo "compiled vma"

if "%yk%" == "1" cl /wd4477 /wd4047 /wd4005 /wd4113 /wd4133 /TC /d2cgsummary /Zi /FC %build_type% /I . /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c ./src/main.c /Fo:yk.obj && echo "compiled yk" && link yk.obj vma.obj /OUT:yk.exe user32.lib kernel32.lib gdi32.lib shell32.lib GLFW/glfw3.lib && echo "linked yk"