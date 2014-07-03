// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The pyRASMUS development team
// 
// This file is part of pyRASMUS.
// 
// pyRASMUS is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with pyRASMUS.  If not, see <http://www.gnu.org/licenses/>
#ifndef __RM_OBJECT_HH__
#define __RM_OBJECT_HH__
#include "lib.h"
#include <cstdint>
#include <shared/type.hh>
#include "refptr.hh"

extern "C" {

struct rm_object {
	uint32_t ref_cnt;
	const LType type;
	rm_object(LType type, uint32_t ref_cnt=0): ref_cnt(ref_cnt), type(type) {}

	void incref()  {ref_cnt++;}
	
	void decref() {
		ref_cnt--;
		if (ref_cnt == 0) rm_free(this);
	}
};

} //extern "C"

namespace rasmus {
namespace stdlib {

extern size_t objectCount;
extern bool debugAllocations;

void reportAllocation(rm_object * ptr);
void reportDeallocation(rm_object * ptr);

inline void registerAllocation(rm_object * ptr) {
	++objectCount;
	if (debugAllocations) reportAllocation(ptr);
}

inline void registerDeallocation(rm_object * ptr) {
	--objectCount;
	if (debugAllocations) reportDeallocation(ptr);
}

template <typename T, typename ...TT>
T * makeObject(TT && ... tt) {
	T * ret = new T(std::forward(tt)...);
	registerAllocation(ret);
	return ret;
}

template <typename T, typename ...TT>
T * makeRefObject(TT && ... tt) {
	T * ret = makeObject<T>(std::forward(tt)...);
	ret->ref_cnt = 1;
	return ret;
}

template <typename T, typename ...TT>
RefPtr<T> makeRef(TT && ... tt) {
	return RefPtr<T>(makeObject<T>(std::forward(tt)...));
}


} //namespace stdlib
} //namespace rasmus

#endif //__RM_OBJECT_HH__
