add_library(stdlib SHARED error.cc inner.hh lib.h print.cc text.cc relation.cc)
set_target_properties(stdlib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)
