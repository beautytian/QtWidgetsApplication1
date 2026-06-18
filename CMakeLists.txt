cmake_minimum_required(VERSION 3.16)
project(QtWidgetsApplication1 LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)      # 自动处理 Qt 的 MOC
set(CMAKE_AUTOUIC ON)      # 自动处理 .ui 文件
set(CMAKE_AUTORCC ON)      # 自动处理 .qrc 资源文件

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找 Qt6 或 Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)




# 方式1: 让 CMake 自动查找（优先）
find_package(GDAL QUIET)

# 方式2: 如果自动查找失败，尝试从环境变量读取路径
if(NOT GDAL_FOUND)
    message(WARNING "GDAL not found by CMake, trying environment variables...")
    
    if(DEFINED ENV{GDAL_ROOT})
        # Windows 风格：用户设置了 GDAL_ROOT 环境变量
        set(GDAL_INCLUDE_DIR "$ENV{GDAL_ROOT}/include")
        set(GDAL_LIBRARY "$ENV{GDAL_ROOT}/lib/gdal_i.lib")
        message(STATUS "Using GDAL from GDAL_ROOT: $ENV{GDAL_ROOT}")
    elseif(DEFINED ENV{GDAL_DIR})
        # Linux 风格：用户设置了 GDAL_DIR 环境变量
        set(GDAL_INCLUDE_DIR "$ENV{GDAL_DIR}/include")
        set(GDAL_LIBRARY "$ENV{GDAL_DIR}/lib/libgdal.so")
        message(STATUS "Using GDAL from GDAL_DIR: $ENV{GDAL_DIR}")
    else()
        # 方式3: 如果都没有，报错并提示用户
        message(FATAL_ERROR 
            "GDAL not found! Please:\n"
            "  - Windows: install via vcpkg or set GDAL_ROOT environment variable\n"
            "  - Linux: sudo apt install libgdal-dev\n"
            "  - Or set GDAL_DIR/GDAL_ROOT environment variable to GDAL installation path"
        )
    endif()
endif()
# set(GDAL_INCLUDE_DIR "D:/gdal-2.3.1/include" CACHE PATH "" FORCE)
# set(GDAL_LIBRARY "D:/gdal-2.3.1/lib/gdal_i.lib" CACHE FILEPATH "" FORCE)

# 查找 GDAL

# 设置源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
    HGTReader.cpp
    HGTCollection.cpp
    GeoidQuery.cpp
)

set(HEADERS
    MainWindow.h
    HGTReader.h
    HGTCollection.h
    GeoidQuery.h
)

# 如果有 UI 文件，可以添加
# set(UI_FILES
#     src/MainWindow.ui
# )

# 如果有资源文件
# set(RESOURCES
#     dem_query_qt.qrc
# )

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    # ${UI_FILES}
    # ${RESOURCES}
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GDAL_INCLUDE_DIR}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    ${GDAL_LIBRARY}
)


# ============================================
# 统一编码为 UTF-8（跨平台）
# ============================================
# Windows 上强制使用 UTF-8 编码
if(MSVC)
    # Visual Studio：使用 /utf-8 选项（MSVC 2015+ 支持）
    add_compile_options(/utf-8)
    # 如果源文件有 BOM，需要保留；如果没有 BOM，MSVC 默认按系统代码页解释
    # 加上 /validate-charset 可以检测非法字符
    add_compile_options(/validate-charset-)
    # 设置源代码字符集和执行字符集都为 UTF-8
    add_compile_options(/source-charset:utf-8)
    add_compile_options(/execution-charset:utf-8)
else()
    # GCC/Clang：强制使用 UTF-8
    add_compile_options(-finput-charset=UTF-8)
    add_compile_options(-fexec-charset=UTF-8)
endif()

# 设置文件编码为 UTF-8（CMake 自身处理文件时使用）
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES)
set(CMAKE_CXX_EXTENSIONS OFF)



# 定义预处理器宏
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 打印配置信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version: ${Qt${QT_VERSION_MAJOR}_VERSION}")






