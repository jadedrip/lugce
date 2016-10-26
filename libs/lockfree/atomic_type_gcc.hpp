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

// Lockfree 在 gcc 上的实现
// 注意：请勿直接包含本文件
#pragma once
#include "../config.hpp"

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
#endif
		/// 允许进行 cas 计算的32位数
		typedef struct atomic_int32
		{
			volatile int32 data;
#ifdef __cplusplus
			atomic_int32(){}
			atomic_int32( const int32 v ) : data(v){}
			operator const int32() const{ return data; }
#endif
		} __attribute__ ((aligned (4)));

		/// 允许进行 cas 计算的64位数
		union atomic_int64
		{
			volatile int64 data;
			struct{
				volatile int32 low;
				volatile int32 high;
			};
#ifdef __cplusplus
			atomic_int64(){}
			atomic_int64( const int64 v ) : data(v){}
			operator const int64() const{ return data; }
#endif
		} __attribute__ ((aligned (8)));

		/* ============== 定义 C 函数 ==================*/
		inline int32 atomic_swap( atomic_int32* atomic, const int32 new_value )
		{
			return __sync_lock_test_and_set( (int32*)atomic, new_value );
		}

		inline bool atomic_cas( atomic_int32* atomic, const int32 cmp, const int32 xchg )
		{
			return __sync_bool_compare_and_swap( (int32*)atomic, cmp, xchg );					
		}

		inline int64 atomic_swap( atomic_int64* atomic, const int64 new_value )
		{
			return __sync_lock_test_and_set( (int64*)atomic, new_value );
		}
		inline bool atomic_cas( atomic_int64* atomic, const int64 cmp, const int64 xchg )
		{
			return __sync_bool_compare_and_swap( atomic, cmp, xchg );					
		}

		inline int32 atomic_increment( atomic_int32* atomic )
		{
			return __sync_add_and_fetch( (int32*)atomic);
		}

		inline int32 atomic_decrement( atomic_int32* atomic )
		{
			return __sync_sub_and_fetch( (int32*)atomic );
		}

		inline int64 atomic_increment( atomic_int64* atomic )
		{
			return __sync_add_and_fetch((int64*)atomic);
		}

		inline int64 atomic_decrement( atomic_int64* atomic )
		{
			return __sync_sub_and_fetch((int64*)atomic);
		}
	};
};