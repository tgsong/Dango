cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(Dango VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  $ENV{DEV_LIBS}/cls/cmake
)

function(enable_all_warnings TARGET)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(${TARGET} INTERFACE -Weverything
      -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-global-constructors -Wno-exit-time-destructors)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${TARGET} INTERFACE -Wall -Wextra -pedantic)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(${TARGET} INTERFACE /W4)
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# Include necessary libraries as imported targets
#---------------------------------------------------------------------------------------------------
find_path(GSL_INCLUDE_DIRS gsl PATH_SUFFIXES GSL)
add_library(libgsl INTERFACE IMPORTED)
set_property(TARGET libgsl PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES ${GSL_INCLUDE_DIRS}
)

find_package(Boost 1.59 REQUIRED COMPONENTS program_options timer chrono filesystem)
add_library(libboost INTERFACE IMPORTED)
set_property(TARGET libboost PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS}
)
set_property(TARGET libboost PROPERTY
  INTERFACE_LINK_LIBRARIES Boost::disable_autolinking Boost::program_options Boost::timer Boost::chrono Boost::filesystem
)

# find_package(OpenCV)
# add_library(libopencv INTERFACE IMPORTED)
# set_property(TARGET libopencv PROPERTY
#   INTERFACE_INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS}
# )
# set_property(TARGET libopencv PROPERTY
#   INTERFACE_LINK_LIBRARIES OpenCV::core
# )
# target_link_libraries(<target> ${OpenCV_LIBS})
#
# find_package(Eigen3 REQUIRED)
# add_library(libeigen INTEFACE IMPORTED)
# set_property(TARGET libeigen PROPERTY
#   INTERFACE_INCLUDE_DIRECTORIES ${EIGEN3_INCLUDE_DIR}
# )
#
# find_package(Qt5 COMPONENTS Core Gui Widgets)
# add_library(libqt INTERFACE IMPORTED)
# set_property(TARGET libqt PROPERTY
#  INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_BINARY_DIR}
# )
# set_property(TARGET libqt PROPERTY
#   INTERFACE_LINK_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets
# )
# set_target_properties(qt_app PROPERTIES
#   AUTOMOC ON
#   AUTOUIC ON
#   AUTORCC ON
# )
#
# find_package(TBB REQUIRED)
# add_library(libtbb INTERFACE IMPORTED)
# set_property(TARGET libtbb PROPERTY
#  INTERFACE_INCLUDE_DIRECTORIES ${TBB_INCLUDE_DIRS}
# )
# set_property(TARGET libtbb PROPERTY
#   INTERFACE_LINK_LIBRARIES ${TBB_LIBRARIES}
# )

#---------------------------------------------------------------------------------------------------
# External 3rdparty libs, not part of the interface of any module
#---------------------------------------------------------------------------------------------------
add_subdirectory(3rdparty EXCLUDE_FROM_ALL)

#---------------------------------------------------------------------------------------------------
# Targets that we develop
#---------------------------------------------------------------------------------------------------
add_subdirectory(cls)
add_subdirectory(cls_ex)
add_subdirectory(app)
