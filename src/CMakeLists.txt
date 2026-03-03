#set(PROJECT_FULL_NAME src)

#设置工程包含当前目录，非必须
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# 包含当前目录 这两个应该是包含关系 可注释掉
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 只收集源文件，不收集生成的文件
file(GLOB CURRENT_HEADERS ./*.h ./*.hpp)

file(GLOB CURRENT_SRCS ./*.cpp)

file(GLOB CURRENT_RC ./*.rc ./*.rcc ./*.qrc)
file(GLOB CURRENT_UI ./*.ui)  # 只收集 .ui 文件，不收集生成的 ui_*.h
#qt5_wrap_ui(WRAP_FILES ${CURRENT_UI})
# 创建源文件组
source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("UI Files" FILES ${CURRENT_UI}) #${WRAP_FILES}#)


# 对生成的文件进行分组
source_group("Generated\\MOC" REGULAR_EXPRESSION "moc_.*\\.cpp$")
source_group("Generated\\UIC" REGULAR_EXPRESSION "autouic_.*")
source_group("Generated\\Resources" REGULAR_EXPRESSION ".*/qrc_[^/]*\\.cpp$")
source_group("Generated" REGULAR_EXPRESSION ".*autogen.*")

# 创建可执行文件 - 只包含源文件，不包含生成的文件
add_executable(RS485SerialTool WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_UI} ${CURRENT_RC} 
)

# 链接Qt库
target_link_libraries(RS485SerialTool
    Qt5::Core
    Qt5::Widgets
    Qt5::SerialPort
)

# 必须包含二进制目录
#target_include_directories(RS485SerialTool PRIVATE
#    ${CMAKE_CURRENT_BINARY_DIR}
#)

# 添加调试信息
message(STATUS "CURRENT_HEADERS: ${CURRENT_HEADERS}")
message(STATUS "CURRENT_SRCS: ${CURRENT_SRCS}")
message(STATUS "CURRENT_UI: ${CURRENT_UI}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
# 检查 Qt5 的 uic 工具
get_target_property(QT_UIC_EXECUTABLE Qt5::uic IMPORTED_LOCATION)
message(STATUS "uic executable: ${QT_UIC_EXECUTABLE}")

# 手动验证 uic 是否能处理 UI 文件
if(CURRENT_UI AND QT_UIC_EXECUTABLE)
    foreach(ui_file ${CURRENT_UI})
        # 获取文件名（不含扩展名）
        get_filename_component(ui_name ${ui_file} NAME_WE)
        # 预期的输出文件名
        set(expected_output "${CMAKE_CURRENT_BINARY_DIR}/ui_${ui_name}.h")
        message(STATUS "Expected output for ${ui_file}: ${expected_output}")
        
        # 检查文件是否存在
        if(EXISTS ${ui_file})
            message(STATUS "UI file exists: ${ui_file}")
        else()
            message(WARNING "UI file does not exist: ${ui_file}")
        endif()
    endforeach()
endif()

# 链接内部库
#target_link_libraries(RS485SerialTool
#    core_lib
#    gui_lib
#    protocol_lib
#)

# 设置可执行文件的属性
set_target_properties(RS485SerialTool PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
)

# 安装规则（可选）
install(TARGETS RS485SerialTool
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

# 部署时复制Qt运行时库
if(WIN32 AND NOT CMAKE_CROSSCOMPILING)
    # 使用 windeployqt 工具更可靠地部署 Qt 运行时
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt5_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
	# 区分 Debug 和 Release 构建
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(WINDEPLOYQT_ARGS "--debug" "--pdb")
        else()
            set(WINDEPLOYQT_ARGS "--release")
        endif()
        add_custom_command(TARGET RS485SerialTool POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" 
			"$<TARGET_FILE:RS485SerialTool>" 
			--no-compiler-runtime 
			--no-system-d3d-compiler 
			--no-angle 
			--no-opengl-sw
			--qmldir ${CMAKE_CURRENT_SOURCE_DIR}  # 如果有 QML 文件
            COMMENT "Deploying Qt runtime with windeployqt"
			VERBATIM
        )
		message(STATUS "找到 windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
    else()
        message(WARNING "windeployqt not found, Qt runtime deployment may not work correctly")
        # 备用方案
        add_custom_command(TARGET RS485SerialTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying Qt runtime dependencies..."
            COMMENT "Manual Qt runtime deployment not implemented"
        )
    endif()
endif()
# 添加自定义目标用于手动部署
add_custom_target(deploy
    COMMAND ${CMAKE_COMMAND} -E echo "正在部署应用程序..."
    COMMAND "${WINDEPLOYQT_EXECUTABLE}" 
        "$<TARGET_FILE:RS485SerialTool>"
        --release
        --dir ${CMAKE_CURRENT_BINARY_DIR}/deploy
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "手动部署应用程序到 deploy 文件夹"
)
# 隐藏生成的文件（可选，但可能需要手动设置VS过滤器）
set_property(TARGET RS485SerialTool PROPERTY AUTOMOC_MACRO_NAMES "Q_OBJECT;Q_GADGET;Q_NAMESPACE")
set_property(TARGET RS485SerialTool PROPERTY AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})
# 设置生成文件的属性为不显示在解决方案资源管理器中
set_source_files_properties(${AUTOGEN_SOURCES} PROPERTIES 
    HEADER_FILE_ONLY TRUE
    VS_SHADER_TYPE "None"
)