/**
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
* 摘    要：提供了一个的 Xml 文本书写器
* 当前版本：1.0
* 作    者：王琛
* 平台测试：Visual Studio 2008，gcc 4.3.3
*****************************************************************************/

#ifndef __XML_WRITER_HPP__
#define __XML_WRITER_HPP__

#pragma once
#include <assert.h>
#include <sstream>
#include <string>
#include <boost/type_traits/is_float.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "xml_define.hpp"
#include "../string_of.hpp"
#include "../stream_of.hpp"
#include "../stringstream_of.hpp"

namespace lugce
{
	namespace xml
	{
		template< typename _CHAR >
		class xml_writer
		{
		private:
			typedef typename string_of<_CHAR>::type xstring;
			typedef typename ostringstream_of<_CHAR>::type xostringstream;
			typedef details::xml_define<_CHAR> XC;
		public:
			xml_writer( const xstring& name=xstring() ) : _name(name)
			{
			}

			void set_version( const xstring& version, const xstring& encoding )
			{
				_processors=XC::create_xml( version, encoding );
			}

			// 添加一个属性
			xml_writer& operator ()(  const xstring &name, const xstring& value )
			{
				_attribs.append( 1, _CHAR(' ') );
				_attribs.append( name );
				_attribs.append( 1, _CHAR('=') );
				_attribs.append( 1, _CHAR('"') );
				_attribs.append( details::xml_escape( value.c_str() ) );
				_attribs.append( 1, _CHAR('"') );
				return *this;
			};

			xml_writer& operator % ( const std::string& name )
			{
				return *this;
			}

			// 添加一个属性
			template< typename T >
			xml_writer& operator ()(  const xstring &name, const T& value )
			{
				_attribs.append( 1, _CHAR(' ') );
				_attribs.append( name );
				_attribs.append( 1, _CHAR('=') );
				_attribs.append( 1, _CHAR('"') );
				_attribs.append( boost::lexical_cast<xstring>(value) );
				_attribs.append( 1, _CHAR('"') );
				return *this;
			}

			// 添加一个不需要转义的内容
			xml_writer& operator[]( const xstring& value )
			{	// 写入 Xml
				_content.append( value );
				return *this;
			}

			// 添加一个需要转义的内容
			xml_writer& operator()( const xstring& value )
			{	// 写入 Xml
				if( _name.empty() )
					_name=value;
				else
					_content.append( details::xml_escape( value.c_str() ) );
				return *this;
			}

			xml_writer& operator()( const xml_writer& p )
			{
				_content.append( p.str() );
				return *this;
			}

			friend std::basic_ostream<_CHAR, std::char_traits<_CHAR> >& operator <<
				( std::basic_ostream<_CHAR, std::char_traits<_CHAR> >& os, const xml_writer& p )
			{
				return os << p.str();
			}

			operator xstring()
			{
				return str();
			}

			xstring str() const
			{
				xstring x=_processors;
				x.append(1,_CHAR('<'));
				x.append(_name);
				if( !_attribs.empty() ) x.append(_attribs);
				if( _content.empty() ){
					x.append(1,_CHAR('/'));
					x.append(1,_CHAR('>'));
				}else{
					x.append(1,_CHAR('>'));
					x.append(_content);
					x.append(1,_CHAR('<'));
					x.append(1,_CHAR('/'));
					x.append(_name);
					x.append(1,_CHAR('>'));
				}
				return x;
			}
		private:
			xstring _processors;
			xstring _name;
			xstring _attribs;
			xstring _content;
		};

		typedef xml_writer<char> xwriter;
		typedef xml_writer<wchar_t> wxwriter;
	};
};

#endif // __XML_WRITER_HPP__

/* Test */
#ifdef _TEST
int _tmain(int argc, _TCHAR* argv[])
{
	using namespace std;
	char * const v="nice";

	wchar_t *val=L"<1>";
	const wchar_t* val2=L"<2>";
	std::wstring val3=L"<3>";
	const std::wstring val4(L"<4>" );
	wcout <<
		xml::write(/* 节点名称*/ L"node")
		[ /* 节点的属性 【可选】 */ xml::write( L"name", 0 )( L"val", val )( L"val2", val2)( L"val3", val3)( L"val4", val4)	](
		xml::write(L"nothing_node") + L"hello<>"
		/* 节点的内容 */// write( L"chld" ) <<
		);

	std::wstring tmp=xml::write(L"ok");

	getchar();

	return 0;
}

#endif
