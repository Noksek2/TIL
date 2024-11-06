#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")
HINSTANCE g_hin;
enum {
	WIN_SIZE_W=400,
	WIN_SIZE_H= 300,
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

	void set_linecaret_pos(HDC hdc, int line,int s, bool iscreate=true){
		int caret_w;
		if(comp)
			caret_w=GetCharWidth(hdc,str+caret_pos-comp,comp);
		else 
			caret_w=2;
		SIZE csz;
		GetTextExtentPoint32(hdc,str+s,caret_pos-s-comp,&csz);
		if(iscreate)CreateCaret(hwnd,0,caret_w,font_h);
		SetCaretPos(csz.cx,line*line_h);
	}
	void getLine(int target_line,int &s, int &e){
		WCHAR* p=str;
		for(int line=0;;line++){
			if(line==target_line)break;
			
			while(true){
				if(*p=='\r'){break;}
				else if(*p==0){s=-1;e=-1;return;}
				p++;
			}
			p+=2;

		}
		s=p-str;
		while(true){
			if(*p=='\r' || *p==0){break;}
			p++;
		}
		e=p-str;//실제 포인터 end + 1(첫문자~'\r')

	}
public:
	
	HWND hwnd;
	WCHAR buf[256];
	WCHAR str[256];
	size_t sz;

	int caret_pos,nowline,real_caret_pos;

	int line_ratio, line_h;
	//HINSTANCE c_hin;
	TempClass(){
		caret_pos=0;
		comp=0;
		buf[0]=str[0]=0;
		sz=0;
		
		line_ratio=120;
	}
	void onCreate(HWND _hwnd){
		TEXTMETRIC tm;
		hwnd=_hwnd;
		HDC hdc=GetDC(hwnd);
		GetTextMetrics(hdc,&tm);
		font_h=tm.tmHeight;
		ReleaseDC(hwnd,hdc);
		line_h=int(font_h*line_ratio/100);
	}
	
	void onPaint(){
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		RECT rt = { 0,0,WIN_SIZE_W, WIN_SIZE_H};
		WCHAR* buf=str;
		int s=0,e=0,_off=0;
		int line=0;
		//병신
		while(true){
			if(caret_pos==_off){
				set_linecaret_pos(hdc,line,s,true);
			}
			if(buf[_off]=='\r'||sz==_off){
				TextOut(hdc,0,line*line_h,str+s,_off-s);
				if(sz==_off){break;}
				_off+=2;
				line++;
				s=_off;
			}
			else _off++;
		}
		EndPaint(hwnd, &ps);
	}
	void onImeInput(WCHAR c, LPARAM lp){
		int len=0;
		HIMC hIMC = ImmGetContext(hwnd);
		if (lp & GCS_RESULTSTR) {
			len = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
			moveChar(str,caret_pos,sz-1,len/2-comp);
			sz-=comp;
			caret_pos-=comp;
			//mov=-comp+len;
			
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, str+caret_pos, len);
			comp=0;
			// 조합중인 글자 출력..
		}

		if (lp & GCS_COMPSTR) {
			len = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
			moveChar(str,caret_pos,sz-1,len/2-comp);
			sz-=comp;
			caret_pos-=comp;
			ImmGetCompositionString(hIMC, GCS_COMPSTR, str+caret_pos, len);
			comp=len/2;
			// 조합중인 글자 출력..
		}
		else comp=0;
		
		
		ImmReleaseContext(hwnd, hIMC);
		sz += len/2;
		caret_pos+=len/2;
		//(len<0?0:(len/2));
		InvalidateRect(hwnd, 0, true);
	}
	void moveChar(WCHAR* mstr,int pos, int endpos,int mov){
		int size=endpos-pos+1;
		if(mov==0||size<=0)return;
		
		memmove(mstr+pos+mov,mstr+pos,size*2);
		if(mov<0){
			mstr[endpos+mov+1]=0;
		}
		else{
			//if(size>0)memmove(mstr+pos*2+mov*2,mstr+pos*2,size*2);
		}
	}
	void onChar(wchar_t wp){
		bool inv=false;
		if (wp == VK_BACK) {
			if (caret_pos > 0) {
				//str[--caret_pos] = '\0';
				inv = 1;
				if(str[caret_pos-1]=='\n'){
					moveChar(str,caret_pos,sz-1,-2);
					sz--;
					caret_pos--;
				}
				else moveChar(str,caret_pos,sz-1,-1);
				sz--;
				caret_pos--;
			}
		}
		else if(wp=='\r'){
			if (caret_pos <= 256-2) {
				moveChar(str,caret_pos,sz-1,2);
				str[caret_pos++] = '\r';
				str[caret_pos++] = '\n';
				inv = 1;
				sz+=2;
			}
		}
		else if(wp==VK_ESCAPE || wp<' '){
		}
		
		else {
			if (caret_pos < 256) {
				moveChar(str,caret_pos,sz-1,1);
				str[caret_pos++] = (wchar_t)wp;
				inv = 1;
				sz++;
			}
		}
		
		if(inv)InvalidateRect(hwnd, 0, true);
	}
	void onSetFocus();
	void onKeyDown(WPARAM wp){
		//bool inv=false;
		switch(wp){
		case VK_LEFT:
			if(caret_pos>0){caret_pos--;
			if(str[caret_pos]=='\n')caret_pos--;
			}
			else return;
			break;
		case VK_RIGHT:
			if(caret_pos<sz){caret_pos++;
				if(str[caret_pos]=='\n')caret_pos++;
			}else return;
			break;
		case VK_HOME:
			caret_pos=0;
			break;
		case VK_DELETE:
			if(caret_pos<sz){
				if(str[caret_pos]=='\r'){
					moveChar(str,caret_pos+2,sz-1,-2);
				sz-=2;
				}
				else{
					moveChar(str,caret_pos+1,sz-1,-1);
					sz--;
				}
			}
			else return;
			break;
		case VK_END:
			caret_pos=sz;
			break;
		}
		InvalidateRect(hwnd, 0, true);
		return;
		
	}
};
void TempClass::onSetFocus(){
	InvalidateRect(hwnd,0,true);/*
		CreateCaret(hwnd, 0, 2, font_h);
		SetCaretPos(sz * 15, 0);
		ShowCaret(hwnd);*/
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
	WndClass.hCursor = LoadCursor(NULL,IDC_IBEAM);
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
	case WM_KEYDOWN:
		
		tclass->onKeyDown(wp);
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
