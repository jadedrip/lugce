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
#include <map>
#include <string>
#include <ostream>
#include <assert.h>
#include <exception>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "url.hpp"
namespace lugce
{
	class bad_protocol : public std::exception
	{
	public:
		virtual const char * what() const throw(){ return "Bad protocol."; }
	};

	/// 一个简单 http 协议分析、组装器
	class http 
	{
	public:
		http() : _data(new my_data)
		{
			_data->_version="1.1";
			_data->_code=0;
			_data->_parse_header=true;
			_data->_content_requisite=0;
		}
		http( const std::string& method, const std::string& version="1.1" ) : _data(new my_data)
		{
			_data->_version=version;
			_data->_code=0;
			_data->_parse_header=true;
			_data->_method=method;
			_data->_content_requisite=0;
		}
		http( const unsigned short code, const std::string& version="1.1" ) : _data(new my_data)
		{
			_data->_version=version;
			_data->_code=code;
			_data->_parse_header=true;
			_data->_content_requisite=0;
		}

		void host( const std::string& host, const unsigned short port=80 )
		{
			modify();
			_data->_host=host;
			if( port!=80 ){
				_data->_host+=':' + boost::lexical_cast< std::string >( port );
			}
		}

		std::string host() const
		{ 
			if( _data->_host.empty() ){
				std::string h=_data->_url.get_host();
				unsigned short port=_data->_url.get_port();
				if( !h.empty() && port!=80 )
					h+=':' + boost::lexical_cast<std::string>( port );
				return h;
			}
			return _data->_host; 
		}

		bool is_unlimit_body() const{ return _data->_content_requisite==(size_t)-1; }

		void version( const std::string& v ){ modify(); _data->_version=v; }
		std::string version() const{ return _data->_version; }

		void code( const unsigned short code ){ modify(); _data->_code=code; _data->_method.clear(); }
		unsigned short code() const{ return _data->_code; }

		void method( const std::string& m ){ modify(); _data->_method=m; _data->_code=0; }
		std::string method() const{ return _data->_method; }

		void url( const url_t<char>& u ){ modify(); _data->_url=u; }
		void url( const char * u ){ modify(); _data->_url=url_t<char>(u); }
		url_t<char> url() const{ return _data->_url; }
		std::string full_url() const{ return "http://" + _data->_host + _data->_url.str(); }

		void body( const std::string& body )
		{ 
			_data->_body=body; 
			//_data->_params["Content-Length"]=boost::lexical_cast<std::string>(_data->_body.size());
			_data->_cache.clear(); 
		}

		std::string body() const{ return _data->_body; }

		std::string& operator[]( const std::string& name )
		{
			modify();
			return _data->_params[name];
		}

		const std::string operator[]( const std::string& name ) const
		{
			BOOST_AUTO( i, _data->_params.find(name) );
			if( i==_data->_params.end() )
				return std::string();
			else
				return i->second;
		}

		const std::string& str() const
		{
			std::ostringstream os;
			if( !_data->_cache.empty() ) return _data->_cache;
			if( _data->_method.empty() ){
				assert( _data->_code > 0 );
				os << "HTTP/" << _data->_version << ' ' << _data->_code << " OK\r\n";
			}else{
				// 
				os << _data->_method << ' ' << _data->_url.str() << " HTTP/" << _data->_version << "\r\n";
				if( !_data->_host.empty() ){
					assert( _data->_version=="1.1" );
					os << "Host: " << _data->_host << "\r\n";
				}else{
					os << "Host: " << _data->_url.get_host() << ":" << _data->_url.get_port() << "\r\n";
				}
			}

			BOOST_AUTO( i, _data->_params.begin() );
			for( ; i!=_data->_params.end(); ++i ){
				if( i->first!="Content-Length" )
	 				os << i->first << ": " << i->second << "\r\n";
			}
			os << "Content-Length: " << boost::lexical_cast<std::string>(_data->_body.size()) << "\r\n\r\n";
			if( !_data->_body.empty() ) os << _data->_body;
			_data->_cache=os.str();
			return _data->_cache;
		}

		bool parse( const std::string& data, bool no_case=false ){ return parse( data.c_str(), data.size(), no_case ); }

		bool parse( const char * buffer, const size_t length, bool no_case=false )
		{
			using namespace std;
			if( _data->_parse_header ){	// 如果正在分析头部
				for( size_t i=0; i<length; ++i ){
					const char c=buffer[i];
					_data->_cache.push_back(c);
					if(c=='\n'){
						if( _data->_cache.size()>1 && _data->_cache[_data->_cache.size()-2]=='\r' ){
							_data->_cache.erase( _data->_cache.size()-2 );
							if( parse_header(_data->_cache,no_case) ){	// 头部分析完成
								_data->_parse_header=false;
								_data->_cache.clear();
								BOOST_AUTO( x, _data->_params.find(no_case ? "content-length" : "Content-Length") );
								if( x!=_data->_params.end() ){
									// 如果找到了 Content-Length, 那么以这个为准
									_data->_content_requisite=boost::lexical_cast<unsigned long>(x->second);
									if( _data->_content_requisite==0 )	// 没有 body
										return true;
									else if( _data->_content_requisite<=(length-i-1) ){
										_data->_body.assign(buffer+i+1, _data->_content_requisite );
										return true;
									}else{	// body 的数据还不够
										_data->_body.assign(buffer+i+1, length-i-1 );
										return false;
									}
								}else{	// 需要注意，如果是不限制 body 大小，这个函数返回 true 仅代表头部解析完成
									_data->_content_requisite=(size_t)-1;	// Unlimit;
									_data->_body.assign(buffer+i+1, length-i-1 );
									return true;
								}
							}
							_data->_cache.clear();
						}else{
							throw bad_protocol();
						}
					}
				}
			}else{
				_data->_body.append( buffer, buffer+length );
				if( _data->_body.size() > _data->_content_requisite ){
					_data->_body.erase( _data->_content_requisite );
					return true;
				}
				return ( _data->_body.size()==_data->_content_requisite );
			}
			return false;
		}
		
		void reset()
		{
			if( _data.use_count()>1 ){
				_data.reset( new my_data );
			}else{
				_data->_cache.clear();
				_data->_host.clear();
				_data->_method.clear();
				_data->_body.clear();
				_data->_params.clear();
				_data->_url.reset();
			}
			_data->_code=0;
			_data->_version="1.1";
			_data->_content_requisite=0;
			_data->_parse_header=true;
		}

		friend std::ostream& operator << ( std::ostream& os, const http& h )
		{
			return os << h.str();
		}
	private:
		void modify()
		{
			if( _data.use_count()>1 ){
				my_data *n=new my_data( *_data );
				_data.reset(n);
			}
			_data->_cache.clear();
		}
		bool parse_header( const std::string& line, bool no_case )
		{
			using namespace std;
			if( _data->_code==0 && _data->_method.empty() ){	// 如果是第一行
				//HTTP/1.1 401 Unauthorized
				if( "HTTP/"==line.substr(0,5) ){	// 这是 http 回应包
					string::size_type i=line.find( ' ', 5 );
					if(i==string::npos) throw bad_protocol();
					_data->_version=line.substr(5,i-5);
					string::size_type e=line.find(' ', i+1);
					if(e==string::npos) throw bad_protocol();
					_data->_code=boost::lexical_cast<unsigned short>(line.substr(i+1,e-i-1));
				}else{
					string::size_type i=line.find( ' ' );
					if(i==string::npos) throw bad_protocol();
					_data->_method=line.substr(0,i);

					string::size_type e=line.find( ' ', i+1 );
					if(e==string::npos) throw bad_protocol();
					_data->_url=line.substr(i+1, e-i-1);

					if( "HTTP/"==line.substr(e+1,5) ){
						_data->_version=line.substr(e+6);
					}else{
						 throw bad_protocol();
					}
				}
			}else if( line.empty() ){	// 最后一行
				return true;
			}else{
				string::size_type i=line.find(':');
				if(i==string::npos) throw bad_protocol();

				string n=line.substr(0,i);
				if( no_case ) boost::algorithm::to_lower(n);
				string & v=_data->_params[ n ];
				if(v.empty())	// 如果属性已经存在，另起一行保存信息
					v=boost::trim_copy(line.substr(i+1));
				else
					v+="\r\n" + boost::trim_copy(line.substr(i+1));
			}
			return false;
		}
	private:
		struct my_data{	// 使用指针的形式，防止复制对象的时候复制整个对象
			std::string _cache;
			std::string _host;
			std::string _version;
			std::string _method;
			std::string _body;
			unsigned short _code;
			size_t _content_requisite;
			bool _parse_header;
			url_t<char> _url;
			std::map< std::string, std::string > _params;
		};
		boost::shared_ptr<my_data> _data;
	};
};
