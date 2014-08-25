set(LIB_FILES 	
	callback.cc
	callback.hh
	default_callback.hh
	error.cc
	function.hh
	lib.h
	print.cc
	refptr.hh
	relation.cc
	relation.hh
	rm_object.cc
	rm_object.hh
	text.cc
	text.hh
	abort.cc
	abort.hh
	gil.hh
	gil.cc
	global.cc)

if (RASMUS_STATIC)
   add_library(
	stdlib 
	STATIC
	${LIB_FILES})
else (RASMUS_STATIC)
   add_library(
	stdlib 
	SHARED
	${LIB_FILES})
   set_target_properties(stdlib PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../../)
   install(TARGETS stdlib DESTINATION lib)
endif (RASMUS_STATIC)


