add_executable(rasmus-cmd main.cc)

target_link_libraries(rasmus-cmd frontend ${Readline_LIBRARY} -lncurses -pthread)

set_target_properties(rasmus-cmd PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)	
