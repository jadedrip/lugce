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
* 摘要：一个简单的工厂模式的实现
* 当前版本：1.0
* 作 者：王琛
* =============================================
*/
#include <assert.h>
#include <map>

namespace lugce{
	namespace details{
		struct helper{
			virtual ~helper(){} 
			virtual void* create()=0;
		};
		template< typename T > 
		struct creater : public helper{	
			virtual void* create(){ return new T(); } 
		};

		template<>
		struct creater<void> : public helper{	
			virtual void* create(){ return nullptr; } 
		};
	};

	template< typename INDEX, typename BASE=void >
	class factory
	{
	public:
		template< typename T >
		void register_class( const INDEX& index )
		{
			assert( nullptr==_creaters[index] );
			_creaters[index]=new details::creater<T>();
		}
		BASE* create( const INDEX& index )
		{
			details::helper* p=_creaters[index];
			if(p) return (BASE*)p->create();
			return nullptr;
		}

		void destory( BASE* p ){ delete p; }
	public:
		~factory()
		{
			auto i=_creaters.begin(), e=_creaters.end();
			for( ; i!=e; ++i )
				delete i->second;
		}
	private:
		std::map< INDEX, details::helper* > _creaters;
	};
};
