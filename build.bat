@echo off
setlocal
cd /D "%~dp0"

for %%a in (%*) do set "%%a=1"

:: default behaviour is to compile in debug mode
if "%~1"=="" (
set app=1
)

set debug_build= /Od /Zi /MTd 
set release_build= /O2 /MT

if "%debug%" == "1" echo [debug] && set build_type=%debug_build%
if "%release%" == "1" echo [release] && set build_type=%release_build%

if "%build_type%" == "" echo [debug] && set build_type=%debug_build%

:: Deletes compiler artifacts and quits
if "%clean%" == "1" (
rm *.exp
rm *.lib
rm *.obj
rm *.pdb
rm *.exe
rm *.rdi
rm *.o
rm *spv
exit /b
)

if "%shader%" == "1" glslc hello_triangle.vert -o hello_triangle.vert.spv && glslc hello_triangle.frag -o hello_triangle.frag.spv && echo "compiled shaders"

if "%ext%" == "1" cl %build_type% /I . /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c vma.cpp /Fo:vma.obj && echo "compiled vma"

cl  /TC /d2cgsummary /W4 /wd4100 /wd4996 /wd4133 /wd4113 /Zi /FC /Zc:strictStrings- %build_type% /I . /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma -c ./main.c /Fo:yk.obj && echo [compiled yk]
link yk.obj vma.obj /OUT:yk.exe user32.lib kernel32.lib gdi32.lib && echo [linked yk]