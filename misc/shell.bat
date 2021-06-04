@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\vsdevcmd" -arch=x64 -no_logo
call "subst" "w:" "C:\Users\AgentOfChaos\Desktop\hh2\"
set path=w:\misc;%path%
call "subl"