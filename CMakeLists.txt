cmake_minimum_required(VERSION 3.19)
project(HH4)
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|Ninja)")


    add_definitions(/DHANDMADE_WIN32=1)

    add_compile_options(
            /W4 /WX
            /wd4201 /wd4100 /wd4189 /wd4068 /wd4505
            # /wd4244 /wd4018 /wd4456 /wd4459
            # /D NDEBUG
            /Ox
            /MTd /GR- /Gm- /EHa- /Oi /FC /Zi /FS
            # /diagnostics:column
    )
    add_link_options(/MAP /incremental:no)


    add_executable(${CMAKE_PROJECT_NAME} WIN32 cpp/code/win32_handmade.cpp)
    target_link_libraries(${CMAKE_PROJECT_NAME}
                          winmm.lib)
    target_link_options(${CMAKE_PROJECT_NAME} PUBLIC /opt:ref /INCREMENTAL:NO)


    include(GenerateExportHeader)
    add_library(handmade SHARED cpp/code/handmade.cpp)
    target_link_options(handmade PUBLIC
                        /EXPORT:GameUpdateAndRender
                        /EXPORT:GameGetSoundSamples
                        /PDB:handmade_pdb%random%.pdb)


    ADD_CUSTOM_COMMAND(TARGET handmade PRE_BUILD COMMAND cmd /c echo Build Lock > lock.tmp)
    ADD_CUSTOM_COMMAND(TARGET handmade POST_BUILD COMMAND cmd /c del lock.tmp)
    ADD_CUSTOM_COMMAND(TARGET handmade PRE_BUILD COMMAND cmd /c del handmade_pdb*)

else ()
    add_definitions(-DHANDMADE_WIN32=0)
    add_compile_options(
            -Wall -Wextra
            -Werror
            -Weverything

            -Wno-string-conversion
            -Wno-sign-conversion
            -Wno-missing-braces
            -Wno-sign-compare
            -Wno-sign-conversion
            -Wno-c++98-compat-pedantic
            -Wno-newline-eof
            -Wno-switch-enum
            -Wno-unused-function

            -Wno-null-dereference
            -Wno-write-strings
            -Wno-old-style-cast
            -Wno-cast-qual
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            -Wno-padded
            -Wno-unused-variable
            -Wno-unused-parameter
            -Wno-pedantic
            -Wno-missing-prototypes
            -Wno-nullable-to-nonnull-conversion
            -Wno-c++11-long-long
            -Wno-format
            -Wno-switch
            -Wno-cast-align
            -Wno-double-promotion
            -Wno-float-conversion
            -Wno-zero-as-null-pointer-constant


            -Wno-unknown-pragmas
            -Wno-undef
            -Dsize_t=__darwin_size_t
    )

    add_executable(${CMAKE_PROJECT_NAME} code/osx_main.mm)
    target_link_libraries(${CMAKE_PROJECT_NAME}
                          "-framework AppKit"
                          "-framework IOKit"
                          "-framework AudioToolbox"
                          "-framework QuartzCore")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/")


    add_library(GameCode SHARED src/handmade.cpp)
    SET_TARGET_PROPERTIES(GameCode PROPERTIES PREFIX "")
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

    target_compile_options(GameCode PUBLIC -g -o -dynamiclib)
    set_target_properties(GameCode PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app/Contents/Resources/")

    #[[    ADD_CUSTOM_COMMAND(TARGET HandmadeHero
                POST_BUILD
                COMMAND bash "../code/copyData.sh"
                )]]

    #    ADD_CUSTOM_COMMAND(TARGET GameCode
    #            POST_BUILD
    #            COMMAND bash "../misc/bundleLIB.sh"
    #            )

    #    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
    #    set(CMAKE_EXE_LINKER_FLAGS "$CMAKE_EXE_LINKER_FLAGS{} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

endif ()