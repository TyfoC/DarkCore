#pragma once
#ifndef TYPEDEFS_HXX
#define TYPEDEFS_HXX

#define STRUCT_MEMBER_OFFSET(__stType, __name)				(size_t)(&((__stType*)0)->__name)
#define STRUCT_MEMBER_ADDRESS(__stObj, __stType, __name)	((size_t)&__stObj + STRUCT_MEMBER_OFFSET(__stType, __name))

//	only for power of 2
//	#define ALIGN_UP(__value, __alignment)						(((size_t)(__value) + (size_t)(__alignment) - 1) & ~((size_t)(__alignment) - 1))
//	#define ALIGN_DOWN(__value, __alignment)					((size_t)(__value) & ~(size_t)((__alignment) - 1))

#define DEFINE_SPECIAL(...)									__attribute__((__VA_ARGS__))
#define PACKED_DEFINITION									__packed__
#define NACKED_DEFINITION									__naked__
#define ALIGNED_DEFINITION(...)								__aligned__(__VA_ARGS__)

#define UNREFERENCED_PARAMETER(...)							(void)(__VA_ARGS__)
#define WRONG_INDEX											((size_t)-1)

#define EXTERN_C											extern "C"
#define EXTERN_CXX											extern

typedef char int8_t, *pint8_t;
typedef unsigned char uint8_t, *puint8_t;
typedef short int16_t, *pint16_t;
typedef unsigned short uint16_t, *puint16_t;
typedef int int32_t, *pint32_t;
typedef unsigned int uint32_t, *puint32_t;
typedef long long int64_t, *pint64_t;
typedef unsigned long long uint64_t, *puint64_t;

#ifndef _GCC_
typedef long int ptrdiff_t, *pptrdiff_t;
typedef __SIZE_TYPE__ size_t, *psize_t;
#else
typedef int ptrdiff_t, *pptrdiff_t;
typedef unsigned int size_t, *psize_t;
#endif

#ifdef __cplusplus
#define ATEXIT_MAX_FUNCS	128
EXTERN_C {
	struct atexit_func_entry_t {
		void	(*destructor_func)(void*);
		void*	obj_ptr;
		void*	dso_handle;
	};

	int __cxa_atexit(void (*f)(void*), void* objptr, void* dso);
	void __cxa_finalize(void* f);
};

namespace __cxxabiv1 {
	__extension__ typedef int __guard DEFINE_SPECIAL(mode(__DI__));
	EXTERN_C int __cxa_guard_acquire(__guard*);
	EXTERN_C void __cxa_guard_release(__guard*);
	EXTERN_C void __cxa_guard_abort(__guard*);
};
#endif

size_t AlignUp(size_t value, size_t alignment);
size_t AlignDown(size_t value, size_t alignment);

#endif