#!/bin/sh

./makebundle.sh ../build/HandmadeHero

CommonCompilerFlags="
-std=gnu++11
-Wall
-Wno-c++11-compat-deprecated-writable-strings
-Wno-tautological-compare
-Wno-null-dereference
-Wno-old-style-cast
-Wno-unused-variable
-Wno-unused-function

-g

-DHANDMADE_SLOW=1
-DHANDMADE_INTERNAL=1
-DCOMPILER_LLVM=1
-DCOMPILER_MSVC=0


-framework AppKit
-framework IOKit
-framework QuartzCore
-framework AudioToolbox

-Wno-unknown-pragmas
-Wno-undef
-Dsize_t=__darwin_size_t
"

mkdir -p ../build
pushd ../build
clang -o ../build/HandmadeHero.app/Contents/Resources/GameCode.dylib ${CommonCompilerFlags} -dynamiclib ../cpp/code/handmade.cpp
clang -o ../build/HandmadeHero.app/HandmadeHero ${CommonCompilerFlags}  ../code/osx_main.mm
cp -R ../data/ ../build/HandmadeHero.app/Contents/Resources
popd