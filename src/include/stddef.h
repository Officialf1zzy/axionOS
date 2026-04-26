#ifndef _STDDEF_H
#define _STDDEF_H

// Null pointer
#define NULL ((void*)0)

// Size type (result of sizeof)
typedef __SIZE_TYPE__ size_t;

// Pointer difference type
typedef __PTRDIFF_TYPE__ ptrdiff_t;

// Offset of a member in a struct
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#endif // _STDDEF_H
