#pragma once
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
* =============================================
* 摘要：url 分析
* 当前版本：1.0
* 作 者：王琛
* 平台测试：Visual Studio 2010，gcc 4.3.3
* =============================================
*/
#include <map>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/typeof/typeof.hpp>
#include "string_of.hpp"

/*
The following are two example URIs and their component parts (taken loosely from RFC 3986 — STD 66):
foo://username:password@example.com:8042/over/there/index.dtb?type=animal;name=narwhal#nose
\_/   \_______________/ \_________/ \__/            \___/ \_/ \______________________/ \__/
|           |               |       |                |    |            |                |
|       userinfo         hostname  port              |    |          query          fragment
|    \________________________________/\_____________|____|/
scheme                  |                          |    |    |
|                authority                    path   |    |
|                                                    |    |
|            path                       interpretable as filename
|   ___________|____________                              |
/ \ /                        \                             |
urn:example:animal:ferret:nose               interpretable as extension
*/

namespace lugce
{
	namespace details
	{
		template< typename _CHAR >
		struct url_base
		{
			static const char * scheme_sep(){ return "://"; }
			static const char * http(){ return "http"; }
			static const char * ftp(){ return "ftp"; }
			static const char * regex_param(){ return "(\\w+)=([^&;#]*)[;&]?"; }
			static const char * regex_expression()
			{
				return "((\\w+)://)?"		// scheme
					"(([a-zA-Z]\\w*)(:([^@]+))?@)?"		// userinfo
					"(([\\w.-]+)(:(\\d+))?)?" // host
					"([^?#]*)?" // path
					"(\\?([^#]+))?" // query
					"(#(\\w+))?"; // fragment
			};
			typedef boost::smatch xsmatch;
		};

		template<>
		struct url_base<wchar_t>
		{
			static const wchar_t * scheme_sep(){ return L"://"; }
			static const wchar_t * http(){ return L"http"; }
			static const wchar_t * ftp(){ return L"ftp"; }
			static const wchar_t * regex_param(){ return L"(\\w+)=([^&;#]*)[;&]?"; }
			static const wchar_t * regex_expression()
			{
				return L"((\\w+)://)?"		// scheme
					L"(([a-zA-Z]\\w*)(:([^@]+))?@)?"		// userinfo
					L"(([\\w.-]+)(:(\\d+))?)?" // host
					L"([^?#]*)?" // path
					L"(\\?([^#]+))?" // query
					L"(#(\\w+))?"; // fragment
			};
			typedef boost::wsmatch xsmatch;
		};
	}

	template< typename _CHAR >
	class url_t
	{
		typedef typename string_of<_CHAR>::type string_t;
		typedef typename details::url_base<_CHAR> def;
	public:
		url_t() : _port(0){}
		url_t(const string_t& u) : _port(0){ set(u); }
		url_t(
			const string_t& scheme, 
			const string_t& host, 
			unsigned short port,
			const string_t& user,
			const string_t& password,
			const string_t& path
			) : _scheme(scheme), _host(host), _port(port), _user(user),
			_passwd(password)
		{
			set( path );
		}

		void set_host( const string_t& host, unsigned short port=0 )
		{ 
			_host=host; 
			if( _port>0 ) _port=port;
			if( _scheme.empty() ) _scheme="http";
			_cache.clear();
		}
		void set_port( unsigned short port=80 ){ _port=port; }
		string_t get_host() const{ return _host; }
		unsigned short get_port() const{ return _port ? _port : 80; }
		string_t get_path() const{ return _path; }

		void set_user( const string_t& user, const string_t& passwd=string_t() )
		{
			_user=user;
			if( !passwd.empty() ) _passwd=passwd;
			_cache.clear();
		}
		void set_password( const string_t& passwd ){ _passwd=passwd; _cache.clear(); }
		string_t get_password() const{ return _passwd; }
		string_t get_user() const{ return _user; }
		void set_scheme( const string_t& scheme ){ _scheme=scheme; _cache.clear(); }
		string_t get_scheme() const{ return _scheme; }
		string_t get_params( const string_t& name) const
		{ 
			BOOST_AUTO( i,_params.find( name ) );
			return (i==_params.end()) ? string_t() : i->second;
		}
		std::map< string_t, string_t >& params(){ return _params; }
		string_t str() const
		{
			if( !_cache.empty() ) return _cache;
			// 组合整个 url
			if( !_scheme.empty() )
				_cache=_scheme + def::scheme_sep();
			if( !_user.empty() ){
				_cache+=_user;
				if( !_passwd.empty() )	_cache+=_CHAR(':') + _passwd;
				_cache+=_CHAR('@');
			}
			if( !_host.empty() ) _cache+=_host;
			if( _port>0 )
				_cache+=_CHAR(':') + boost::lexical_cast< string_t >( _port );
			if( !_path.empty()  ){
				if( _CHAR('/')!=_path[0] ) _cache+=_CHAR('/');
				_cache+=_path;
			}
			if( !_params.empty() ){
				_cache+=_CHAR('?');
				BOOST_AUTO( i, _params.begin() );
				for( ;i!=_params.end(); ++i ){
					_cache+=i->first + _CHAR('=') + i->second + _CHAR('&');
				}
				_cache.erase( _cache.end()-1 );
			}
			if( !_fragment.empty() ){
				_cache+=_CHAR('#') + _fragment;
			}
			return _cache;
		}
		void set( const string_t& u )
		{ 
			using namespace boost;
			basic_regex<_CHAR, regex_traits<_CHAR> > expression( def::regex_expression() );
			smatch what;
			if(regex_match(u, what, expression)) 
			{ 
				if( what[2].matched ){ _scheme=what[2]; boost::to_lower( _scheme ); };
				if( what[4].matched ) _user=what[4];
				if( what[6].matched ) _passwd=what[6];
				if( what[8].matched ){ _host=what[8]; boost::to_lower( _host ); }
				if( what[10].matched ){
					_port=boost::lexical_cast<unsigned short>( string_t(what[10]) );
					if( _port==80 && _scheme==def::http() ) 
						_port=0;
					else if( _port==21 && _scheme==def::ftp() ) 
						_port=0;
				}
				if( what[11].matched ) _path=what[11];
				if( what[13].matched ) parse_param( what[13] );
				if( what[15].matched ) _fragment=what[15];
			} 
		}
		void reset()
		{
			_cache.clear();
			_params.clear();
			_path.clear();
			_scheme.clear();
			_user.clear();
			_passwd.clear();
			_host.clear();
			_fragment.clear();
			_port=0;
		}
	private:
		void parse_param( const string_t& query )
		{
			using namespace boost;
			basic_regex<_CHAR, regex_traits<_CHAR> > expression( def::regex_param() );
			smatch what; 
			match_flag_type flags = match_default; 
			BOOST_AUTO( start, query.begin() );
			while(regex_search( start, query.end(), what, expression, flags)) 
			{ 
				string_t name=string_t( what[1].first, what[1].second );
				boost::to_lower( name );
				_params[ name ]=string_t( what[2].first, what[2].second );
				start = what[0].second; 
				// update flags: 
				flags |= match_prev_avail; 
				flags |= match_not_eol; 
			} 
		}
	private:
		mutable string_t _cache;
		std::map< string_t, string_t > _params;
		string_t _path;
		string_t _scheme;
		string_t _user;
		string_t _passwd;
		string_t _host;
		string_t _fragment;
		unsigned short _port;
	};
		
	typedef url_t<char> url;
	typedef url_t<wchar_t> wurl;
};
