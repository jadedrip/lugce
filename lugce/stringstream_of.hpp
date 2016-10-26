/*
 * Copyright (c) 
 * 保留所有权利。( All rights reserved. )
 * 
 * 文件名称：stringstream_of.hpp
 * 文件标识：
 * 摘    要：
 * 
 * 当前版本：1.0
 * 作    者：王琛
 * 完成日期：2008年7月18日
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 */
#if !defined( __STRINGSTREAM_OF_HPP_7715DC23_271A_4F86_BD97_1B348D92822C__ )
#define __STRINGSTREAM_OF_HPP_7715DC23_271A_4F86_BD97_1B348D92822C__
#if _MSC_VER > 1000
#pragma once
#endif

#include <sstream>
namespace lugce
{
	// istringstream
	template< typename _CHAR >
	struct istringstream_of;

	template<>
	struct istringstream_of<char>
	{
		typedef std::istringstream type;
	};
	template<>
	struct istringstream_of<wchar_t>
	{
		typedef std::wistringstream type;
	};

	// ostringstream
	template< typename _CHAR >
	struct ostringstream_of;

	template<>
	struct ostringstream_of<char>
	{
		typedef std::ostringstream type;
	};
	template<>
	struct ostringstream_of<wchar_t>
	{
		typedef std::wostringstream type;
	};

	// stringstream
	template< typename _CHAR >
	struct stringstream_of;

	template<>
	struct stringstream_of<char>
	{
		typedef std::stringstream type;
	};
	template<>
	struct stringstream_of<wchar_t>
	{
		typedef std::wstringstream type;
	};
}

#endif //__STRINGSTREAM_OF_HPP_7715DC23_271A_4F86_BD97_1B348D92822C__
