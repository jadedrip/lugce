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
#include <string>
#include <boost/any.hpp>
#include <boost/unordered_map.hpp>

namespace lugce
{
	/// 一个数据的集合存放器
	class aggregate
	{
	public:
		void store( const std::string& name, const boost::any& value ){ _data[name]=value; }

		template< typename T >
		void store( const T& value ){ store( typeid(T).name(), value ); }

		template< typename T >
		const T extract( const std::string& name ) const
		{
			BOOST_AUTO( i, _data.find( name ) );
			return i==_data.end() ? T() : boost::any_cast<T>(i->second);
		}
		template< typename T >
		const T extract() const{ return extract<T>( typeid(T).name() ); }
	private:
		boost::unordered_map< std::string, boost::any > _data;
	};
};
