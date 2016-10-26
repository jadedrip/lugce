// url.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "lugce/url.hpp"

void parse( const std::wstring& str )
{
	std::wcout << L"Parse: " << str << std::endl;
	lugce::url_t<wchar_t> u(str);
	std::wcout << u.str() << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace std;
	try{
		parse( L"http://user@hello1.com:80" );
		parse( L"http://user:pw@192.168.1.1:80/pho.php#flag");
		parse( L"/pho.php?par=12;dt=45;nt=89");
	}catch( regex_error& e ){
		std::cout << e.what() << " at " << e.code() << std::endl;
	}
	return 0;
}

