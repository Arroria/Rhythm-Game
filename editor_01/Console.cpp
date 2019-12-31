#include "pch.h"
#include "Console.h"
#include <consoleapi.h>
#include <iostream>
//#include <consoleapi3.h>
//#include <Windows.h>

bool Console::Create()
{
	if (AllocConsole())
	{
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
		//std::ios::sync_with_stdio();
		return true;
	}
	return false;
}

void Console::Release()
{
	FreeConsole();
}
