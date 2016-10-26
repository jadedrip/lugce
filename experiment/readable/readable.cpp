// readable.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "lugce/readable_data.hpp"

int _tmain(int argc, _TCHAR* argv[])
{
	for( int x=0; x>=0; --x )
		std::cout << "Hello" << std::endl;
	int data[]={ 0x12345678, 0x14, 0x15 };
	std::string s=lugce::data_to_string( data, data+3 );

	int datavec[50];
	lugce::readable_to_data( s, datavec );

	system("pause");
	return 0;
}

