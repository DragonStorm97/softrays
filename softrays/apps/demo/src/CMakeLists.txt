set(APP_NAME demo)

# Whenever this glob's value changes, cmake will rerun and update the build with
# the new/removed files.
if(softrays_ENABLE_GLOBS)
  file(
    GLOB_RECURSE
    SOURCES
    CONFIGURE_DEPENDS
    "*.cpp")
else()
  set(SOURCES "${APP_NAME}.cpp")
endif()
set(SOURCES ${SOURCES})

add_executable(${APP_NAME} ${SOURCES})

# include the library:
target_link_libraries(${APP_NAME} PRIVATE libsoftrays)

if(softrays_ENABLE_RAYLIB)
  #set(raylib_VERBOSE 1)
  target_link_libraries(${APP_NAME} PRIVATE raylib raylib_cpp)

  # Web Configurations
  if(${PLATFORM} STREQUAL "Web")
    # Tell Emscripten to build an example.html file.
    set_target_properties(${APP_NAME} PROPERTIES SUFFIX ".html")

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../resources/)
      target_link_options(${APP_NAME} PUBLIC "--use-preload-plugins")
      target_link_options(${APP_NAME} PUBLIC --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../resources/@resources/)
    endif()

    # target_link_options(${APP_NAME} PUBLIC "-sASSERTIONS=1")
    target_link_options(${APP_NAME} PUBLIC "-sUSE_GLFW=3")
    target_link_options(${APP_NAME} PUBLIC "-sALLOW_MEMORY_GROWTH=1")
    # copy binaries to release folder
    add_custom_command(
      TARGET ${APP_NAME} POST_BUILD
      # COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${APP_NAME}>/${APP_NAME}.html ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/${APP_NAME}.html
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${APP_NAME}>/${APP_NAME}.data ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/${APP_NAME}.data
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${APP_NAME}>/${APP_NAME}.js ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/${APP_NAME}.js
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:${APP_NAME}>/${APP_NAME}.wasm ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/${APP_NAME}.wasm
      COMMENT "Release ${APP_NAME}/${APP_NAME}.html, ${APP_NAME}.js, and ${APP_NAME}.wasm"
      DEPENDS ${APP_NAME})
  endif()

  # Checks if OSX and links appropriate frameworks (Only required on MacOS)
  if(APPLE)
    target_link_libraries(${APP_NAME} "-framework IOKit")
    target_link_libraries(${APP_NAME} "-framework Cocoa")
    target_link_libraries(${APP_NAME} "-framework OpenGL")
  endif()
endif()

if(softrays_ENABLE_FLECS)
  target_link_libraries(${APP_NAME} PRIVATE flecs::flecs_static)
endif()

target_link_libraries(${APP_NAME} PRIVATE ${SANITIZER_FLAGS})
target_include_directories(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME})
target_include_directories(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/apps/${APP_NAME}/include)
target_include_directories(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/external)

set_target_properties(${APP_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(${APP_NAME} PUBLIC cxx_std_23)
target_compile_options(${APP_NAME} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})

enable_coverage(${APP_NAME})

# copy binaries to release folder
add_custom_command(
  TARGET ${APP_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${APP_NAME}> ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/$<TARGET_FILE_NAME:${APP_NAME}>
  # COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${PROJECT_SOURCE_DIR}/release/${APP_NAME}.zip" --format=zip  -- ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/
  COMMENT "Release ${APP_NAME}/${APP_NAME}"
  DEPENDS ${APP_NAME})

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../resources/)
  add_custom_command(
    TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../resources/ ${PROJECT_SOURCE_DIR}/release/raw/${APP_NAME}/resources/
    COMMENT "Release ${APP_NAME}/${APP_NAME}/resources"
    DEPENDS ${APP_NAME})
endif()
