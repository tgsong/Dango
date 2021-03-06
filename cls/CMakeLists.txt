project(cls VERSION 0.1 LANGUAGES CXX)

add_library(libcls INTERFACE)
enable_all_warnings(libcls)

#---------------------------------------------------------------------------------------------------
# Define headers for this library
#---------------------------------------------------------------------------------------------------
target_include_directories(libcls
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  INTERFACE $<INSTALL_INTERFACE:include>
)

#---------------------------------------------------------------------------------------------------
# List compiler features requirements
#---------------------------------------------------------------------------------------------------
# target_compile_features(cls
#   PRIVATE cxx_auto_type
#   PRIVATE cxx_variadic_templates
# )

# Depend on a library that we defined in the top-level file
target_link_libraries(libcls
  INTERFACE libgsl libthreads
)

#---------------------------------------------------------------------------------------------------
# Specify installation location
#---------------------------------------------------------------------------------------------------
install(TARGETS libcls EXPORT CLSConfig
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT CLSConfig DESTINATION share/CLS)

#---------------------------------------------------------------------------------------------------
# Make project importable from the build directory
#---------------------------------------------------------------------------------------------------
export(TARGETS libcls FILE CLSConfig.cmake)

#---------------------------------------------------------------------------------------------------
# Unit tests
#---------------------------------------------------------------------------------------------------
add_executable(test_cls
  include/cls
  test/main.cpp
  test/test.cpp
  test/example.cpp
)

target_link_libraries(test_cls libcls libcatch)

add_test(test_cls test_cls)