@echo off
if not exist w:\build mkdir w:\build
pushd w:\build

set CommonCompilerFlags=-nologo -FC -WX -W4 -wd4100 -wd4089 -wd4068 -wd4505 -diagnostics:column -wd4201 -wd4100 -wd4505 -wd4189 -wd4457 -MTd -Oi -Od -GR- -Gm- -EHa- -Zi -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -DHANDMADE_WIN32=1
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

rem 32-bit build
rem cl %CommonCompilerFlags% w:\handmade-hero\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

rem 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches: /O2 /Oi /fp:fast
cl %CommonCompilerFlags% -Fmwin32_handmade.map w:\HandmadeHero\cpp\code\win32_handmade.cpp /link %CommonLinkerFlags%
echo Build Lock > lock.tmp
cl %CommonCompilerFlags% -Fmhandmade.map w:\HandmadeHero\cpp\code\handmade.cpp -LD /link -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples -incremental:no -PDB:handmade_pdb_%random%.pdb
del lock.tmp
popd
