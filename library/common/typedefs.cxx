#include "typedefs.hxx"

#ifdef __cplusplus
EXTERN_C {
	atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
	uint32_t __atexit_func_count = 0;
	__volatile__ void* __dso_handle = 0;

	int __cxa_atexit(void (*f)(void*), void* objptr, void* dso) {
		if (__atexit_func_count >= ATEXIT_MAX_FUNCS) return -1;
		__atexit_funcs[__atexit_func_count].destructor_func = f;
		__atexit_funcs[__atexit_func_count].obj_ptr = objptr;
		__atexit_funcs[__atexit_func_count].dso_handle = dso;
		++__atexit_func_count;
		return 0;
	}

	void __cxa_finalize(void* f) {
		uint32_t i = __atexit_func_count;
		if (!f) {
			while (i--) {
				if (__atexit_funcs[i].destructor_func) {
					(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
				}
			}
			return;
		}

		while (i--) {
			if (__atexit_funcs[i].destructor_func == f) {
				(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
				__atexit_funcs[i].destructor_func = 0;
			}
		}
	}	
};

EXTERN_C int __cxxabiv1::__cxa_guard_acquire(__guard* g) {
	return !*(pint8_t)(g);
}

EXTERN_C void __cxxabiv1::__cxa_guard_release(__guard* g) {
	*(pint8_t)g = 1;
}

EXTERN_C void __cxxabiv1::__cxa_guard_abort(__guard*) {
	//	this is stub
}
#endif

size_t AlignUp(size_t value, size_t alignment) {
	return value - (value % alignment) + alignment;
}

size_t AlignDown(size_t value, size_t alignment) {
	size_t unalignedPart = value % alignment;
	value -= unalignedPart;
	
	if (value < alignment) value = 0;
	else value -= alignment;
	
	return value;
}