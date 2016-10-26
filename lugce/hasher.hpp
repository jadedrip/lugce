/* 
* Copyright (C) 2010  
* author:	Chen Wang ( China )
* Email:	jadedrip@gmail.com
* ============================================
*		* GNU Lesser General Public License *
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* =============================================
* Copyright (c) 2010，作者保留所有权利。( All rights reserved. )
* 文件名称：hasher.hpp
* 摘 要：通用哈希函数，注意，这里的哈希函数并非以加密为目的，主要是提供给哈希表使用。
* 
* 当前版本：1.0
* 作 者：王琛
* 完成日期：2009年6月22日
* 平台测试：Visual Studio 2010，gcc 4.3.3
* =============================================
*/

#pragma once
#include <functional>
#include "typedef.hpp"
namespace lugce
{
	namespace details
	{
		inline size_t swap_data( const size_t v )
		{
			return ( v << (sizeof(size_t)/2) ) | ( v >> (sizeof(size_t)/2) );
		}

		template< int rem >
		struct hasher_get
		{
			static size_t get( const void * data )
			{
				size_t v=0;
				memcpy( &v, data, rem );
				return v;
			}
		};

		template<>
		struct hasher_get<1>
		{
			static size_t get( const void * data )
			{
				const char *p=(const char*)data;
				return size_t(*p);
			}
		};

		template<>
		struct hasher_get<2>
		{
			static size_t get( const void * data )
			{
				const uint16 *p=(const uint16*)data;
				return size_t(*p);
			}
		};

		template<>
		struct hasher_get<4>
		{
			static size_t get( const void * data )
			{
				const uint32 *p=(const uint32 *)data;
				return size_t(*p);
			}
		};

		template<>
		struct hasher_get<8>
		{
			size_t get( const void * data )
			{
				const uint64 *p=(const uint64 *)data;
				return size_t(*p);
			}
		};

		template< int sz >
		struct hasher_data
		{
			static size_t hash( const void * data )
			{
				size_t *p=(size_t*)data;
				const int remd=( sz % sizeof(size_t) );
				const int cnt=(sz-remd) / sizeof(size_t);
				size_t v=2166136261U;
				if( 0==(cnt % 2) ){
					for( int i=0; i<cnt; i+=2 ){
						v|=swap_data(p[i]) | p[i+1];
					}
				}else{
					v|=p[0];
					for( int i=1; i<cnt; i+=2 ){
						v|=swap_data(p[i]) | p[i+1];
					}
				}

				p+=cnt;
				v|=hasher_get<remd>(p);
				return v;
			}
		};

		template<>
		struct hasher_data<sizeof(size_t)*2>
		{
			static size_t hash( const void * data )
			{
				size_t *p=(size_t*)data;
				return swap_data( p[0] ) | p[1];
			}
		};

		template<>
		struct hasher_data<sizeof(size_t)*3>
		{
			static size_t hash( const void * data )
			{
				size_t *p=(size_t*)data;
				return swap_data( p[0] ) | p[1] | p[2];
			}
		};
		template<>
		struct hasher_data<sizeof(size_t)*4>
		{
			static size_t hash( const void * data )
			{
				size_t *p=(size_t*)data;
				return swap_data( p[0] ) | p[1] | swap_data(p[2]) | p[3];
			}
		};
	}

	template< typename _Kty >
	struct hasher : public std::unary_function<_Kty, size_t>
	{
		size_t operator()( const _Kty& key ) const
		{
			size_t h=details::hasher_data<sizeof(_Kty)>::hash( &key );
			return h;
		}
	};

	template< typename _Kty >
	struct equrer : public std::binary_function<_Kty, _Kty, bool >
	{
		bool operator()( const _Kty& left, const _Kty& right ) const{ return 0==memcmp( &left, &right, sizeof(_Kty) ); }
	};
}
