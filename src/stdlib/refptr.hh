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
#ifndef __REFPTR_HH__
#define __REFPTR_HH__

namespace rasmus {
namespace stdlib {

template <typename T>
class RefPtr {
public:
	explicit RefPtr(T * p=0): ptr(0) {reset(p);}
	RefPtr(const RefPtr & p): ptr(0) {reset(p.get());}
	RefPtr(RefPtr && p): ptr(p.ptr) {p.ptr=nullptr;}
	~RefPtr() {reset(0);}
	RefPtr & operator=(const RefPtr & p)  {reset(p.get());	return *this; }
	RefPtr & operator=(RefPtr && p)  {
		if (ptr) ptr->decref();
		ptr = p.ptr;
		p.ptr = nullptr;
		return *this;
	}
	T & operator*() const {return *ptr;}
	T * operator->() const {return ptr;}
	T * get() const  {return ptr;}

	template<typename TT>
	TT * getAs(){return static_cast<TT *>(ptr);}

	bool operator <(const RefPtr & o) const {return ptr < o.ptr;}
	bool operator >(const RefPtr & o) const {return ptr > o.ptr;}
	bool operator <=(const RefPtr & o) const {return ptr <= o.ptr;}
	bool operator >=(const RefPtr & o) const {return ptr >= o.ptr;}
	bool operator ==(const RefPtr & o) const {return ptr == o.ptr;}
	bool operator !=(const RefPtr & o) const {return ptr == o.ptr;}
	bool operator!() const {return !ptr;}
	
	void reset(T * p=nullptr) {
		if (p) p->incref();
		if (ptr) ptr->decref();
		ptr = p;
	}

	T * unbox(){
		T * tmp = ptr;
		ptr = nullptr;
		return tmp;
	}
	
	static RefPtr steal(T * p) {
		RefPtr r(p);
		p->decref();
		return r;
	}
private:
	T * ptr;
};


} //namespace stdlib
} //namespace rasmus
#endif //__REFPTR_HH__
