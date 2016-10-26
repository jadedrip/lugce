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
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* =============================================
* Copyright (c) 2010，作者保留所有权利。( All rights reserved. )
* 文件名称：singleton.hpp
* 摘 要：提供一个简单的单例
* 
* 当前版本：1.0
* 作 者：王琛
* 平台测试：Visual Studio 2008，gcc 4.3.3
* =============================================
*/
#pragma once
#include <boost/noncopyable.hpp>

namespace lugce
{
    template<typename T>
    class singleton : public boost::noncopyable
    {
    public:
        static T& instance()
        {
            T* &p=*get_instance();
            if( !p ){
                p=new T();
            }
            return *p;
        }
        static void release()
        {
            T* &p=*get_instance();
            delete p;
            p=NULL;
        }
    private:
        static T** get_instance()
        {
            static T* obj=NULL;
            return &obj;
        }
    };
};
