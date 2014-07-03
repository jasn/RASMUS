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

	explicit RefPtr(T * p=0): ptr(nullptr) {reset(p);}

	RefPtr(const RefPtr<T> & p): ptr(nullptr) {reset(p.get());}

	RefPtr(RefPtr<T> && p): ptr(p.ptr) {p.ptr=nullptr;}
	~RefPtr() {reset(nullptr);}


	RefPtr & operator=(const RefPtr<T> & p)  {reset(p.get());	return *this; }
	
	RefPtr & operator=(RefPtr<T> && p)  {
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

	template<typename TT>
	const TT * getAs() const {return static_cast<const TT *>(ptr);}

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

	template <typename TT>
	RefPtr(const RefPtr<TT> & p): ptr(nullptr) {reset(p.get());}

	template <typename TT>
	RefPtr(RefPtr<TT> && p): ptr(p.ptr) {p.ptr=nullptr;}

	template <typename TT>
	RefPtr & operator=(const RefPtr<TT> & p)  {reset(p.get());	return *this; }

	template <typename TT>
	RefPtr & operator=(RefPtr<TT> && p)  {
		if (ptr) ptr->decref();
		ptr = p.ptr;
		p.ptr = nullptr;
		return *this;
	}
private:
	T * ptr;
};


} //namespace stdlib
} //namespace rasmus
#endif //__REFPTR_HH__
