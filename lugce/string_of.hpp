/*
 * Copyright (c) 2008
 * 保留所有权利。( All rights reserved. )
 * 
 * 文件名称：string_of.hpp
 * 文件标识：
 * 摘    要：
 * 
 * 当前版本：1.0
 * 作    者：王琛
 * 完成日期：2008年7月18日
 *
 */
#if !defined( __STRING_OF_HPP_7B6D6375_E63F_4847_955D_38C6D2228E09__ )
#define __STRING_OF_HPP_7B6D6375_E63F_4847_955D_38C6D2228E09__

#include <string>
namespace lugce
{
	template< typename _CHAR >
	struct string_of
	{
		typedef std::basic_string<_CHAR, std::char_traits<_CHAR>, std::allocator<_CHAR> > type;
	};
}


#endif //__STRING_OF_HPP_7B6D6375_E63F_4847_955D_38C6D2228E09__
