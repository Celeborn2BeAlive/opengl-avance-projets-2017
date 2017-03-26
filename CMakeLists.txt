cmake_minimum_required(VERSION 3.1)

project(OpenGL_Project)

configure_file(cmake/Config.hpp.in ${CMAKE_SOURCE_DIR}/src/Config.hpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLM REQUIRED)
find_package(assimp REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_definitions(-Wall -W -Weffc++)

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${assimp_INCLUDE_DIR}

    third_party/stb/include
    third_party/glad/include

    src/
)


file(
    GLOB_RECURSE
    SRC_FILES
    src/*.cpp src/*.hpp shaders/*.glsl assets/*
)

add_executable(
    OpenGL_Project
    ${SRC_FILES}

    third_party/glad/src/glad.c
)

target_link_libraries(
    OpenGL_Project
    ${SDL2_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${assimp_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

install(
    TARGETS OpenGL_Project
    DESTINATION .
)






#include(c2ba-glsl-shaders)
#include(c2ba-copy-dll-post-build)
#include(c2ba-assets-directory)

#if(MSVC)
#    set(SHADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/shaders)
#    set(ASSET_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/assets)
#else()
#    set(SHADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/shaders)
#    set(ASSET_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/assets)
#endif()

#c2ba_add_shader_directory(${CMAKE_CURRENT_SOURCE_DIR}/lib/shaders ${SHADER_OUTPUT_PATH}/glmlv)
#c2ba_add_assets_directory(${CMAKE_CURRENT_SOURCE_DIR}/lib/assets ${ASSET_OUTPUT_PATH}/glmlv)
#
#install(
#    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/shaders/
#    DESTINATION shaders/glmlv
#    FILES_MATCHING PATTERN "*.glsl"
#)

#set(
#    LIBRARIES
    #
#    glfw
#    glmlv
#)
#
#
#source_group ("glsl" REGULAR_EXPRESSION "*/*.glsl")
#source_group ("third-party" REGULAR_EXPRESSION "third-party/*.*")
#
#file(GLOB APP_DIRECTORIES "apps/*")
#foreach(DIR ${APP_DIRECTORIES})
#    get_filename_component(APP ${DIR} NAME)
#
    #
#
#    c2ba_add_shader_directory(${DIR}/shaders ${SHADER_OUTPUT_PATH}/${APP})
#    c2ba_add_assets_directory(${DIR}/assets ${ASSET_OUTPUT_PATH}/${APP})
#
#    install(
#        DIRECTORY ${DIR}/shaders/
#        DESTINATION shaders/${APP}
#        FILES_MATCHING PATTERN "*.glsl"
#    )
#
#    if(EXISTS ${DIR}/assets/)
#        install(
#            DIRECTORY ${DIR}/assets/
#            DESTINATION assets/${APP}
#        )
#    endif()
#endforeach()