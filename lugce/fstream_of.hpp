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
#if !defined( __FSTREAM_OF_HPP_4353065C_D31A_4067_BD64_9B6CEC96CA6F__ )
#define __FSTREAM_OF_HPP_4353065C_D31A_4067_BD64_9B6CEC96CA6F__
#if _MSC_VER > 1000
#pragma once
#endif

#include <ostream>
#include <istream>
namespace lugce
{
	// ifstream
	template< typename _CHAR >
	struct ifstream_of
	{
		typedef std::basic_ifstream<_CHAR, std::char_traits<_CHAR> > type;
	};

	// ofstream
	template< typename _CHAR >
	struct ofstream_of
	{
		typedef std::basic_ofstream<_CHAR, std::char_traits<_CHAR> > type;
	};

	// fstream
	template< typename _CHAR >
	struct fstream_of
	{
		typedef std::basic_fstream<_CHAR, std::char_traits<_CHAR> > type;
	};
}

#endif //__FSTREAM_OF_HPP_4353065C_D31A_4067_BD64_9B6CEC96CA6F__
