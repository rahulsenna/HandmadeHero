cmake_minimum_required(VERSION 3.17)
project(HandmadeHero)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions(-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1)


if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|Ninja)")

    add_definitions(/DHANDMADE_WIN32=1)

    add_compile_options(
            /W4 /WX
            /wd4201 /wd4100 /wd4189 /wd4068 /wd4505
#            /wd4244 /wd4018 /wd4456 /wd4459
            /MTd /GR- /Gm- /EHa- /Oi /Od /FC /Zi /FS
#            /diagnostics:column
    )
    add_link_options(/MAP /incremental:no)

    add_executable(HandmadeHero WIN32 cpp/code/win32_handmade.cpp)
    target_link_libraries(HandmadeHero winmm.lib)
    target_link_options(HandmadeHero PUBLIC /opt:ref /INCREMENTAL:NO)


    include(GenerateExportHeader)
    add_library(handmade SHARED cpp/code/handmade.cpp)
    target_link_options(handmade PUBLIC /EXPORT:GameUpdateAndRender /EXPORT:GameGetSoundSamples /PDB:handmade_pdb%random%.pdb)


    ADD_CUSTOM_COMMAND(TARGET handmade
            PRE_BUILD
            COMMAND cmd /c echo Build Lock > lock.tmp
            )
    ADD_CUSTOM_COMMAND(TARGET handmade
            POST_BUILD
            COMMAND cmd /c del lock.tmp
            )

    ADD_CUSTOM_COMMAND(TARGET handmade
            PRE_BUILD
            COMMAND cmd /c del handmade_pdb*
            )
endif ()