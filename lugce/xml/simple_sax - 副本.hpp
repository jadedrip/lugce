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
* License along with this library.
* =============================================
* 授权：LGNU
* 作 者：王琛 ( jadedrip@gmail.com )
* =============================================
* 当前版本：1.0
* 平台测试：Visual Studio 2008，gcc 4.3.3
* =============================================
* 摘    要：一个简化版本的 xml 解析器
* 
* 当前版本：1.1
* 作    者：王琛
****************************************************************************/
#ifndef __SIMPLESAX_HPP_25EAE119_C8C9_44CB_883A_049499D9E137__
#define __SIMPLESAX_HPP_25EAE119_C8C9_44CB_883A_049499D9E137__
#if _MSC_VER > 1000
#pragma once
#endif
#include <string>
#include <map>
#include <istream>
#include <boost/function.hpp>
#include "../string_of.hpp"

namespace lugce{
	namespace xml{
		using namespace std;
		using boost::function;

		namespace details{
			template< typename _CHAR >
			struct xmlchar;

			template<>
			struct xmlchar<char>
			{
				//inline static const char *snbsp(){ return "&nbsp;"; }
				static const char *samp(){ return "&amp;"; }
				static const char *squot(){ return "&quot;"; }
				static const char *sgt(){ return "&gt;"; }
				static const char *slt(){ return "&lt;"; }
				static const char *xml(){ return "xml"; }

				static bool is_name_first_char( const char c, std::locale& loc )
				{
					return std::isalpha(c, loc) || ( c=='_' );
				}
				static bool is_name_char( const char c, std::locale& loc )
				{
					return std::isalnum(c, loc) || ( c=='_' ) || ( c==':' );
				}
			};

			template<>
			struct xmlchar<wchar_t>
			{
				static const wchar_t *samp(){ return L"&amp;"; }
				static const wchar_t *squot(){ return L"&quot;"; }
				static const wchar_t *sgt(){ return L"&gt;"; }
				static const wchar_t *slt(){ return L"&lt;"; }
				static const char *xml(){ return "xml"; }
				static bool is_name_first_char( const wchar_t c, std::locale& loc )
				{
					if( c > 127 ) return true;
					return std::isalpha(c, loc) || ( c=='_' );
				}
				static bool is_name_char( const wchar_t c, std::locale& loc )
				{
					if( c > 127 ) return true;
					return std::isalnum(c, loc) || ( c=='_' ) || ( c==':' );
				}
			};
		};

		using namespace details;

		template< typename _CHAR >
		class simple_sax : private details::xmlchar<_CHAR>
		{	// 0: 开始
		private:		
			typedef typename string_of<_CHAR>::type SaxString;
			typedef map< SaxString, SaxString >	Attributes;
			typedef void ( simple_sax::*SaxFunc )( const _CHAR c );
			std::locale _locale;
		public:
			typedef function< void( const SaxString&, Attributes& ) >		HandleProcessor;
			typedef function< void( const SaxString&, Attributes& ) >		HandleElementStart;
			typedef function< void( const SaxString& ) >					HandleElementEnd;
			typedef function< void( const SaxString& ) >					HandleElementContent;
			void bind_processor( const HandleProcessor& handle ){ _hdl_processor=handle; }
			void bind_element( const HandleElementStart& hstart, const HandleElementEnd& hend ){ _hdl_start=hstart; _hdl_end=hend; }
			void bind_element_start( const HandleElementStart& hstart ){ _hdl_start=hstart; }
			void bind_element_end(  const HandleElementEnd& hend ){ _hdl_end=hend; }
			void bind_content( const HandleElementContent& hcontent ){ _hdl_content=hcontent; }
		protected:
			void CallProcessor( const SaxString&name, Attributes&attrs ){ if( _hdl_processor) _hdl_processor(name, attrs); }
			void CallElementStart( const SaxString&name, Attributes&attrs ){ if( _hdl_start ) _hdl_start(name, attrs); }
			void CallElementEnd( const SaxString&name ){ if( _hdl_end ) _hdl_end(name); }
			void CallElementContent( const SaxString&content ){ if( _hdl_content ) _hdl_content( content ); }
		private:
			HandleProcessor			_hdl_processor;
			HandleElementStart		_hdl_start;
			HandleElementEnd		_hdl_end;
			HandleElementContent	_hdl_content;
		public:
			simple_sax( bool part=false, const std::locale& loc=std::locale() ) : 
			  _locale(loc)
			  {  
				  _name=xml(); 
				  _status=0; 
				  _xmlpart=part; 
				  set_machines(); 
			  }

			  void init(){ XChar<_CHAR>::xml(); _status=0; _proattr.clear(); }
			  typedef map< SaxString, SaxString >	Attributes;
			  void load( basic_istream<_CHAR, char_traits<_CHAR> >& istm ) throw(...)
			  {
				  is_space()

					  for(;;)
					  {
						  _CHAR c=istm.get();
						  if( istm.eof() ) break;
						  push(c);
					  }
			  }
			  void push( const _CHAR c )
			  {
				  assert( _machines[ _status ] );
				  SaxFunc& _func=_machines[ _status ];
				  (this->*_func)( c );
			  }
		private:
			SaxFunc _machines[ 80 ];
			// 公共的状态迁徙器
			template< _CHAR _C, int status_equal >
			void machine_test_equal( const _CHAR c )
			{
				if( c==_C ) 
					_status=status_equal;
				else{
					++_status;
					push(c);
				}
			}
			// 必须为这个字符
			template< _CHAR _C >
			void machine_mast_equal( const _CHAR c ){ if( c==_C ) ++_status; else throw -1; }

			void machine_is_white( const _CHAR c )
			{ 
				if( std::isspace(c, _locale) ) ++_status; 
				else throw -1; 
			}
			void machine_skip_white( const _CHAR c )
			{   
				if( std::isspace(c, _locale) ) return;
				++_status; 
				push(c);
			}

			void machine_attrib_name( const _CHAR c )
			{
				assert( _machines[_status+1]==(&simple_sax::machine_mast_equal< '=' >) );
				if( std::isspace(c, _locale) ){
					++_status;
					_attrvalue.clear();
				}else if( '='==c ){
					_status+=2;
					_attrvalue.clear();
				}
				else _attrname+=c;
			}
			void machine_attrib_value( const _CHAR c )
			{
				if( '"'==c ){
					_proattr[ _attrname ]=_attrvalue;
					_status=_next;
				}else
					_attrvalue+=c;
			}
		private:
			void machine_xml_starts( const _CHAR c )
			{
				_proattr.clear();
				_attrname.clear();
				if( '<'==c )	
					_status= _xmlpart ? 30 : 14 ;
				else 
					throw -1;
			}

			void machine_xml_processor( const _CHAR c )
			{	// 处理器属性结束
				assert( _status==21 );
				if( '>'==c ){
					// 当处理器属性结束，允许另一个处理器，或者开始 node
					_status=24;
					push(c);
				}else 	if( _is_space( c ) ){
					++_status;	// 收到空格以后，允许另一个属性或者结尾
				}
			}

			template< _CHAR endchar, int nextstatus >
			void machine_attrib_start_or_end( const _CHAR c )
			{	// 名称后的空格出现以后，是结尾还是属性开始
				if( endchar==c ){
					++_status;
					push( c );
				}else{
					_attrname.clear();
					_attrname+=c;
					_status=Status_Get_Attrib;	// 等待属性
					_next=nextstatus;
				}
			}

			void machine_processor_attrib_first_char( const _CHAR c )
			{
				if( '?'==c )
					++_status;
				else{
					_attrname.clear();
					_attrname+=c;
					_status=Status_Get_Attrib;	// 等待属性
					_next=21;
				}
			}

			template< bool checkchar >
			void machine_xml_processor_callback( const _CHAR c )
			{
				if( checkchar && '>'!=c ) throw 1;
				CallProcessor( _name, _proattr );
				++_status;
			}

			void machine_name_first_char( const _CHAR c )
			{
				if( !is_name_first_char(c,_locale) ) throw std::exception("名称为非法字符");
				_name.clear();
				_proattr.clear();
				_name+=c;
				++_status;
			}

			void machine_name( const _CHAR c )
			{
				if( is_name_char(c, _locale) ) 
					_name+=c;
				else{
					++_status;
					push(c);
				}
			}

			void machine_processor( const _CHAR c )
			{
				if( std::isspace(c, _locale) )
					_status++;
				else if( '?'==c )
					_status=5;
			}

			void 	machine_node_salf_close( const _CHAR c )
			{
				if( c!='>' ) throw std::exception("节点自我关闭时，'/' 后缺少 '>'");
				CallElementStart( _name, _proattr );
				CallElementEnd( _name );
				_attrvalue.clear();
				_proattr.clear();
				_status=Status_Node_Content;
			}


			void machine_node_value( const _CHAR c )
			{	
				if( '<'==c ){
					CallElementContent( _attrvalue );
					++_status;
				}else
					_attrvalue+=c;
			}

			void machine_node_end( const _CHAR c )
			{
				if( '>'!=c ) throw std::exception("节点结束是，发现其他字符（期望'>'）。");
				CallElementEnd( _name );
				_attrvalue.clear();
				_status=Status_Node_Content;
			}

			template< _CHAR checkchar, int to >
			void machine_if_not( const _CHAR c )
			{
				if ( checkchar==c )
					++_status;
				else{
					_status=to;
					push(c);
				}
			}

			template< _CHAR _C >
			void machine_skip_not_char( const _CHAR c )
			{
				if( c==_C ){
					++_status;
				}
			}

			template< int to >
			void machine_goto( const _CHAR c )
			{
				_status=to;
				push(c);
			}

			void machine_after_gt( const _CHAR c )
			{
				if( '/'==c )
					_status=Status_Node_Close;
				else if( '!'==c )
					_status=Status_mark;
				else if( is_name_first_char(c,_locale) ){
					_name.clear();
					_name+=c;
					_status=Status_Node+1;
				}else{
					throw std::exception("'<'符号后有未知字符。");
				}
			}
			void machine_char_in_node( const _CHAR c )
			{
				if( '/'==c )
					_status=Status_Node_Self_Close;
				else if('>'==c ){
					CallElementStart( _name, _proattr );
					_attrvalue.clear();
					_attrname.clear();
					_proattr.clear();
					_status=Status_Node_Content;
				}else{		// 下一个属性值开始
					_attrvalue.clear();
					_attrname.clear();
					_attrname+=c;
					_next=_status-1;
					_status=Status_Get_Attrib;
				}
			}
		private:
			const static size_t Status_Node=20;
			const static size_t Status_Node_Self_Close=30;
			const static size_t Status_Node_Content=41;
			const static size_t Status_Node_Or_Close=50;
			const static size_t Status_Node_Close=55;
			const static size_t Status_Get_Attrib=60;
			const static size_t Status_mark=70;


			void set_machines()
			{
				memset( _machines, 0, sizeof(_machines) );
				size_t i=0;
				_machines[ 0 ]=&simple_sax::machine_mast_equal< '<' >;			// 第一个字符必须为 <
				_machines[ 1 ]=&simple_sax::machine_if_not< '?', Status_Node >;			// 如果第二个字符 不为 ?，直接跳转到节点名称处理
				_machines[ 2 ]=&simple_sax::machine_mast_equal< L'x' >;			
				_machines[ 3 ]=&simple_sax::machine_mast_equal< L'm' >;
				_machines[ 4 ]=&simple_sax::machine_mast_equal< L'l' >;
				_machines[ 5 ]=&simple_sax::machine_is_white;								// 空格
				_machines[ 6 ]=&simple_sax::machine_skip_white;
				_machines[ 7 ]=&simple_sax::machine_attrib_start_or_end< '?', 6 >;	// 获取属性值
				_machines[ 8 ]=&simple_sax::machine_mast_equal< '?' >;
				_machines[ 9 ]=&simple_sax::machine_xml_processor_callback<true>;			// 呼叫回调函数
				_machines[ 10 ]=&simple_sax::machine_skip_white;
				_machines[ 11 ]=&simple_sax::machine_mast_equal< '<' >;
				_machines[ 12 ]=&simple_sax::machine_if_not< '?', Status_Node >;	
				// 开始下一个 processor
				_machines[ 13 ]=&simple_sax::machine_name_first_char;
				_machines[ 14 ]=&simple_sax::machine_name;
				_machines[ 15 ]=&simple_sax::machine_skip_white;
				_machines[ 16 ]=&simple_sax::machine_if_not<'?', 7>;
				_machines[ 17 ]=&simple_sax::machine_goto<8>;

				// 开始一个 node
				i=Status_Node;
				_machines[ i++ ]=&simple_sax::machine_name_first_char;
				_machines[ i++ ]=&simple_sax::machine_name;
				_machines[ i++ ]=&simple_sax::machine_skip_white;
				_machines[ i++ ]=&simple_sax::machine_char_in_node;

				// 节点自我关闭
				i=Status_Node_Self_Close;
				_machines[ i++ ]=&simple_sax::machine_node_salf_close;

				i=Status_Node_Close;
				_machines[ i++ ]=&simple_sax::machine_name_first_char;
				_machines[ i++ ]=&simple_sax::machine_name;
				_machines[ i++ ]=&simple_sax::machine_skip_white;
				_machines[ i++ ]=&simple_sax::machine_node_end;

				// 节点内容
				i=Status_Node_Content;
				_machines[ i++ ]=&simple_sax::machine_skip_white;
				_machines[ i++ ]=&simple_sax::machine_node_value;
				_machines[ i++ ]=&simple_sax::machine_after_gt;

				i=Status_Get_Attrib;
				_machines[ i++ ]=&simple_sax::machine_attrib_name;
				_machines[ i++ ]=&simple_sax::machine_mast_equal< '=' >;
				_machines[ i++ ]=&simple_sax::machine_skip_white;
				_machines[ i++ ]=&simple_sax::machine_mast_equal< '"' >;
				_machines[ i++ ]=&simple_sax::machine_attrib_value;

				// 处理注释
				i=Status_mark;
				_machines[ i++ ]=&simple_sax::machine_mast_equal< L'-' >;
				_machines[ i++ ]=&simple_sax::machine_mast_equal< L'-' >;
				_machines[ i++ ]=&simple_sax::machine_skip_not_char< L'-' >;
				_machines[ i++ ]=&simple_sax::machine_if_not< L'-', 62 >;
				_machines[ i++ ]=&simple_sax::machine_if_not< L'>', 62 >;
				_machines[ i++ ]=&simple_sax::machine_goto<42>;
			}
		private:
			SaxString _attrname, _attrvalue;
			bool _xmlpart;	// 解析 xml 片段（无 xml 头 )
			int _status;
			int _next;
			map< SaxString, SaxString > _proattr;
			SaxString _name;
		};
	};
};
#endif //__SIMPLESAX_HPP_25EAE119_C8C9_44CB_883A_049499D9E137__
