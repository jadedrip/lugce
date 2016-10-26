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

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
#endif

/// 交换值, 并返回原始值
FORCEINLINE int32 STDCALL atomic_exchange( atomic_int32_t& atomic, const int32 new_value )
{
	__asm{
		mov		ecx, [atomic];
		mov		eax, new_value;
		xchg	[ecx], eax;
	};
}

/// 交换值, 并返回原始值
FORCEINLINE int64 STDCALL atomic_exchange( atomic_int64_t& atomic, const int64 new_value )
{
	__asm{
		lea	esi,new_value;
		mov	ebx,[esi];
		mov	ecx,4[esi];
		mov	esi,[atomic];
label3:
		mov	eax,[esi];
		mov	edx,4[esi];
		lock	cmpxchg8b [esi]; 
		jnz short label3; 
	};
}

/// 比较并设置新值，返回是否成功
FORCEINLINE bool STDCALL atomic_compare_and_set( atomic_int32_t& atomic, const int32 comp, const int32 xchgn )
{
	__asm{
		mov     ecx, [atomic];
		mov     edx, xchgn;
		mov     eax, comp;
		lock	cmpxchg [ecx], edx;		
		setz    al
	}
}

/// 比较并设置新值，返回是否成功
FORCEINLINE bool STDCALL atomic_compare_and_set( atomic_int64_t& atomic, const int64 comp, const int64 xchgn )
{
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
}

/// 自增，并返回原始值
FORCEINLINE int32 STDCALL atomic_exchange_and_increment( atomic_int32_t& atomic )
{
	__asm
	{
		mov     ecx, [atomic];
		mov     eax, 1;
		lock	xadd [ecx], eax; //加
	}
}

/// 自减，并返回原始值
FORCEINLINE int32 STDCALL atomic_exchange_and_decrement( atomic_int32_t& atomic )
{
	__asm
	{
		mov		ecx, [atomic];
		mov		eax, 0FFFFFFFFh;//-1
		lock	xadd [ecx], eax; //加-1
	}
}

/// 自增，并返回原始值
FORCEINLINE int64 STDCALL atomic_exchange_and_increment( atomic_int64_t& atomic )
{
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
}

/// 自减，并返回原始值
FORCEINLINE int64 STDCALL atomic_exchange_and_decrement( atomic_int64_t& a )
{
	__asm{
		mov		esi, [a]
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
}

/// 原子加法，返回原始值
FORCEINLINE int32 STDCALL atomic_exchange_and_add( atomic_int32_t& atomic, const int32 v )
{
	__asm
	{
		mov     esi, [atomic];
		mov     eax, v;
		lock	xadd [esi], eax; //加
		//add     eax, v;
	}
}

FORCEINLINE int64 STDCALL atomic_exchange_and_add( atomic_int64_t& atomic, const int64 v )
{
	__asm{
		mov esi, [atomic];
		mov	eax, [esi];
		mov	edx, 4[esi];
lable_increment:
		lea esi, v;
		mov ebx, [esi];
		mov ecx, 4[esi];
		add ebx,eax;  
		adc ecx,edx;  
		mov esi, [atomic];
		lock	cmpxchg8b [esi];
		jnz	short lable_increment;
	}
}

#ifdef __cplusplus
	};
};
#endif