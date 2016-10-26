#pragma once
#include <map>
#include <istream>
#include <fstream>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>

#include "string_utility.hpp"
#include "string_of.hpp"
#include "stream_of.hpp"
#include "fstream_of.hpp"

namespace lugce{
	template< typename CHAR >
	class initial_t
	{
		typedef typename string_of<CHAR>::type string_type;
		typedef typename istream_of<CHAR>::type istream_type;
		typedef typename ostream_of<CHAR>::type ostream_type;
		typedef typename ofstream_of<CHAR>::type ofstream_type;
	public:
		initial_t(){};
		initial_t( const std::string& filename ){ load(filename); };
		initial_t( const std::wstring& filename ){ load(filename); };
		initial_t( istream_type& is ){ load(is); }
	public:
		typedef std::map< string_type, string_type > item_map_t;
		typedef std::map< string_type, item_map_t > session_map_t;

		item_map_t& session( const string_type& session_name=string_type() ){ return _session[ boost::algorithm::to_lower_copy(session_name) ]; }
		void load( const std::string& filename )
		{
			typename ifstream_of<CHAR>::type file( filename.c_str() );
			if( file.is_open() )
				load( file );
		}
		void load( const std::wstring& filename )
		{
			typename ifstream_of<CHAR>::type file( filename.c_str() );
			if( file.is_open() )
				load( file );
		}
		void load( istream_type& is )
		{
			using namespace boost::algorithm;
			string_type line;
			string_type current;
			while( !is.eof() && is.good() ){
				std::getline( is, line );
				trim( line );
				if( line.empty() ) continue;
				char c=line[0];
				if( '#'==c ) continue;
				else if( '\''==c ) continue;
				else if( '['==c ){
					current=get_session( line );
					trim( current );
				}
				else{
					std::pair< string_type, string_type > p=lugce::slice_pair( line, '=' );
					trim(p.first);
					trim(p.second);
					to_lower( p.first );
					_session[ current ].insert( p );
				}
			}
		}
	public:
		item_map_t& operator[]( const string_type& session_name ){ return session(session_name); }
		string_type operator()( const string_type& session_name, const string_type& name, const string_type& default_value=string_type() )
		{
			string_type v=session( session_name )[ name ];
			return ( v.empty() ) ? default_value : v;
		}
	public:
		friend ostream_type& operator << ( ostream_type& os, const initial_t<CHAR>& ini )
		{
			for( initial_t<CHAR>::session_map_t::const_iterator i=ini._session.begin(); i!=ini._session.end(); ++i ){
				os << CHAR('[') << boost::algorithm::to_upper_copy(i->first) << CHAR(']') << std::endl;
				for( initial_t<CHAR>::item_map_t::const_iterator l=i->second.begin(); l!=i->second.end(); ++l ){
					if( !l->second.empty() )
						os << l->first << CHAR('=') << l->second << std::endl;
				}
				os << std::endl;
			}
			return os;
		}

		bool save( const std::string& filename )
		{
			ofstream_type file( filename );
			if( file.is_open() ){
				file << *this;
				return true;
			}
			return false;
		}

		bool save( const std::wstring& filename )
		{
			ofstream_type file( filename );
			if( file.is_open() ){
				file << *this;
				return true;
			}
			return false;
		}
	private:
		inline string_type get_session( const string_type& line )
		{
			return boost::algorithm::to_lower_copy( line.substr( 1, line.size()-2 ) );
		}
	private:
		session_map_t _session;
	};
	typedef initial_t<char> initial;
	typedef initial_t<wchar_t> winitial;
};
