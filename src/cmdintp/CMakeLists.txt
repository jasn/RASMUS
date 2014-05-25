add_executable(rm main.cc)

target_link_libraries(rm frontend ${Readline_LIBRARY} -lncurses -pthread)

set_target_properties(rm PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)