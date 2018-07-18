cmake_minimum_required (VERSION 3.2)
project (PhotoAlignment)

#############################

find_package(OpenCV REQUIRED)

find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
    include(ExternalProject)
    find_package(Git REQUIRED)
    
    ExternalProject_Add(
        spdlog_git
        PREFIX ${CMAKE_BINARY_DIR}/spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    
    add_library(spdlog INTERFACE)
    
    ExternalProject_Get_Property(spdlog_git source_dir)
    target_include_directories(spdlog INTERFACE "${source_dir}/include")
    
    add_dependencies(spdlog spdlog_git)
    add_library(spdlog::spdlog ALIAS spdlog)
endif()

find_package(clara QUIET)
if (NOT clara_FOUND)
    include(ExternalProject)
    find_package(Git REQUIRED)
    
    ExternalProject_Add(
        clara_git
        PREFIX ${CMAKE_BINARY_DIR}/clara
        GIT_REPOSITORY https://github.com/catchorg/Clara.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    
    add_library(clara INTERFACE)
    
    ExternalProject_Get_Property(clara_git source_dir)
    target_include_directories(clara INTERFACE "${source_dir}/include")
    
    add_dependencies(clara clara_git)
    add_library(clara::clara ALIAS clara)
endif()

set (SRC "PhotoAlignment.cpp")
add_executable(PhotoAlignment ${SRC} "Readme.md")
set(TARGET_NAME PhotoAlignment)
target_link_libraries(${TARGET_NAME} PRIVATE spdlog::spdlog clara::clara ${OpenCV_LIBS})

find_package(OpenMP)
set(USE_OPENMP ON CACHE BOOL "Use OpenMP if found on system")
if(OpenMP_FOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENMP=1)
endif()

## enable c++14
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD_REQUIRED 14)
if(MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "/std:c++14")
endif(MSVC)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(${TARGET_NAME} PRIVATE stdc++fs)
endif(CMAKE_COMPILER_IS_GNUCXX)