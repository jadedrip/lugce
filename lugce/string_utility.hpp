/* 
* Auctor: Chen Wang ( From China )
* Email: jadedrip@gmail.com
*
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
* 授权：LGNU
* 作 者：王琛 ( jadedrip@gmail.com )
* =============================================
* 文件名称：string_utility.hpp
* 摘 要：操作字符串的几个函数
* 当前版本：1.0
* 平台测试：Visual Studio 2008，gcc 4.3.3
****************************************************************************/
//	is_digits	判断字符串是否为全数字
//	slice_pair	将一个字符串切成2半

#ifndef __STRING_UTILITY_HPP__
#define __STRING_UTILITY_HPP__
#pragma once
#include <algorithm>
#include <string>
#include "string_of.hpp"

namespace lugce
{
	using namespace std;
		
	inline bool is_digits( const string &_Left )
	{
		for ( string::const_iterator p=_Left.begin(); p!=_Left.end(); p++ ){
			if ((*p < '0') || (*p >'9')) return false;
		}
		return true;
	}

	inline bool is_digits( const wstring &_Left )
	{
		for ( wstring::const_iterator p=_Left.begin(); p!=_Left.end(); p++ ){
			if ((*p < L'0') || (*p > L'9')) return false;
		}
		return true;
	}

	template< typename CHAR >
	inline std::pair< typename string_of<CHAR>::type, typename string_of<CHAR>::type >
		slice_pair( 
			const typename string_of<CHAR>::type& var, 
			const CHAR separator 
		)
	{
		typedef typename string_of<CHAR>::type string_type;
		string_type::size_type o=var.find(separator);
		if( o==string_type::npos )
			return std::make_pair(var, string_type());

		return std::make_pair( var.substr(0,o), var.substr(o+1) );
	}

	template< typename CHAR >
	inline std::pair< typename string_of<CHAR>::type, typename string_of<CHAR>::type >
		slice_pair( 
		const typename string_of<CHAR>::type& var, 
		const typename string_of<CHAR>::type separators 
		)
	{
		typedef basic_string<CHAR, char_traits<CHAR>, allocator<CHAR> > string_type;
		string_type::size_type o=var.find_first_of(separators);
		if( o==string_type::npos )
			return std::make_pair(var, string_type());

		return std::make_pair( var.substr(0,o), var.substr(o+1) );
	}
};
#endif