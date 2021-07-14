@echo off

cd C:\Users\AgentOfChaos\Desktop\hh2
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\vsdevcmd" -arch=x64 -no_logo
cmake -G "NMake Makefiles"  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

del Makefile
del CMakeCache.txt
del cmake_install.cmake
rd /s /q "CMakeFiles" 

move compile_commands.json build\compile_commands.json