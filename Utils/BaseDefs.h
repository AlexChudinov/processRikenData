#pragma once

#define DEF_NOT_COPYABLE(Obj)\
	Obj(const Obj&) = delete;\
	Obj(Obj&&) = delete;\
	Obj& operator=(const Obj&) = delete;\
	Obj& operator=(Obj&&) = delete;\

#define DEF_STR(X) #X