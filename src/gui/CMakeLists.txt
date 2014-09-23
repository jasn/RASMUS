include(${QT_USE_FILE})

if (WIN32)

   set(QT_USE_QTMAIN TRUE)
endif(WIN32)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

QT4_WRAP_UI(UISrs main.ui settings.ui editor.ui about.ui relation.ui colorSelector.ui fontSelector.ui)
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

target_link_libraries(rasmus frontend ${QT_LIBRARIES} -pthread stdlib)
set_target_properties(rasmus PROPERTIES AUTOMOC TRUE)
set_target_properties(rasmus PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)
install(TARGETS rasmus
  RUNTIME DESTINATION bin COMPONENT Runtime
  BUNDLE DESTINATION . COMPONENT Runtime
)
