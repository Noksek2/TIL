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
int GetCharWidth(HDC hdc, TCHAR *ch, int len)
{
     SIZE sz;

     GetTextExtentPoint32(hdc, ch, len, &sz);

     return sz.cx;

}
class TempClass{
	int comp;
	int font_h;
public:
	
	HWND hwnd;
	WCHAR buf[256];
	WCHAR str[256];
	bool inv;
	size_t sz;
	//HINSTANCE c_hin;
	TempClass(){
		comp=0;
		buf[0]=str[0]=0;
		sz=0;
		inv=false;
	}
	void onCreate(HWND _hwnd){
		TEXTMETRIC tm;
		hwnd=_hwnd;
		HDC hdc=GetDC(hwnd);
		GetTextMetrics(hdc,&tm);
		font_h=tm.tmHeight;
		ReleaseDC(hwnd,hdc);
	}
	void onPaint(){
		
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		RECT rt = { 0,0,WIN_SIZE_W, WIN_SIZE_H};
		{
			TextOut(hdc, 0, 0, str, sz);
		}
		int caret_w;
		if(comp)
			caret_w=GetCharWidth(hdc,str+sz-comp,comp);
		else 
			caret_w=2;
		SIZE csz;
		GetTextExtentPoint32(hdc,str,sz-comp,&csz);
		CreateCaret(hwnd,0,caret_w,font_h);
		SetCaretPos(csz.cx,0);
		EndPaint(hwnd, &ps);
	}
	void onImeInput(WCHAR c, LPARAM lp){
		int len=0;
		HIMC hIMC = ImmGetContext(hwnd);
		if (lp & GCS_RESULTSTR) {
			len = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
			sz-=comp;
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, str+sz, len);
			comp=0;
			// 조합중인 글자 출력..
		}

		if (lp & GCS_COMPSTR) {
			len = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
			sz-=comp;
			ImmGetCompositionString(hIMC, GCS_COMPSTR, str+sz, len);
			comp=len/2;
			// 조합중인 글자 출력..
		}
		else comp=0;
		
		
		ImmReleaseContext(hwnd, hIMC);
		sz += (len<0?0:(len/2));
		InvalidateRect(hwnd, 0, true);
	}
	void onChar(wchar_t wp){
		
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
	}
	void onSetFocus();
};
void TempClass::onSetFocus(){
		CreateCaret(hwnd, 0, 2, font_h);
		SetCaretPos(sz * 15, 0);
		ShowCaret(hwnd);
}
TempClass *tclass;
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
	static int comp=0;
	switch (msg) {
	case WM_CREATE: tclass=new TempClass(); tclass->onCreate(hwnd);return 0;
	case WM_PAINT: 
		tclass->onPaint();
		break;
	case WM_IME_COMPOSITION:
		tclass->onImeInput(wp,lp);
	return 0;
	case WM_SETFOCUS:
		tclass->onSetFocus();
		return 0;
	case WM_KILLFOCUS:
		HideCaret(hwnd);
		DestroyCaret();
		return 0;
	case WM_MOUSEWHEEL:
		return 0;
	case WM_CHAR:
		tclass->onChar(wp);
		return 0;
	case WM_DESTROY:
		delete tclass;
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hwnd, msg, wp, lp));
}
