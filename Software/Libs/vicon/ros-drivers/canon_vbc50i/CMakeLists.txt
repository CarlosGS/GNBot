cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# add dynamic reconfigure api
rosbuild_find_ros_package(dynamic_reconfigure)
include(${dynamic_reconfigure_PACKAGE_PATH}/cmake/cfgbuild.cmake)
gencfg()

#uncomment if you have defined messages
rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()

SET(USE_IPP no)
IF(USE_IPP MATCHES yes)
SET(IPP_INC xxx/xxx/xxx/include)
SET(IPP_LIBDIR xxx/xxx/xxx/sharedlib)
SET(IPP_LIBS ippjpeg ippj ippi ipps ippcore )
SET(IPP_DIR libIppJpeg)
SET(IPP_SRC JpegReader-ipp.cpp)
SET(IPP_DEF -DUSE_IPP)
SET(IPP_JPEG IppJpeg)
ENDIF(USE_IPP MATCHES yes)

ADD_DEFINITIONS(${IPP_DEF})

#common commands for building c++ executables and libraries
rosbuild_add_library(${PROJECT_NAME} 
    src/libSock/Socket.cpp
    src/libSock/SocketCom.cpp
    # src/libSock/Serializer.cpp
    src/libCanon/CanonDriver.cpp
    src/libCanon/DgmQueue.cpp
    src/libCanon/JpegReader.cpp
    src/libCanon/Videogram.cpp
    src/libCanon/login.cpp
    src/libCanon/VideoManager.cpp
    ${IPP_SRC}
    src/libCanon/CommManager.cpp
    src/libCanon/JpegStreamSaver.cpp
    src/libCanon/Datagram.cpp
)
target_link_libraries(${PROJECT_NAME} ${IPP_JPEG} jpeg rt)

IF(USE_IPP MATCHES yes)
rosbuild_add_library(${IPP_JPEG}
    src/libIppJpeg/decoder.cpp
    src/libIppJpeg/decqtbl.cpp
    src/libIppJpeg/colorcomp.cpp
    src/libIppJpeg/enchtbl.cpp
    src/libIppJpeg/encqtbl.cpp
    src/libIppJpeg/jpegbase.cpp
    src/libIppJpeg/encoder.cpp
    src/libIppJpeg/dechtbl.cpp
)
target_link_libraries(${IPP_JPEG} ${IPP_LIBS})
ENDIF(USE_IPP MATCHES yes)


#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)

rosbuild_add_executable(${PROJECT_NAME}_node src/node/node.cpp)
target_link_libraries(${PROJECT_NAME}_node ${PROJECT_NAME})
