if (QT_FOUND)
  include(${QT_USE_FILE})
else()
endif()

if (WIN32)
   set(QT_USE_QTMAIN TRUE)
endif(WIN32)

include_directories(${QT_INCLUDES} ${Qt5PrintSupport_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

if (QT_FOUND)
  QT4_WRAP_UI(UISrs main.ui settings.ui editor.ui about.ui relation.ui colorSelector.ui fontSelector.ui)
else()
  QT5_WRAP_UI(UISrs main.ui settings.ui editor.ui about.ui relation.ui colorSelector.ui fontSelector.ui)
endif()

add_executable(rasmus 
  WIN32
  MACOSX_BUNDLE
  ${UISrs} 
  console.cc console.hh 
  main.cc 
  interpreter.cc interpreter.hh
  relation_model.hh relation_model.cc
  settings.cc settings.hh
  editor.hh editor.cc
  highlighter.cc highlighter.hh
  table.cc table.hh
  codeTextEdit.cc codeTextEdit.hh
  help.cc help.hh
  fontSelector.cc fontSelector.hh
  colorSelector.cc colorSelector.hh
  )

target_link_libraries(rasmus frontend ${QT_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} -pthread stdlib)

set_target_properties(rasmus PROPERTIES AUTOMOC TRUE)
set_target_properties(rasmus PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)
install(TARGETS rasmus
  RUNTIME DESTINATION bin COMPONENT Runtime
  BUNDLE DESTINATION . COMPONENT Runtime
)
