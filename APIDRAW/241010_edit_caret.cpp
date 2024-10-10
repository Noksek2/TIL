#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")
HINSTANCE g_hin;
enum {
	WIN_SIZE_W=800,
	WIN_SIZE_H= 600,
};
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	LPCWSTR strclass=L"붹";
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hin = hInstance;;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = strclass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(strclass, strclass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIN_SIZE_W, WIN_SIZE_H,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static WCHAR str[256] = {0,};
	static WCHAR buf[256] = { 0, };
	static size_t sz = 0;
	bool inv = false;
	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		RECT rt = { 0,0,WIN_SIZE_W, WIN_SIZE_H};
		//swprintf_s(buf,L"%d",sz);
		{
			for (int i = 0;i < sz;i++) {
				buf[1]='\0';
				buf[0] = str[i];
				TextOut(hdc, i*10, 0, buf, 1);
			}
		}
		//DrawText(hdc,str, sz, &rt, DT_LEFT);
		SetCaretPos(sz*10,0);
		EndPaint(hwnd, &ps);
	}
		break;
	case WM_IME_COMPOSITION:
	{
		int len=1;
		HIMC hIMC = ImmGetContext(hwnd);
		if (lp & GCS_COMPSTR) {
			len = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
			ImmGetCompositionString(hIMC, GCS_COMPSTR, str+sz, len);
			
			// 조합중인 글자 출력..
		}
		else if (lp & GCS_RESULTSTR) {

			len = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
			ImmGetCompositionString(hIMC, GCS_COMPSTR, str+sz, len);
		}
		ImmReleaseContext(hwnd, hIMC);
		sz += len-1;
		InvalidateRect(hwnd, 0, true);
	}
	return 0;
	case WM_SETFOCUS:
		CreateCaret(hwnd, 0, 10, 20);
		SetCaretPos(sz * 10, 0);
		ShowCaret(hwnd);
		return 0;
	case WM_KILLFOCUS:
		HideCaret(hwnd);
		DestroyCaret();
		return 0;
	case WM_IME_CHAR:
		return 0;
	case WM_MOUSEWHEEL:
	{
		HWND hwnd = FindWindow(0, L"다운로드");
		SetWindowText(hwnd, L"힝힝");
	}
		return 0;
	case WM_CHAR:
		if (wp == VK_BACK) {
			if (sz > 0) { str[--sz] = '\0'; inv = 1; }
		}
		else {
			if (sz < 256) {
				str[sz++] = (wchar_t)wp;
				inv = 1;
			}
		}
		if(inv)InvalidateRect(hwnd, 0, true);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hwnd, msg, wp, lp));
}
