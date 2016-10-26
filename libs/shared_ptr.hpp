#pragma once
#include "config.hpp"

#ifdef STL_TR1
#	include<memory>
#else
#	include<boost/shared_ptr.hpp>
	namespace std
	{
		typedef boost::shared_ptr shared_ptr;
	};
#endif
