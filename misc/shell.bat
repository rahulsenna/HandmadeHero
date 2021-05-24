@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\vsdevcmd" -arch=x64 -no_logo
call "subst" "w:" "C:\Users\AgentOfChaos\CLionProjects\Handmade\"
set path=w:\HandmadeHero\misc;%path%
call "subl"