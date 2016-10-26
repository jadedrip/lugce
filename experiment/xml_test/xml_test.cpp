// xml_test.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "xml_test.h"
//#define XML_STEP 1

#include <memory>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "lugce/xml/simple_sax.hpp"
#include "lugce/xml/xml_writer.hpp"
#include "lugce/xml/dom.hpp"


/// 遍历目录strPath中的所有文件
int GetAllFile(const std::string &strPath)
{
	namespace fs = boost::filesystem;	
	// 得到配置文件夹.	
	fs::path full_path( fs::initial_path() );
	full_path = fs::system_complete( fs::path(strPath, fs::native ) );	
	unsigned long file_count = 0;
	unsigned long dir_count = 0;
	unsigned long err_count = 0;	
	if ( !fs::exists( full_path ) )
	{
		std::string strMsg = "找不到文件目录,请检查该目录是否存在: ";
		strMsg.append(full_path.native_file_string());	 return -1;
	}	// 遍历指定的文件夹,得到所有的文件名.
	if ( fs::is_directory( full_path ) )
	{
		fs::directory_iterator end_iter;
		for( fs::directory_iterator dir_itr( full_path ); dir_itr != end_iter; ++dir_itr ){
			try
			{
				if ( fs::is_directory( *dir_itr ) )
				{
					std::string strSubDir(full_path.native_directory_string()) ;
					strSubDir.append( "\\ ");
					strSubDir.append(dir_itr-> leaf());	
					GetAllFile(strSubDir);	// 如果有子目录,则递归遍历.
				}
				else
				{
					// 先组成包括完整路径的文件名
					std::string strFileName(full_path.native_directory_string());
					strFileName.append( "\\");
					strFileName.append(dir_itr-> leaf());
					//判断文件是否为XML配置文件
					if( strFileName.substr( strFileName.size()-4 )==".xml" ){
						try{
							std::cout << "解析:" << strFileName << std::endl;
							std::ifstream f( strFileName );
							lugce::xml::document dx( f );
							std::cout << "输出:" << std::endl;
							std::cout << dx.xml() << std::endl << std::endl;
						}catch( std::exception& e ){
							std::cout << "解析错误:" << e.what() << std::endl;
						}
					}

					
					fs::path full_file( fs::initial_path() );
					full_file = fs::system_complete(fs::path(strFileName, fs::native));	 //加载解析文件中的信息.
					//readFileInfo(full_file.native_directory_string());
				}
			}
			catch ( const std::exception & ex )
			{
				++err_count;	 
			}
		}// <--for()
		std::cout <<"成功遍历所有设备配置文件. ";
	}
	else // must be a file
	{ 
		std::string strMsg = full_path.native_file_string();
		strMsg.append( ",不是文件目录. ");
		return -1;
	}	
	return err_count;
}

int main(int _Argc, char ** )
{
	using namespace std;
	using namespace lugce::xml;

	GetAllFile(".");
#ifndef _DEBUG
	//system( "pause" );
#endif
	return 0;
}