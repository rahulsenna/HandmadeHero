cmake_minimum_required(VERSION 3.18)
project(hh2)



#include_directories(code)
#include_directories(cpp/code)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wno-error -Wno-shadow -Wno-bool-conversion -Wno-writable-strings -Wno-null-dereference -Wno-string-compare -Wno-unused-variable -Wno-infinite-recursion -Werror=implicit-function-declaration -Wno-shadow-field-in-constructor-modified -Wno-shadow-ivar -Wuninitialized -Wunused-label -Wunused-lambda-capture)


add_definitions(/DHANDMADE_SLOW=1 /DHANDMADE_INTERNAL=1 /DHANDMADE_WIN32=1)


FILE(GLOB CppFiles  cpp/code/*.cpp)
FILE(GLOB HFiles  cpp/code/*.h)

add_executable(${CMAKE_PROJECT_NAME} 
cpp/code/win32_handmade.cpp
cpp/code/handmade.cpp
cpp/code/handmade_entity.cpp
cpp/code/handmade_render_group.cpp
cpp/code/handmade_world.cpp
)

set(ForceIncludes 
/FIhandmade_platform.h
/FIwin32_handmade.h
/FIhandmade_intrinsics.h
/FIhandmade_math.h
/FIhandmade_world.h
/FIhandmade_sim_region.h
/FIhandmade_entity.h
/FIhandmade_render_group.h
/FIhandmade.h
)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC 
/FIGL/glew.h /FIGLFW/glfw3.h
/FIwindows.h
${ForceIncludes}
# /FIhandmade_random.h
)


add_executable(${CMAKE_PROJECT_NAME}2 cpp/code/handmade_sim_region.cpp)
target_compile_options(${CMAKE_PROJECT_NAME}2 PUBLIC 
${ForceIncludes}
/FIhandmade_world.cpp
)