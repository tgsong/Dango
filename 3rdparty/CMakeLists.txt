project(3rdparty LANGUAGES CXX)

add_library(libcatch INTERFACE)
target_include_directories(libcatch
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Catch/single_include>
)