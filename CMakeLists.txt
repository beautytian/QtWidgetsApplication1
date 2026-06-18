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

# 手动指定 GDAL 路径
set(GDAL_INCLUDE_DIR "D:/gdal-2.3.1/include" CACHE PATH "" FORCE)
set(GDAL_LIBRARY "D:/gdal-2.3.1/lib/gdal_i.lib" CACHE FILEPATH "" FORCE)

# 查找 GDAL
find_package(GDAL REQUIRED)

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

# 如果是 Windows，可能需要链接额外的库
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_DL_LIBS}
    )
endif()

# 设置编译选项
if(MSVC)
    # Visual Studio 编译器
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /utf-8
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
    )
endif()

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






