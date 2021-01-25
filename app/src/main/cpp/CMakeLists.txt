cmake_minimum_required(VERSION 3.10.2)

project("rtmplearn")

add_subdirectory(librtmp)

message("----------------------")
message(${CMAKE_BUILD_TYPE})
message(${ANDROID_PLATFORM})
message(${CMAKE_SOURCE_DIR})
message(${ANDROID_ABI})

file(GLOB cpp_source *.cpp)
#头文件成功引入
include_directories(${CMAKE_SOURCE_DIR}/x264/${ANDROID_ABI}/include)
#静态库 FFmpeg
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/x264/${ANDROID_ABI}/lib")
#头文件成功引入
include_directories(${CMAKE_SOURCE_DIR}/faac/${ANDROID_ABI}/include)
#静态库 FFmpeg
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/faac/${ANDROID_ABI}/lib")

add_library( native-lib
             SHARED
             ${cpp_source})

find_library( log-lib
              log )

target_link_libraries( native-lib
                       ${log-lib}
                        x264
                        faac
                        rtmp)