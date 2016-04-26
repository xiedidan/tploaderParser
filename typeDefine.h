#ifndef typeDefine_h
#define typedefine_h

#ifdef x86_64
// 64 bits
#define uint unsigned long
#define uint64 uint
#define ulong uint
// 32 bits
#define uint32 unsigned int
// 16 bits
#define ushort unsigned short
#define uint16 ushort
#endif

#ifdef x86
#define uint unsigned long long
#define uint32 unsigned int
#define ushort unsigned short
#endif

// 64 bits
#define uint64 uint
#define ulong uint
// 16 bits
#define uint16 ushort

#endif
