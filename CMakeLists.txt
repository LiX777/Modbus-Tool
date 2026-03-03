# 设置最低 CMake 版本
cmake_minimum_required(VERSION 3.16)  # Qt5 推荐的最低版本
# 设置项目名称和版本
project(RS485SerialTool VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#message(STATUS "Qt5 Dir: ${Qt5_DIR}")
# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含第三方库目录
include_directories(${CMAKE_SOURCE_DIR}/include/third_party)

# 查找Qt依赖
find_package(Qt5 COMPONENTS Core SerialPort Widgets REQUIRED)

# 添加第三方库链接目录（如果有预编译库）
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(${CMAKE_SOURCE_DIR}/lib/x64)
else()
    link_directories(${CMAKE_SOURCE_DIR}/lib/Win32)
endif()

# 自动处理UI、资源和MOC文件
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
# 添加子目录
add_subdirectory(src)



