cmake_minimum_required(VERSION 2.8)

project(cafTestCvfApplication)

# Qt MOC
set(MOC_HEADER_FILES MainWindow.h WidgetLayoutTest.h)

# Resource file
set(QRC_FILES textedit.qrc)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets OpenGL)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
qt5_add_resources(QRC_FILES_CPP ${QRC_FILES})

option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

include_directories(
  ${LibCore_SOURCE_DIR}
  ${LibGeometry_SOURCE_DIR}
  ${LibGuiQt_SOURCE_DIR}
  ${LibRender_SOURCE_DIR}
  ${LibViewing_SOURCE_DIR}
  ${cafProjectDataModel_SOURCE_DIR}
  ${cafUserInterface_SOURCE_DIR})

if(USE_COMMAND_FRAMEWORK)
  include_directories(${cafCommand_SOURCE_DIR})
  add_definitions(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)

include_directories(${cafPdmCore_SOURCE_DIR} ${cafPdmUiCore_SOURCE_DIR}
                    ${cafPdmXml_SOURCE_DIR} ${cafPdmCvf_SOURCE_DIR})

set(PROJECT_FILES Main.cpp MainWindow.cpp WidgetLayoutTest.cpp
                  TapCvfSpecialization.cpp TapProject.cpp)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES} ${MOC_SOURCE_FILES}
                               ${QRC_FILES_CPP})

set(TAP_LINK_LIBRARIES cafUserInterface cafPdmXml cafPdmCvf ${QT_LIBRARIES})

if(USE_COMMAND_FRAMEWORK)
  set(TAP_LINK_LIBRARIES ${TAP_LINK_LIBRARIES} cafCommand)
endif(USE_COMMAND_FRAMEWORK)

target_link_libraries(${PROJECT_NAME} ${TAP_LINK_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})

foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach(qtlib)

if(WIN32
   AND TARGET Qt5::qmake
   AND NOT TARGET Qt5::windeployqt)
  get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

  execute_process(
    COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE qt5_install_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

  message(STATUS "QMAKE: ${_qt5_qmake_location}")
  message(STATUS "QTPREFIX: ${qt5_install_prefix}")

  if(EXISTS ${imported_location})
    add_executable(Qt5::windeployqt IMPORTED)

    set_target_properties(Qt5::windeployqt PROPERTIES IMPORTED_LOCATION
                                                      ${imported_location})
  endif()
endif()

# TODO(wjwwood): find a way to make this optional or to run without "deploying"
# the necessary dlls and stuff to the bin folder. see:
# https://stackoverflow.com/questions/41193584/deploy-all-qt-dependencies-when-building#41199492
if(TARGET Qt5::windeployqt)
  # execute windeployqt in a tmp directory after build
  add_custom_command(
    TARGET cafTestCvfApplication
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
    COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
    COMMAND
      Qt5::windeployqt --no-compiler-runtime --no-system-d3d-compiler
      --no-quick-import --no-translations --verbose 0 --dir
      "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
      "$<TARGET_FILE_DIR:cafTestCvfApplication>/$<TARGET_FILE_NAME:cafTestCvfApplication>"
  )

  # Qt DLLs
  message(STATUS "Creating post build step for copying Qt DLLs")

  # copy all files in the windeployqt directory to the installation directory
  add_custom_command(
    TARGET cafTestCvfApplication
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
      $<TARGET_FILE_DIR:cafTestCvfApplication>)

  # copy deployment directory during installation
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/" DESTINATION .)
endif()
