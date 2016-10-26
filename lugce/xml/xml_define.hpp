#pragma once
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
* 平台测试：Visual Studio 2008，gcc 4.3.3
* =============================================
* 摘    要：XML 常数定义
* 当前版本：1.1
****************************************************************************/
#include "../string_of.hpp"

namespace lugce
{
	namespace xml
	{
		namespace details{
			template< typename _CHAR >
			struct xml_define;

			template<>
			struct xml_define<char>
			{
				//inline static const char *snbsp(){ return "&nbsp;"; }
				static const char *samp(){ return "&amp;"; }
				static const char *squot(){ return "&quot;"; }
				static const char *sgt(){ return "&gt;"; }
				static const char *slt(){ return "&lt;"; }
				static const char *sapos(){ return "&apos;"; }
				static const char *sxml(){ return "xml"; }
				static const char *sversion(){ return "version"; }
				static const char *sencoding(){ return "encoding"; }

				static bool is_name_first_char( const char c, std::locale& loc )
				{
					return std::isalpha(c, loc) || ( c=='_' );
				}
				static bool is_name_char( const char c, std::locale& loc )
				{
					return std::isalnum(c, loc) || ( c=='_' );
				}

				static std::string create_xml( const std::string& version, const std::string& encoding )
				{
					std::string s("<?xml version=\"");
					s.append( version );
					if( encoding.empty() ) 
						s.append("\" ?>");
					else
						s.append("\" encoding=\"").append( encoding ).append("\" ?>");
					return s;
				}
			};

			template<>
			struct xml_define<wchar_t>
			{
				static const wchar_t *samp(){ return L"&amp;"; }
				static const wchar_t *squot(){ return L"&quot;"; }
				static const wchar_t *sgt(){ return L"&gt;"; }
				static const wchar_t *slt(){ return L"&lt;"; }
				static const wchar_t *sapos(){ return L"&apos;"; }
				static const wchar_t *sxml(){ return L"xml"; }
				static const wchar_t *sversion(){ return L"version"; }
				static const wchar_t *sencoding(){ return L"encoding"; }
				static bool is_name_first_char( const wchar_t c, std::locale& loc )
				{
					if( c > 127 ) return true;
					return std::isalpha(c, loc) || ( c=='_' );
				}
				static bool is_name_char( const wchar_t c, std::locale& loc )
				{
					if( c > 127 ) return true;
					return std::isalnum(c, loc) || ( c=='_' );
				}

				static std::wstring create_xml( const std::wstring& version, const std::wstring& encoding )
				{
					std::wstring s(L"<?xml version=\"");
					s.append( version );
					if( encoding.empty() ) 
						s.append(L"\" ?>");
					else
						s.append(L"\" encoding=\"").append( encoding ).append(L"\" ?>");
					return s;
				}
			};

			// 转义函数，将 字符串中相关字符 使用实体转义代替
			template< typename _CHAR >
			const _CHAR* xml_escape( const _CHAR c )
			{
				typedef details::xml_define<_CHAR> XC;
				switch( c ){
				case '<':
					return XC::slt();
				case '>':
					return XC::sgt();
				case '&':
					return XC::samp();
				case '"':
					return XC::squot();
				case '\'':
					return XC::sapos();
				default:
					return NULL;
				}
			}

			template< typename _CHAR >
			typename string_of<_CHAR>::type xml_escape( const _CHAR* text )
			{
				assert( text!=NULL );
				register const _CHAR *p=text;
				typename string_of<_CHAR>::type _out;
				while( *p!=0 ){
					const _CHAR *i=xml_escape(*p);
					if ( i )
						_out+=i;
					else
						_out+=*p;
					++p;
				}
				return _out;
			}
		};
	};
};
