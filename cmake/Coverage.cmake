function(enable_coverage APP_NAME)
  if(softrays_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      target_compile_options(${APP_NAME} PRIVATE -finstrument-functions -pg --coverage)
      target_link_libraries(${APP_NAME} PRIVATE --coverage)
    endif()
  endif()
endfunction()

