cmake_minimum_required(VERSION 3.26)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("softrays" LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# Check if the project is the top-level project
set(softrays_IS_TOP_LEVEL_PROJECT FALSE)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(softrays_IS_TOP_LEVEL_PROJECT TRUE)
endif()

# Check if the project is the top-level project
set(softrays_IS_TOP_LEVEL_PROJECT FALSE)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(softrays_IS_TOP_LEVEL_PROJECT TRUE)
endif()

# adjust or override some CMake defaults
include(cmake/OverrideCMakeDefaults.cmake)

# default compiler options and warnings
include(cmake/DefaultCompilerOptionsAndWarnings.cmake)

# static analyzers
include(cmake/StaticAnalyzers.cmake)

# sanitizers
include(cmake/Sanitizers.cmake)

include(cmake/Coverage.cmake)

if(softrays_IS_TOP_LEVEL_PROJECT)
  # Generate compile_commands.json
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  option(softrays_ENABLE_COVERAGE "Enable Coverage for softrays" OFF)
  option(softrays_ENABLE_CATCH2_TESTS "Enable Tests for softrays" ON)
  option(softrays_ENABLE_GLOBS "Enable GLOB sourcing for softrays" ON)
  option(softrays_ENABLE_RAYLIB "Enable Raylib for softrays" ON)
  option(softrays_ENABLE_FLECS "Enable flecs for softrays" OFF)
  option(softrays_BUILD_APPS "Enable App building for softrays" ON)
  option(softrays_STANDALONE "Enable App building for softrays" OFF)
  # create a symbolic link to the compile_commands file:
  file(
    CREATE_LINK
    ${CMAKE_BINARY_DIR}/compile_commands.json
    compile_commands.json
    COPY_ON_ERROR
    SYMBOLIC)
else()
  # disable tests if we're a sub-project
  set(softrays_ENABLE_CATCH2_TESTS FALSE)
  set(softrays_ENABLE_COVERAGE OFF)
  # enable globs if we're a sub-project?
  set(softrays_ENABLE_GLOBS TRUE)
endif()


if(softrays_STANDALONE)
  add_subdirectory(src/)
else()
  add_subdirectory(${CMAKE_PROJECT_NAME}/library/src)
endif()

# Catch2 v3
if(softrays_ENABLE_CATCH2_TESTS)
  # enable testing with CTest
  include(CTest)
  find_package(Catch2 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG devel)
    FetchContent_MakeAvailable(Catch2)
### make all include directories -isystem
    get_target_property(CATCH2_IID Catch2 INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(Catch2 PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CATCH2_IID}")
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  endif()
  include(Catch)

  include(cmake/AddCatch2Tests.cmake)
  if(NOT softrays_STANDALONE)
    add_subdirectory(${CMAKE_PROJECT_NAME}/library/tests)
  else()
    add_subdirectory(tests/)
  endif()
endif()

# Web Configurations
if(${PLATFORM} STREQUAL "Web")
  message("#######${PLATFORM}#####")
  # add emscripten include directory to a .clangd file
  # as we want the LSP to be happy, but not effect the build! (not we include the systroot emscripten, but the system includes to help with linter errors)
  file(WRITE ".clangd" "CompileFlags:\n  Add:\n    - \"-I${EMSDK}/upstream/emscripten/cache/sysroot/include/emscripten\"\n    - \"-I${EMSDK}/upstream/emscripten/system/include/\"")
  # file(WRITE ".clangd" "CompileFlags:\n  Add:\n    - \"-I${EMSDK}/upstream/emscripten/cache/sysroot/include/\"")
  # file(WRITE ".clangd" "CompileFlags:\n  Add:\n    - \"-I${EMSDK}/upstream/emscripten/system/include/\"")

  # include_directories(${EMSDK}/upstream/emscripten/cache/sysroot/include/)
  # add_compile_options(-I${EMSDK}/upstream/emscripten/cache/sysroot/include/emscripten)
  # include_directories(SYSTEM ${EMSDK}/upstream/emscripten/system/include/)
endif()

# raylib:
if(softrays_ENABLE_RAYLIB)
  set(RAYLIB_VERSION 5.0)
  find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
  if(NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
      raylib
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_MakeAvailable(raylib)
    get_target_property(RAYLIB_IID raylib INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(raylib PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${RAYLIB_IID}")
  endif()

# raylib-cpp
  find_package(raylib_cpp QUIET)
  if(NOT raylib_cpp_FOUND)
    if(NOT DEFINED RAYLIB_CPP_VERSION)
      set(RAYLIB_CPP_VERSION v5.0.2)
    endif()
    include(FetchContent)
    FetchContent_Declare(
          raylib_cpp
          GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
          GIT_TAG ${RAYLIB_CPP_VERSION}
    )
    FetchContent_MakeAvailable(raylib_cpp)
    get_target_property(RAYCPP_IID raylib_cpp INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(raylib_cpp PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${RAYCPP_IID}")
  endif()

# Fetch raygui
  FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
  )
  FetchContent_MakeAvailable(raygui)
  
# Include the raygui header path
  include_directories(SYSTEM ${raygui_SOURCE_DIR}/src)
endif()

# flecs:
if(softrays_ENABLE_FLECS)
  set(FLECS_VERSION 4.0.0)
  find_package(flecs ${FLECS_VERSION} QUIET) # QUIET or REQUIRED
  if(NOT flecs_FOUND) # If there's none, fetch and build flecs
    include(FetchContent)
    FetchContent_Declare(
      flecs
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      URL https://github.com/SanderMertens/flecs/archive/refs/tags/v${FLECS_VERSION}.tar.gz
    )
    FetchContent_GetProperties(flecs)
    if(NOT flecs_POPULATED) # Have we downloaded flecs yet?
      set(FETCHCONTENT_QUIET NO)
      FetchContent_Populate(flecs)
      add_subdirectory(${flecs_SOURCE_DIR} ${flecs_BINARY_DIR})
    endif()
  endif()
endif()

if(softrays_BUILD_APPS)
  # loop through each directory and add their sources, etc. to their own executable
  file(GLOB found_folders LIST_DIRECTORIES true ${CMAKE_PROJECT_NAME}/apps/*)
  foreach(folder ${found_folders})
    if(IS_DIRECTORY ${folder})
      message(STATUS "adding folder as app: ${folder}")
      add_subdirectory(${folder}/src/)
      if(softrays_ENABLE_CATCH2_TESTS)
        add_subdirectory(${folder}/tests/)
      endif()
    endif()
  endforeach()
endif()


