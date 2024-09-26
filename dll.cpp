#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
using namespace std;

void main()
{
	HMODULE hdll=LoadLibraryA("RealDll.dll");
	if (!hdll)return;
	typedef void* (*F_vs)(size_t s);


	F_vs dllfn=(F_vs)GetProcAddress(hdll, "dllmalloc");
	auto mem=(int*)dllfn(100);
	free(mem);

	FreeLibrary(hdll);

}

#ifdef SETDLL
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif
extern "C" DLL void* dllmalloc(size_t);
