cmake_minimum_required(VERSION 2.8.12)

project(cafTestApplication)

# Open GL
find_package(OpenGL)

# Qt

# Qt MOC
set(MOC_HEADER_FILES MainWindow.h WidgetLayoutTest.h CustomObjectEditor.h
                     MenuItemProducer.h)

# Resource file
set(QRC_FILES ${QRC_FILES} textedit.qrc)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets OpenGL)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
qt5_add_resources(QRC_FILES_CPP ${QRC_FILES})

option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

if(USE_COMMAND_FRAMEWORK)
  include_directories(${cafCommand_SOURCE_DIR})
  add_definitions(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)

set(PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    MainWindow.h
    WidgetLayoutTest.cpp
    WidgetLayoutTest.h
    ManyGroups.cpp
    ManyGroups.h
    CustomObjectEditor.cpp
    CustomObjectEditor.h
    MenuItemProducer.cpp
    MenuItemProducer.h
    TamComboBox.h
    TamComboBox.cpp)

# add the executable
add_executable(
  ${PROJECT_NAME}
  ${PROJECT_FILES}
  ${MOC_SOURCE_FILES}
  ${QRC_FILES_CPP}
  $<TARGET_OBJECTS:cafCommandFeatures> # Needed for cmake version < 3.12. Remove
                                       # when we can use target_link_libraries
                                       # with OBJECT libraries
)

set(TAP_LINK_LIBRARIES cafUserInterface)

if(USE_COMMAND_FRAMEWORK)
  set(TAP_LINK_LIBRARIES
      ${TAP_LINK_LIBRARIES} cafCommand
      # cafCommandFeatures # Not possible using cmake version < 3.12. Use when
      # we can use target_link_libraries with OBJECT libraries
  )
endif(USE_COMMAND_FRAMEWORK)

# According to ivarun this is needed on OpenSuse, and Fedora. See:
# https://github.com/OPM/ResInsight/pull/7
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND EXTERNAL_LINK_LIBRARIES rt pthread)
endif()

target_link_libraries(${PROJECT_NAME} ${TAP_LINK_LIBRARIES} ${QT_LIBRARIES}
                      ${EXTERNAL_LINK_LIBRARIES})

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
    TARGET cafTestApplication
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
    COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
    COMMAND
      Qt5::windeployqt --no-compiler-runtime --no-system-d3d-compiler
      --no-quick-import --no-translations --verbose 0 --dir
      "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
      "$<TARGET_FILE_DIR:cafTestApplication>/$<TARGET_FILE_NAME:cafTestApplication>"
  )

  # Qt DLLs
  message(STATUS "Creating post build step for copying Qt DLLs")

  # copy all files in the windeployqt directory to the installation directory
  add_custom_command(
    TARGET cafTestApplication
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
      $<TARGET_FILE_DIR:cafTestApplication>)

  # copy deployment directory during installation
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/" DESTINATION .)
endif()
