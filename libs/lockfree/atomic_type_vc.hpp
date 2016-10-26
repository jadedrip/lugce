/*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*****************************************************************************
*	A beg: Please tell me if you are using this library.
*	Author: Chen Wang ( China )
*	Email: jadedrip@gmail.com
*/

// 支持 VC 编译的原子操作的实现
// 注意：请勿直接包含本文件
#pragma once
#include <Windows.h>

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
#endif

/// 交换值, 并返回原始值
FORCEINLINE int32 atomic_exchange( atomic_int32_t& a, int32 new_value )
{
	return InterlockedExchange( &a.data, new_value );
}

/// 交换值, 并返回原始值
FORCEINLINE int64 atomic_exchange( atomic_int64_t& atomic, int64 new_value )
{
#if _WIN32_WINNT >= 0x0601	
	return InterlockedExchange64( &atomic.data, new_value );
#elif defined( __LUGCE_64 )
#	error _WIN32_WINNT is too low.
#else
	__asm{
		lea	esi,new_value;
		mov	ebx,[esi];
		mov	ecx,4[esi];
		mov	esi,[atomic];
	label_atomic_exchange_64:
		mov	eax,[esi];
		mov	edx,4[esi];
		lock	cmpxchg8b [esi]; 
		jnz short label_atomic_exchange_64; 
	};
#endif
}

/// 比较并设置新值，返回是否成功
FORCEINLINE bool atomic_compare_and_set( atomic_int32_t& atomic, int32 comp, int32 xchgn )
{
	return ( comp==InterlockedCompareExchange( &atomic.data, xchgn, comp ) );
}

/// 比较并设置新值，返回是否成功
FORCEINLINE bool atomic_compare_and_set( atomic_int64_t& atomic, int64 comp, int64 xchgn )
{
#if _WIN32_WINNT >= 0x0601	
	return ( comp==InterlockedCompareExchange64( &atomic.data, xchgn, comp ) );
#elif defined( __LUGCE_64 )
#	error _WIN32_WINNT is too low.
#else
	__asm{
		lea	esi,comp;
		mov	eax,[esi];
		mov	edx,4[esi];
		lea	esi,xchgn;
		mov	ebx,[esi];
		mov	ecx,4[esi];
		mov	esi,[atomic];
		lock	cmpxchg8b [esi];
		setz	al;
	}
#endif
}

/// 自增，并返回原始值
FORCEINLINE int32 atomic_exchange_and_increment( atomic_int32_t& atomic )
{
	return InterlockedIncrement( &atomic.data );
}

/// 自减，并返回原始值
FORCEINLINE int32 atomic_exchange_and_decrement( atomic_int32_t& atomic )
{
	return InterlockedDecrement( &atomic.data );
}

/// 自增，并返回原始值
FORCEINLINE int64 atomic_exchange_and_increment( atomic_int64_t& atomic )
{
#if _WIN32_WINNT >= 0x0601	
	return InterlockedIncrement64( &atomic.data );
#elif defined( __LUGCE_64 )
#	error _WIN32_WINNT is too low.
#else
	__asm{
		mov esi, [atomic]
		mov	eax, [esi];
		mov	edx, 4[esi];
	lable_increment:
		mov ebx, eax;
		mov ecx, edx;
		add ebx,1;  
		adc ecx,0;  
		lock	cmpxchg8b [esi];
		jnz	short lable_increment;
	}
#endif
}

/// 自减，并返回原始值
FORCEINLINE int64 atomic_exchange_and_decrement( atomic_int64_t& atomic )
{
#if _WIN32_WINNT >= 0x0601	
	return InterlockedDecrement64( &atomic.data );
#elif defined( __LUGCE_64 )
#	error _WIN32_WINNT is too low.
#else
	__asm{
		mov		esi, [atomic]
		mov		eax, [esi];
		mov		edx, 4[esi];
	lable_atomic_decrement_64:
		mov		ebx, eax;
		mov		ecx, edx;

		add		ebx,0FFFFFFFFh;
		adc		ecx,0FFFFFFFFh;  
		lock	cmpxchg8b [esi];
		jnz	short lable_atomic_decrement_64;
	}			
#endif
}

/// 原子加法，返回原始值
FORCEINLINE int32 atomic_exchange_and_add( atomic_int32_t& atomic, int32 v )
{
	return InterlockedExchangeAdd( &atomic.data, v );
}

FORCEINLINE int64 atomic_exchange_and_add( atomic_int64_t& atomic, int64 v )
{
#if _WIN32_WINNT >= 0x0601	
	return InterlockedExchangeAdd64( &atomic.data, v );
#else
	int64 old;
	do{
		old=atomic.data;
	}while( !atomic_compare_and_set( atomic, old, old+v ) );
	return old;
#endif
}

#ifdef __cplusplus
	};
};
#endif