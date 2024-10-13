set(APP_NAME offline)

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

target_link_libraries(${APP_NAME} PRIVATE ${SANITIZER_FLAGS})
target_include_directories(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME})
target_include_directories(${APP_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/apps/${APP_NAME}/include)

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

