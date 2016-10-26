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
*/
#pragma once
#include <memory>
#include <type_traits>
#include <utility>

namespace std
{
	template<class _Ty, class _Alloc >
	class vector;

	template<class _Ty, class _Alloc >
	class deque;

	template<class _Ty, class _Alloc >
	class list;

	template<class _Kty, class _Pr, class _Alloc >
	class set;

	template<class _Kty, class _Pr, class _Alloc >
	class multiset;

	template<class _Kty, class _Ty, class _Pr, class _Alloc >
	class map;

	template<class _Kty, class _Ty, class _Pr, class _Alloc >
	class multimap;
};

namespace lugce
{
	template< typename T >
	struct is_container : public std::false_type{};

	template<class _Ty, class _Alloc >
	struct is_container< std::vector<_Ty,_Alloc> > : public std::true_type{};

	template<class _Ty, class _Alloc >
	struct is_container< std::deque<_Ty,_Alloc> > : public std::true_type{};

	template<class _Kty, class _Pr, class _Alloc >
	struct is_container< std::set<_Kty, _Pr, _Alloc> > : public std::true_type{};

	template<class _Kty, class _Pr, class _Alloc >
	struct is_container< std::multiset<_Kty, _Pr, _Alloc> > : public std::true_type{};

	template<class _Kty, class _Ty, class _Pr, class _Alloc >
	struct is_container< std::map<_Kty, _Ty, _Pr, _Alloc> > : public std::true_type{};

	template<class _Kty, class _Ty, class _Pr, class _Alloc >
	struct is_container< std::multimap<_Kty, _Ty, _Pr, _Alloc> > : public std::true_type{};

	template< typename T >
	struct add_unsigned{ typedef T value; };

	template<>
	struct add_unsigned<int8_t> { typedef uint8_t value; };

	template<>
	struct add_unsigned<int16_t> { typedef uint16_t value; };

	template<>
	struct add_unsigned<int32_t> { typedef uint32_t value; };

	template<>
	struct add_unsigned<int64_t> { typedef uint64_t value; };

	template< typename T >
	struct is_pair : public std::false_type{};

	template< typename T1, typename T2 >
	struct is_pair< std::pair<T1,T2> > : public std::true_type{};
};
