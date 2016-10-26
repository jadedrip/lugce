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
#include <map>
#include <string>
#include <istream>
#include <sstream>
#include <boost/function.hpp>

#include "../string_of.hpp"
#include "../stream_of.hpp"

// #ifdef _DEBUG
// #	include <iostream>
// #	include <fstream>
// #	define TRACE(xx) lg << #xx << std::endl << "\tname=" << _name << std::endl << "\tattrname=" << _attrname << std::endl << "\tattrvalue=" << _attrvalue << std::endl;
// std::ofstream lg("log.txt");
// #else
#	define TRACE(x)
//#endif

namespace lugce
{
	namespace xml
	{
		using boost::function;

		class bad_format : public std::exception
		{
		public:
			virtual const char * what() const throw()
			{
				return "XML parsing failed.";
			}
		};

		class simple_sax
		{	
		private:		
			typedef std::string string_t;
		public:
			typedef std::map< string_t, string_t >	attributes_t;
		private:		
			typedef void ( simple_sax::*machine_t )( const char c );
			std::locale _locale;
			attributes_t _attrs;
#ifdef _DEBUG
			std::string _xmldoc;
#endif
		public:
			typedef function< void( const string_t&, attributes_t& ) >		HandleProcessor;	// 处理器回调
			typedef function< void( const string_t&, attributes_t& ) >		HandleElementStart;
			typedef function< void( const string_t& ) >						HandleElementEnd;
			typedef function< void( const string_t& ) >						HandleElementContent;
			typedef function< void( const string_t&, const string_t& ) >	HandleDTD;			// 简单的 DTD 处理
			void bind_processor( const HandleProcessor& handle ){ _hdl_processor=handle; }
			void bind_element_start( const HandleElementStart& hstart ){ _hdl_start=hstart; }
			void bind_element_end(  const HandleElementEnd& hend ){ _hdl_end=hend; }
			void bind_content( const HandleElementContent& hcontent ){ _hdl_content=hcontent; }
			void bind_dtd( const HandleDTD& hdtd ){ _hdl_dtd=hdtd; }
		protected:
			void CallProcessor( const string_t&name, attributes_t&attrs ){ if( _hdl_processor) _hdl_processor(name, attrs); }
			void CallElementStart( const string_t&name, attributes_t&attrs ){ if( _hdl_start ) _hdl_start(name, attrs); }
			void CallElementEnd( const string_t&name ){ if( _hdl_end ) _hdl_end(name); }
			void CallElementContent( const string_t&content ){ if( _hdl_content ) _hdl_content( content ); }
			void CallDTD( const string_t&name, const string_t& value ){ if( _hdl_dtd) _hdl_dtd(name, value); }
		private:
			HandleProcessor			_hdl_processor;
			HandleElementStart		_hdl_start;
			HandleElementEnd		_hdl_end;
			HandleElementContent	_hdl_content;
			HandleDTD				_hdl_dtd;
		public:
			simple_sax( const std::locale& loc=std::locale() ) : _locale(loc)
			{  
				_machine=&simple_sax::equal< '<', &simple_sax::machine_node_begin >;
				_flag=0;
			}

			void load( std::istream& istm )
			{
				istm.readsome()
				_locale=istm.getloc();
				for(;;){
					char c=istm.get();
					if( istm.eof() ) break;
					push(c);
				}
			}

			void load( const string_t& xmldoc, bool do_reset=true )
			{
				if( do_reset ) reset();
				std::istringstream ss;
				ss.str(xmldoc);
				load(ss);
			}

			void push( const char c )
			{
#ifdef _DEBUG
				_xmldoc.push_back(c);
#endif
				(this->*_machine)( c );
			}

			void reset()
			{
				_machine=&simple_sax::equal< '<', &simple_sax::machine_node_begin >;
				_flag=0;
				_name.clear();
				_attrname.clear();
				_attrvalue.clear();
				_attrs.clear();
#ifdef _DEBUG
				_xmldoc.clear();
#endif
			}
		private:
			unsigned char _flag;
			machine_t _machine;
			string_t _attrname, _attrvalue;
			string_t _name;
		private:
			void throw_exception( const char ){ throw bad_format(); }
			// 如果相等，设置状态到 machine_equal， 否则直接调用 machine_not_equal
			template< char _C, machine_t machine_equal, machine_t machine_not_equal >
			void is_equal( const char c )
			{
				TRACE(is_equal);
				if( c==_C ) 
					_machine=machine_equal;
				else
					(this->*machine_not_equal)(c);
			}

			// 必须为这个字符，否则抛出异常
			template< char _C, machine_t machine_equal >
			void equal( const char c )
			{ 
				TRACE(equal);
				if( c==_C ) 
					_machine=machine_equal; 
				else 
					throw_exception(c);
			}

			// 必须为引号
			template< machine_t machine_equal >
			void equal_quote( const char c )
			{ 
				TRACE(equal);
				if( c=='\'' )
					_flag|=8;
				else if( c!='"' )
					throw_exception(c);
				_machine=machine_equal; 
			}

			// 跳过空白字符，然后调用下一个
			template< machine_t machine >
			void skip_space( const char c )
			{   
				TRACE(skip_space);
				if( !std::isspace(c, _locale) ){
					_machine=machine;
					(this->*machine)(c);
				}
			}

			// 转义字符处理
			template< machine_t machine_after_attrib >
			void machine_escape_first( const char c )
			{
				TRACE(machine_escape_first);
				switch( c ){
				case 'a':	// 必须是 &amp; 或者 &apos;
					_machine=&simple_sax::is_equal< 
						'm',	
						&simple_sax::equal< 'p', &simple_sax::machine_escape_push_value< '&', machine_after_attrib > >,
						&simple_sax::equal< 'p', &simple_sax::equal< 'o', &simple_sax::equal< 's', &simple_sax::machine_escape_push_value< '\'', machine_after_attrib > > > >
					>;
					break;
				case 'q':	// 必须是 &quot; 
					_machine=&simple_sax::equal< 'u', 
						&simple_sax::equal< 'o', 
						&simple_sax::equal< 't', 
						&simple_sax::machine_escape_push_value< '"', machine_after_attrib >
						> > >;
					break;
				case 'g':	// 必须是 &gt;
					_machine=&simple_sax::equal< 't', &simple_sax::machine_escape_push_value< '>', machine_after_attrib > >;
					break;
				case 'l':	// 必须是 &lt;
					_machine=&simple_sax::equal< 't', &simple_sax::machine_escape_push_value< '<', machine_after_attrib > >;
					break;
				default:
					throw_exception(c);
				}
			}

			template< char _C, machine_t machine_after_attrib >
			void machine_escape_push_value( const char c )
			{	// 转义字符串结束，必须是 ; 结尾
				TRACE(machine_escape_push_value);
				if( ';'==c ){
					_attrvalue.push_back( _C );
					_machine=machine_after_attrib;
				}else{
					throw_exception(c);
				}
			}

			void machine_attrib_name( const char c )
			{
				TRACE(machine_attrib_name);
				if( std::isspace(c, _locale) ){	// 如果属性中出现空格
					_attrvalue.clear();
					_machine=&simple_sax::skip_space< 
						&simple_sax::equal< 
						'=', 
						&simple_sax::skip_space< &simple_sax::equal_quote< &simple_sax::machine_attrib_value >
						> > >;
				}else if( '='==c ){
					_attrvalue.clear();
					_machine=&simple_sax::skip_space< &simple_sax::equal_quote< &simple_sax::machine_attrib_value > >;
				}else 
					_attrname.push_back(c);
			}

			void machine_attrib_value( const char c )
			{
				TRACE(machine_attrib_value);
				if(( '"'==c && (_flag & 8)==0 ) || ('\''==c && (_flag & 8) ) ){
					_attrs[ _attrname ]=_attrvalue;
					_machine=&simple_sax::machine_node_name_next;
				}else if('&'==c){	// 是转义字符开始
					_machine=&simple_sax::machine_escape_first< &simple_sax::machine_attrib_value >;
				}else
					_attrvalue.push_back(c);	
			}
		private:
			void machine_content( const char c )
			{
				if( c=='<' ){
					if( !_attrvalue.empty() ) CallElementContent(_attrvalue);
					_name.clear(); _attrs.clear(); 
					_machine=&simple_sax::machine_node_begin;
				}else if('&'==c){
					_machine=&simple_sax::machine_escape_first< &simple_sax::machine_content >;
				}else{
					_attrvalue.push_back(c);
				}
			}
				 
			void machine_node_begin( const char c )
			{	// XML 的开始，允许 <?xml 或者省略 xml 版本信息
				TRACE(machine_node_begin);
				if( '?'==c ){
					_flag=1;	// 进入处理器节点
					_name.clear();
					_machine=&simple_sax::machine_node_name_next;
				}else if( '!'==c ){	// <! 后面可能是注释或者 DTD 或者 <![CDATA[
					_machine=&simple_sax::machine_after_exclamation;
				}else if( '/'==c ){
					_flag=2;
				}else{
					assert( _name.empty() );
					_name.clear();
					_name.push_back(c);
					_machine=&simple_sax::machine_node_name_next;	
				}
			}

			void machine_after_exclamation( const char c )
			{
				TRACE(machine_after_exclamation);
				if( '-'==c ){	// <!- 这应该是一个注释
					_flag=0;
					_machine=&simple_sax::equal< '-', &simple_sax::machine_note >;
				}else if( '['==c ){	// <![ 那么要求必须是 <![CDATA[
					_flag=0; 
					if( !_attrvalue.empty() ) CallElementContent(_attrvalue);
					_attrvalue.clear();
					_machine=&simple_sax::equal< 'C', &simple_sax::equal< 'D',  &simple_sax::equal< 'A',  &simple_sax::equal< 'T',  &simple_sax::equal< 'A',  &simple_sax::equal< '[', &simple_sax::machine_cdata > > > > > >;
				}else{	
					_name.clear();	// DTD 节点
					_name.push_back(c);
					_attrvalue.clear();
					_machine=&simple_sax::machine_dtd;
				}
			}

			void machine_dtd( const char c )
			{
				if( std::isspace(c, _locale) )
					_machine=&simple_sax::skip_space< &simple_sax::machine_dtd_value >;
				else
					_name.push_back(c);
			}

			void machine_dtd_value( const char c )
			{
				if( '>'==c ){
					CallDTD( _name, _attrvalue );
					_attrvalue.clear();
					_machine=&simple_sax::skip_space< &simple_sax::machine_content >;
				}else{
					_attrvalue.push_back(c);
				}
			}

			void machine_cdata( const char c )
			{
				_attrvalue.push_back(c);
				if( ']'==c ){
					if(_flag < 2)	++_flag;
				}else if( '>'==c && _flag==2 ){	// CDATA 结束
					_attrvalue.erase( _attrvalue.size()-3, 3 );	// 去除最后的 ]]>
					CallElementContent( _attrvalue );
					_attrvalue.clear();
					_machine=&simple_sax::skip_space< &simple_sax::machine_content >;
				}
			}

			void machine_note( const char c )
			{	// 注释
				if( '-'==c ){
					if( _flag < 2 ) 
						++_flag;
				}else if( '>'==c && _flag==2 ){	// 注释结束
					_machine=&simple_sax::skip_space< &simple_sax::machine_content >;
				}else{
					_flag=0;
				}
			}

			bool is_node_end( const char c )
			{
				if('?'==c && (_flag & 1 ) ){	// 处理器属性
					CallProcessor( _name, _attrs );
					_machine=&simple_sax::skip_space< &simple_sax::equal< '>', &simple_sax::skip_space< &simple_sax::machine_content > > >;
				}else if('/'==c){
					CallElementStart( _name, _attrs );
					CallElementEnd( _name );
					_machine=&simple_sax::skip_space< &simple_sax::equal< '>', &simple_sax::skip_space< &simple_sax::machine_content > > >;
				}else if('>'==c){
					if( _flag & 2 )
						CallElementEnd( _name );
					else
						CallElementStart( _name, _attrs );
					_flag=0;
					_machine=&simple_sax::skip_space< &simple_sax::machine_content >;
				}else
					return false;
				_attrvalue.clear();
				return true;
			}
			
			void machine_node_name_next( const char c )
			{
				if(	is_node_end(c) ) return;

				if( std::isspace(c,_locale) ){	// 进入属性
					_machine=&simple_sax::skip_space< &simple_sax::machine_node_name_after >;
				}else{
					_name.push_back(c);
				}
			}

			void machine_node_name_after( const char c )
			{
				if(	is_node_end(c) ) return;
				_attrname.clear();
				_attrname.push_back(c);
				_machine=&simple_sax::machine_attrib_name;
			}


		};
	};
};
#endif //__SIMPLESAX_HPP_25EAE119_C8C9_44CB_883A_049499D9E137__
