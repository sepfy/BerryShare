cmake_minimum_required(VERSION 3.1)
project(src)

find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)

file(GLOB SRCS "*.cc")

include_directories(${CMAKE_SOURCE_DIR}/third_party/pear/third_party/media-server/librtp/include/
    ${CMAKE_SOURCE_DIR}/third_party/pear/third_party/libsrtp/include/
    ${CMAKE_SOURCE_DIR}/third_party/pear/third_party/libnice/agent/
    ${CMAKE_SOURCE_DIR}/third_party/pear/src/
    ${GLIB_INCLUDE_DIRS}
    /opt/vc/include/
    ${CMAKE_SOURCE_DIR}/third_party/userland/host_applications/linux/apps/hello_pi/libs/ilclient/
)

link_directories(${CMAKE_SOURCE_DIR}/third_party/pear/third_party/libnice/builddir/nice/
    ${CMAKE_SOURCE_DIR}/third_party/pear/third_party/libsrtp/build/
    ${CMAKE_SOURCE_DIR}/third_party/pear/third_party/media-server/librtp/debug.linux
    ${CMAKE_SOURCE_DIR}/third_party/pear/cmake/src/
    ${CMAKE_SOURCE_DIR}/third_party/userland/host_applications/linux/apps/hello_pi/libs/ilclient/
    /opt/vc/lib/
)

add_executable(berry-share ${SRCS})

target_link_libraries(berry-share pear glib-2.0 ssl crypto nice srtp2 rtp boost_system pthread ilclient bcm_host vcos vcilcs vchiq_arm)
