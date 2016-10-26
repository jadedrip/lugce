/* 
* Copyright (C) 2010  Chen Wang ( China )
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
* Copyright (c) 2010，作者保留所有权利。( All rights reserved. )
* 文件名称：typedef.hpp
* 摘 要：定义一个版本号结构
* 
* 当前版本：1.0
* 作 者：王琛
* 平台测试：Visual Studio 2010，gcc 4.3.3
* =============================================
*/

#pragma once
#include <stdint.h>
#include <exception>
#include <string>
#include <boost/operators.hpp>

namespace lugce
{
	class version : public boost::totally_ordered<version>
	{
	public:
		version(){ value=0; }

		version( const char* str )
		{
			this->parse<char>(str);
		}

		version( const wchar_t* str )
		{
			this->parse<wchar_t>(str);
		}

		template< typename CHAR >
		version( const std::basic_string<CHAR, std::char_traits<CHAR>, std::allocator<CHAR> >& str )
		{
			this->parse(str);
		}

		version( uint16_t major_, uint16_t minor_=0, uint16_t revision_=0, uint16_t build_=0 )
			: major(major_), minor(minor_), revision(revision_), build(build_)
		{
		}

		bool operator < ( const version& v ) const
		{
			if( major < v.major ) return true;
			if( major > v.major ) return false;
			if( minor < v.minor ) return true;
			if( minor > v.minor ) return false;
			if( revision < v.revision ) return true;
			if( revision > v.revision ) return false;
			return build < v.build;
		}
	private:
		template< typename CHAR >
		void parse( const std::basic_string<CHAR, std::char_traits<CHAR>, std::allocator<CHAR> >& str )
		{
			CHAR schar=0;
			value=0;
			int x=0;
			for( size_t i=0; i<str.size(); ++i ){
				const CHAR c=str[i];
				if( isspace((int)c) ) break;	// 忽略空格
				char o=c2i( (char)c );
				if(o<0){
					if( c!='.' && ','!=c && '_'!=c ) throw std::bad_cast();	// 分隔符只允许是 . , _
					if(schar==0)	// 分隔符必须一致
						schar=c;
					else if(schar!=c)
						throw std::bad_cast();	

					if(++x>3)	throw std::bad_cast();
				}else{
					data[x]=(data[x] << 8) | o;
				}
			}
		}

		int8_t c2i( char c )
		{
			if(c >= '0' && c <= '9')
				return c-'0';
			return -1;
		}
	public:
		union 
		{
			struct 
			{
				uint16_t major;
				uint16_t minor;
				uint16_t revision;
				uint16_t build;
			};
			uint16_t data[4];
			uint64_t value;
		};
	};

	template< typename CHAR >
	inline std::basic_ostream<CHAR, std::char_traits<CHAR> >& operator << ( std::basic_ostream<CHAR, std::char_traits<CHAR> >& os, const version v )
	{
		return os << v.major << CHAR('.') << v.minor <<  CHAR('.') << v.revision <<  CHAR('.') << v.build;
	}
};
