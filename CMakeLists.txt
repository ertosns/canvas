cmake_minimum_required (VERSION 2.8.11)

set (PROJECT canvas)

message(STATUS "System: " ${CMAKE_HOST_SYSTEM_NAME} " " ${CMAKE_HOST_SYSTEM_VERSION})
message(STATUS "Processor: " ${CMAKE_HOST_SYSTEM_PROCESSOR})


set(CANVAS_VERSION 0.1.1)


set (HEADERS
  commands.h
  dialog_windows.h
  canvas.h
  main_window.h
  toolbar.h
  tool.h
  )

set (SOURCES
  commands.cpp
  dialog_windows.cpp
  canvas.cpp
  main.cpp
  main_window.cpp
  toolbar.cpp
  tool.cpp
  )

set (RESOURCE_PATH
  .)

set (RESOURCES
  ${RESOURCE_PATH}/resources.qrc)


project (${PROJECT})

include_directories (./)

find_package (Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(VTK COMPONENTS
  vtkCommonColor
  vtkCommonCore
  vtkFiltersSources
  vtkGUISupportQt
  vtkInteractionStyle
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingGL2PSOpenGL2
  vtkRenderingOpenGL2
  QUIET
  )
if (NOT VTK_FOUND)
  message("Skipping RenderWindowNoUiFile: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")


include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

qt5_add_resources (QRC_SOURCES ${RESOURCES})
qt5_wrap_cpp (MOC_SOURCES ${HEADERS})

add_definitions (-Wall)


source_group ("Header Files" FILES ${HEADERS})
source_group ("Source Files" FILES ${SOURCES})
source_group ("Generated Files" FILES ${MOC_SOURCES})
source_group ("Resource Files" FILES ${QRC_SOURCES})


if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  add_executable (${PROJECT} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${TRANSLATIONS_QM})


  target_link_libraries (${PROJECT} Qt5::Widgets Qt5::PrintSupport ${VTK_LIBRARIES})
else()
  add_executable (${PROJECT} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${TRANSLATIONS_QM})


  target_link_libraries (${PROJECT} Qt5::Widgets Qt5::PrintSupport ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS ${PROJECT}
    MODULES ${VTK_LIBRARIES}
    )
endif()


if(UNIX AND NOT APPLE)

    INSTALL(TARGETS canvas RUNTIME DESTINATION bin)

    INSTALL(FILES "sources/media/canvas.desktop" DESTINATION share/applications)

    INSTALL(FILES "sources/media/logo/canvas_64.png" DESTINATION share/pixmaps)

    INSTALL(FILES ${TRANSLATIONS_QM} DESTINATION share/canvas/translations)
endif(UNIX AND NOT APPLE)
