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
#include <cmath>
#include <string>
#include <stdlib/rm_object.hh>
#include <stdlib/ile.hh>
#include <stdlib/lib.h>

extern "C" {
rm_object *rm_floorFunc, *rm_ceilFunc, *rm_roundFunc, *rm_expFunc, *rm_logFunc, *rm_log2Func, *rm_log10Func;
rm_object *rm_cosFunc, *rm_sinFunc, *rm_tanFunc, *rm_acosFunc, *rm_asinFunc, *rm_atanFunc, *rm_atan2Func, *rm_powFunc, *rm_sqrtFunc, *rm_absFunc;
rm_object *rm_substrFunc;
}

namespace rasmus {
namespace stdlib {

namespace {

union Alias {
	int64_t i;
	double d;
	void * ptr;
};

template <typename T>
struct FromAny {};

template <>
struct FromAny<double> {
	double operator()(int64_t val, int8_t type) {
		switch(type) {
		case TInt:
			return (double)val;
		case TFloat: {
			Alias a;
			a.i=val;
			return a.d;
		}
		default:
			ILE("Wrong argument type");
			break;
		}
	}
};

template <>
struct FromAny<int64_t> {
	double operator()(int64_t val, int8_t type) {
		switch(type) {
		case TInt:
			return val;
		default:
			ILE("Wrong argument type");
			break;
		}
	}
};


template <>
struct FromAny<rm_object *> {
	rm_object * operator()(int64_t val, int8_t type) {
		switch (type) {
		case TText:
		case TRel:
		case TTup:
			return reinterpret_cast<rm_object*>(val);
		default:
			ILE("Wrong argument type");
			break;
		}
	}
};


template <typename T>
struct ToAny {};

template <>
struct ToAny<double> {
	void operator()(AnyRet * ret, double v) {
		if (!std::isfinite(v))
			v=std::numeric_limits<double>::quiet_NaN();
		Alias a;
		a.d=v;

		ret->value = a.i;
		ret->type = TFloat;
	}
};

template <>
struct ToAny<int64_t> {
	void operator()(AnyRet * ret, int64_t v) {
		ret->value = v;
		ret->type = TInt;
	}
};

template <>
struct ToAny<rm_object*> {
	void operator()(AnyRet * ret, rm_object * v) {
		ret->value = reinterpret_cast<uint64_t>(v);
		switch(v->type) {
		case LType::smallText:
		case LType::concatText:
		case LType::substrText:
		case LType::canonicalText:
		case LType::undefText:
			ret->type = TText;
			break;
		case LType::function:
			ret->type = TFunc;
			break;
		case LType::tuple:
			ret->type = TTup;
			break;
		case LType::relation:
			ret->type = TRel;
			break;
		case LType::schema:
			ILE("Schema not supported");
			break;
		}
	}
};


void buildinDtor() {
	ILE("Buildin DTOR called");
}

struct buildin_func_object: public rm_object {
	int16_t argc;
	void * dtor;
	void * func;
	buildin_func_object(size_t argc, void * func)
		: rm_object(LType::function, 1)
		, argc(argc)
		, dtor((void*)buildinDtor)
		, func(func) {}
};

template <typename ...TS>
struct Cnt {
	static const size_t v=0;
};

template <typename T1, typename ...TS>
struct Cnt<T1, TS...> {
	static const size_t v=Cnt<TS...>::v+1;
};

template <typename F, typename RetType, typename ...Args>
struct RasmusFuncWrap {
	static void call(F func);
};


template <typename F, typename RetType, typename T1> 
struct RasmusFuncWrap<F, RetType, T1> {
	static void call(rm_object *, 
					 AnyRet * ret, 
					 int64_t v1,
					 int8_t t1) {
		RetType t=F::call(FromAny<T1>()(v1, t1));
		ToAny<RetType>()(ret, t);
	}
};

template <typename F, typename RetType, typename T1, typename T2> 
struct RasmusFuncWrap<F, RetType, T1, T2> {
	static void call(rm_object *, 
					 AnyRet * ret, 
					 int64_t v1,
					 int8_t t1,
					 int64_t v2,
					 int8_t t2) {
		RetType t=F::call(FromAny<T1>()(v1, t1), FromAny<T2>()(v2, t2));
		ToAny<RetType>()(ret, t);
	}
};

template <typename F, typename RetType, typename T1, typename T2, typename T3> 
struct RasmusFuncWrap<F, RetType, T1, T2,  T3> {
	static void call(rm_object *, 
					 AnyRet * ret, 
					 int64_t v1,
					 int8_t t1,
					 int64_t v2,
					 int8_t t2,
					 int64_t v3,
					 int8_t t3) {
		RetType t=F::call(FromAny<T1>()(v1, t1), FromAny<T2>()(v2, t2) , FromAny<T2>()(v3, t3));
		ToAny<RetType>()(ret, t);
	}
};


// C++ magic gadget to capture global functions as template argumens
// Perhaps there is a prettier way of doing this?
template <typename RetType, typename ...Args>
struct FuncCapture {
	template< RetType(* X)(Args...) >
	struct Func {
		static inline RetType call(Args ... args) {
			return X(args...);
		}
	};
};

	   
template <typename F, typename RetType, typename ...Args>
rm_object * createBuildIn() {
	return new buildin_func_object(
		Cnt<Args...>::v,
		(void*)RasmusFuncWrap<F, RetType, Args...>::call);
}


struct Initer {
	Initer() {
		rm_floorFunc = createBuildIn<FuncCapture<double, double>::Func<std::floor>, int64_t, double >();
		rm_ceilFunc = createBuildIn<FuncCapture<double, double>::Func<std::ceil>, int64_t, double >();
		rm_roundFunc = createBuildIn<FuncCapture<double, double>::Func<std::round>, int64_t, double >();
		rm_sinFunc = createBuildIn<FuncCapture<double, double>::Func<std::sin>, double, double >();
		rm_cosFunc = createBuildIn<FuncCapture<double, double>::Func<std::cos>, double, double >();
		rm_tanFunc = createBuildIn<FuncCapture<double, double>::Func<std::tan>, double, double >();
		rm_asinFunc = createBuildIn<FuncCapture<double, double>::Func<std::asin>, double, double >();
		rm_acosFunc = createBuildIn<FuncCapture<double, double>::Func<std::acos>, double, double >();
		rm_atanFunc = createBuildIn<FuncCapture<double, double>::Func<std::atan>, double, double >();
		rm_atan2Func = createBuildIn<FuncCapture<double, double, double>::Func<std::atan2>, double, double, double >();
		rm_powFunc = createBuildIn<FuncCapture<double, double, double>::Func<std::pow>, double, double, double >();
		rm_sqrtFunc = createBuildIn<FuncCapture<double, double>::Func<std::sqrt>, double, double >();
		rm_absFunc = createBuildIn<FuncCapture<double, double>::Func<std::abs>, double, double >();
		rm_expFunc = createBuildIn<FuncCapture<double, double>::Func<std::exp>, double, double >();
		rm_logFunc = createBuildIn<FuncCapture<double, double>::Func<std::log>, double, double >();
		rm_log10Func = createBuildIn<FuncCapture<double, double>::Func<std::log10>, double, double >();
		rm_log2Func = createBuildIn<FuncCapture<double, double>::Func<std::log2>, double, double >();
		rm_substrFunc = createBuildIn<FuncCapture<rm_object *, rm_object *, int64_t, int64_t>::Func<rm_substrText>, 									  rm_object*, rm_object*, int64_t, int64_t>();
	}
};


Initer _;

}
}
}

