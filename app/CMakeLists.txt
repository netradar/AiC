# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)


set(SRC_DIR src/main/cpp)

include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)
link_directories(${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})
add_library(opus_coder SHARED   src/main/cpp/OpusEncoder.cpp)

add_library(network_vmtl SHARED   src/main/cpp/Network.cpp)

target_link_libraries(network_vmtl
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} android log vmtl )

target_link_libraries(opus_coder
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} android log opus )

