:: check readme for build instructions

@echo off
setlocal
cd /D "%~dp0"

for %%a in (%*) do set "%%a=1"

:: Project Wide
set debug_build= /Od /Zi /MD  
set release_build= /O2 /MD
set build_type=%debug_build%
set inc=/I lib\ /I %VULKAN_SDK%\Include\ /I %VULKAN_SDK%\Include\vma /I src\
if "%release%" == "1" set build_type=%release_build%
if "%clean%" == "1" echo cleaned && rmdir /s /q out
:: =======

if not exist "out" mkdir out

echo [%build_type%]

:: shader
if "%shader%"=="1" (
	for %%f in (src\shaders\*.vert src\shaders\*.frag) do (
		glslc "%%f" -o "out\%%~nxf.spv"
	)
	echo compiled shaders
)
:: =======

:: vma
if "%vma%" == "1" cl %build_type% %inc% -c lib\vma\vma_impl.cpp /Fo:out\vma.obj /Fd:out\vma.pdb && echo "compiled vma"
:: =======

:: ladybird
set lb_flags= /wd4477 /wd4047 /wd4005 /wd4113 /wd4133 /TC /d2cgsummary /Zi
set lb_lib= lib\GLFW\glfw3.lib

if "%ladybird%" == "1" cl %build_type% %inc% %lb_flags% -c ./src/ladybird/main.c /Fo:out\ladybird.obj /Fd:out\ladybird.pdb && echo "compiled ladybird" && ^
link out\ladybird.obj out\vma.obj /OUT:out\ladybird.exe /DEBUG %lb_lib% && echo "linked ladybird"
:: =======

:: game
set lb_flags= /wd4477 /wd4047 /wd4005 /wd4113 /wd4133 /TC /d2cgsummary /Zi
set lb_lib= lib\GLFW\glfw3.lib

if "%winter%" == "1" cl %build_type% %inc% %lb_flags% -c ./src/winter/winter_main.c /Fo:out\winter.obj /Fd:out\winter.pdb && echo "compiled winter" && ^
link out\winter.obj out\vma.obj /OUT:out\winter.exe /DEBUG %lb_lib% && echo "linked winter"
:: =======