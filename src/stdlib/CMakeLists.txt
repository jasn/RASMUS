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
	funcInvocation.cc
	funcInvocation.hh
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
   if (NOT APPLE)
       install(TARGETS stdlib DESTINATION lib)
   endif()
endif (RASMUS_STATIC)


