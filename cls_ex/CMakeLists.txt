project(cls_ex VERSION 0.1 LANGUAGES CXX)

add_library(libcls_ex INTERFACE)

#---------------------------------------------------------------------------------------------------
# Define headers for this library
#---------------------------------------------------------------------------------------------------
target_include_directories(libcls_ex
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  INTERFACE $<INSTALL_INTERFACE:include>
)

target_link_libraries(libcls_ex
  INTERFACE libcls libboost
)

#---------------------------------------------------------------------------------------------------
# Specify installation location
#---------------------------------------------------------------------------------------------------
install(TARGETS libcls_ex
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

#---------------------------------------------------------------------------------------------------
# Make project importable from the build directory
#---------------------------------------------------------------------------------------------------
export(TARGETS libcls_ex FILE CLSexConfig.cmake)

#---------------------------------------------------------------------------------------------------
# Unit tests
#---------------------------------------------------------------------------------------------------
add_executable(test_cls_ex
  include/cls_ex/allocator.h
  include/cls_ex/deque.h
  include/cls_ex/deque_x.h
  src/allocator.cpp
  test/main.cpp
)

target_link_libraries(test_cls_ex libcls_ex libcatch)