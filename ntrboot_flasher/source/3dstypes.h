/*---------------------------------------------------------------------------------

	ndstypes.h -- Common types (and a few useful macros)

	Copyright (C) 2005 - 2008
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)
		Chris Double (doublec)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
/*! \file ndstypes.h
	\brief Custom types employed by libnds
*/

#ifndef _NDSTYPES_INCLUDE
#define _NDSTYPES_INCLUDE
//---------------------------------------------------------------------------------
// define libnds types in terms of stdint
#include <stdint.h>
#include <stdbool.h>

//---------------------------------------------------------------------------------
// libgba compatible section macros
//---------------------------------------------------------------------------------
#define ITCM_CODE	__attribute__((section(".itcm"), long_call))

#define DTCM_DATA	__attribute__((section(".dtcm")))
#define DTCM_BSS	__attribute__((section(".sbss")))

//! aligns a struct (and other types?) to m, making sure that the size of the struct is a multiple of m.
#define ALIGN(m)	__attribute__((aligned (m)))

//! packs a struct (and other types?) so it won't include padding bytes.
#define PACKED __attribute__ ((packed))
#define packed_struct struct PACKED

//---------------------------------------------------------------------------------
// These are linked to the bin2o macro in the Makefile
//---------------------------------------------------------------------------------
#define GETRAW(name)      (name)
#define GETRAWSIZE(name)  ((int)name##_size)
#define GETRAWEND(name)  ((int)name##_end)


/*!
	\brief returns a number with the nth bit set.
*/
#define BIT(n) (1 << (n))

#define u8 unsigned char
#define uint8 unsigned char
#define vu8 volatile unsigned char
#define vuint8 volatile unsigned char
#define vuint8_t volatile unsigned char

#define s8 signed char
#define sint8 signed char
#define vs8 volatile signed char
#define vsint8 volatile signed char

#define u16 unsigned short
#define uint16 unsigned short
#define vu16 volatile unsigned short
#define vuint16 volatile unsigned short

#define s16 signed short
#define sint16 signed short
#define vs16 volatile signed short
#define vsint16 volatile signed short

#define u32 unsigned int
#define uint32 unsigned int
#define vu32 volatile unsigned int
#define vuint32 volatile unsigned int

#define s32 signed int
#define sint32 signed int
#define vs32 volatile signed int
#define vsint32 volatile signed int

#define u64 unsigned long long int
#define uint64 unsigned long long int
#define vu64 volatile unsigned long long int
#define vuint64 volatile unsigned long long int

#define s64 signed long long int
#define sint64 signed long long int
#define vs64 volatile signed long long int
#define vsint64 volatile signed long long int

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

static inline uint32_t min(uint32_t a, uint32_t b)
{
	return (a<b)?a:b;
}

static inline uint32_t max(uint32_t a, uint32_t b)
{
	return (a>b)?a:b;
}

// Handy function pointer typedefs
//! a function pointer that takes no arguments and doesn't return anything.
typedef void (* VoidFn)(void);

typedef void (* IntFn)(void);
typedef void (* fp)(void);

//---------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------------
