@echo off
where /q cl
if ERRORLEVEL 1 (
    call  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)


set application_name=game.exe



if not exist build mkdir build
pushd build
start /b /wait "" "../tcc/tcc" -w -m64 -lgdi32 -lopengl32 -luser32 -r -c ..\src\win32_main.c -c ..\src\animation.c -c ..\src\renderer.c -c ..\src\dui.c -Idir../src/ -o %application_name%
popd