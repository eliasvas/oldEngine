@echo off
set application_name=game.exe

if not exist build mkdir build
pushd build
"../tcc/tcc" -w -lgdi32 -lopengl32 -luser32 -r -c ..\src\win32_main.c -c ..\src\animation.c -c ..\src\renderer.c -c ..\src\dui.c -Idir../src/ -o game.exe
popd