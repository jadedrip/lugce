/*
 * Copyright (c) 2008
 * 保留所有权利。( All rights reserved. )
 * 
 * 文件名称：stringstream_of.hpp
 * 文件标识：
 * 摘    要：
 * 
 * 当前版本：1.0
 * 作    者：王琛
 * 完成日期：2008年7月18日
 */
#if !defined( __STREAM_OF_HPP_2E4C4079_069E_458B_8026_4771F060A3A0__ )
#define __STREAM_OF_HPP_2E4C4079_069E_458B_8026_4771F060A3A0__
#if _MSC_VER > 1000
#pragma once
#endif

#include <ostream>
#include <istream>
namespace lugce
{
	// istream
	template< typename _CHAR >
	struct istream_of
	{
		typedef std::basic_istream<_CHAR, std::char_traits<_CHAR> > type;
	};

	// ostream
	template< typename _CHAR >
	struct ostream_of
	{
		typedef std::basic_ostream<_CHAR, std::char_traits<_CHAR> > type;
	};
}




#endif //__STREAM_OF_HPP_2E4C4079_069E_458B_8026_4771F060A3A0__
