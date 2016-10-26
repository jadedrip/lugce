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

// 整数原子操作的实现
#pragma once
#include <boost/static_assert.hpp>

#include "../config.hpp"
#include "../../lugce/typedef.hpp"

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
#endif

#ifdef _MSC_VER
#		pragma pack(push, 4)
#endif
		/// 允许进行原子计算的32位数
		typedef struct _atomic_int32_t
		{
			volatile int32 data; 
		}
#ifdef _MSC_VER
#		pragma pack(pop)
#elif defined( __GNUC__ )
		__attribute__ ((aligned (4)))
#else
#	error	Unknow compiler.
#endif
		atomic_int32_t;

#ifdef _MSC_VER
#	pragma pack(push, 8)
#endif
		/// 允许进行原子计算的64位数
		typedef struct _atomic_int64_t
		{
			union{
				volatile int64 data;
				struct{
					volatile int32 low;
					volatile int32 high;
				};
			};
		}
#ifdef _MSC_VER
#	pragma pack(pop)
#elif defined( __GNUC__ )
		__attribute__ ((aligned (8)))
#endif
		atomic_int64_t;

#ifdef __LUGCE_32
		BOOST_STATIC_ASSERT( sizeof(atomic_int32_t)==4 );
		BOOST_STATIC_ASSERT( sizeof(atomic_int64_t)==8 );
#endif

#ifdef __cplusplus
	};
};
#endif

#if defined(_MSC_VER)
#		include "atomic_type_vc.hpp"
#elif defined( __GNUC__ )
#	include "atomic_type_gcc.hpp"		
#endif

#ifdef __cplusplus
namespace lugce{
	namespace lockfree{
#endif
		FORCEINLINE int32 STDCALL atomic_exchange_and_subtract( atomic_int32_t& a, const int32 v )
		{
			return atomic_exchange_and_add( a, -v );
		}

		FORCEINLINE int64 STDCALL atomic_exchange_and_subtract( atomic_int64_t& a, const int64 v )
		{
			return atomic_exchange_and_add( a, -v );
		}
#ifdef __cplusplus
	};
};
#endif