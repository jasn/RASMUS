include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
QT4_WRAP_UI(UISrs main.ui)
add_executable(gui ${UISrs} console.hh main.cc interpreter.cc interpreter.hh console.cc)

target_link_libraries(gui frontend ${QT_LIBRARIES})

set_target_properties(gui PROPERTIES AUTOMOC TRUE)
