cmake_minimum_required (VERSION 3.5)
set(REQUIRED_CPP_VERSION 17)
set(CMAKE_CXX_STANDARD ${REQUIRED_CPP_VERSION})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(NOT TARGET_NAME)
  set(TARGET_NAME LogViewer)
endif()
project(${TARGET_NAME} VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(include_dirs  
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/CustomWidgets")

# gather the files
set(headers
  "${PROJECT_SOURCE_DIR}/src/ComUtils.h"
  "${PROJECT_SOURCE_DIR}/src/DefaultValues.h"
  "${PROJECT_SOURCE_DIR}/src/HypCommands.h"
  "${PROJECT_SOURCE_DIR}/src/HypReader.h"
  "${PROJECT_SOURCE_DIR}/src/LogWindow.h"
  "${PROJECT_SOURCE_DIR}/src/MainWnd.h"
  "${PROJECT_SOURCE_DIR}/src/Recording.h"
  "${PROJECT_SOURCE_DIR}/src/RecordingDataModel.h"
  "${PROJECT_SOURCE_DIR}/src/RecordingDisplayOptionsDlg.h"
  "${PROJECT_SOURCE_DIR}/src/RecordingDisplayWnd.h"
  "${PROJECT_SOURCE_DIR}/src/RecordingPlotter.h"
  "${PROJECT_SOURCE_DIR}/src/RecordingTableView.h"
  "${PROJECT_SOURCE_DIR}/src/SamplePoint.h"
  "${PROJECT_SOURCE_DIR}/src/TypeWrapper.h"
  "${PROJECT_SOURCE_DIR}/src/CustomWidgets/ColorButton.h"
  "${PROJECT_SOURCE_DIR}/src/CustomWidgets/YesNoDlg.h"
  "${PROJECT_SOURCE_DIR}/resources.qrc"
)
set(sources 
  "${PROJECT_SOURCE_DIR}/src/ComUtils.cpp"
  "${PROJECT_SOURCE_DIR}/src/DefaultValues.cpp"
  "${PROJECT_SOURCE_DIR}/src/HypReader.cpp"
  "${PROJECT_SOURCE_DIR}/src/LogWindow.cpp"
  "${PROJECT_SOURCE_DIR}/src/main.cpp"
  "${PROJECT_SOURCE_DIR}/src/MainWnd.cpp"
  "${PROJECT_SOURCE_DIR}/src/Recording.cpp"
  "${PROJECT_SOURCE_DIR}/src/RecordingDataModel.cpp"
  "${PROJECT_SOURCE_DIR}/src/RecordingDisplayOptionsDlg.cpp"
  "${PROJECT_SOURCE_DIR}/src/RecordingDisplayWnd.cpp"
  "${PROJECT_SOURCE_DIR}/src/RecordingPlotter.cpp"
  "${PROJECT_SOURCE_DIR}/src/RecordingTableView.cpp"
  "${PROJECT_SOURCE_DIR}/src/SamplePoint.cpp"
  "${PROJECT_SOURCE_DIR}/src/CustomWidgets/ColorButton.cpp"
  "${PROJECT_SOURCE_DIR}/src/CustomWidgets/YesNoDlg.cpp"
)

find_package(Qt5 COMPONENTS Core Widgets SerialPort REQUIRED)
if (NOT Qt5_FOUND)
  message(FATAL_ERROR "Qt5 not found.")
endif()
message(STATUS "-> Qt: ${Qt5_DIR}")

add_executable(${TARGET_NAME} ${sources} ${headers})
target_include_directories(${TARGET_NAME} 
  PRIVATE 
    "${include_dirs}"
    "${PROJECT_SOURCE_DIR}/3rdparty"
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD ${REQUIRED_CPP_VERSION})
if (WIN32 OR WIN64)
  add_definitions(-D_WIN32_WINNT=0x0601) # Windows 7
  set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
  target_compile_options(${TARGET_NAME} PRIVATE /W4 "/permissive-")
  target_link_libraries(${TARGET_NAME}
    Qt5::Widgets Qt5::SerialPort
  )

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            \"$<TARGET_FILE:${TARGET_NAME}>\"
    COMMENT "Deploying Qt...")
  
elseif(UNIX)
  # if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  #   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
  #   set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -fsanitize=thread")
  # endif()      
  target_link_libraries(${TARGET_NAME} stdc++fs)
endif()
