
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib,"comctl32.lib")
class CPen {
	public:
		HDC hdc;
		HPEN hpen, oldpen;
		CPen(HDC _hdc,int color,int width=1,int style= PS_SOLID) {
			hdc = _hdc;
			hpen=CreatePen(style, width,color);
			oldpen = (HPEN)SelectObject(hdc, hpen);
		}
		~CPen() {
			SelectObject(hdc, oldpen);
			DeleteObject(hpen);
		}
};
class CBrush
{
public:
	HDC hdc;
	HBRUSH hbrush, old;
	CBrush(HDC _hdc,int color) {
		hdc = _hdc;
		hbrush = CreateSolidBrush(color);
		old = (HBRUSH)SelectObject(hdc, hbrush);
	}
	~CBrush() {
		SelectObject(hdc, old);
		DeleteObject(hbrush);
	}

};
class CFont {
public:
	HDC hdc;
	HFONT hfont, old;
	CFont(HDC _hdc, LPCWSTR fontname) {
		hdc = _hdc;
		hfont=CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, fontname);
		old = (HFONT)SelectObject(hdc, hfont);
	}
	CFont(HDC _hdc, LOGFONT* lf) {
		hdc = _hdc;
		hfont = CreateFontIndirect(lf);
		//CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, fontname);
		old = (HFONT)SelectObject(hdc, hfont);
	}

	~CFont() {
		SelectObject(hdc, old);
		DeleteObject(hfont);
	}
};
struct CPoint {
	USHORT x;
	USHORT y;
};

class CScroll{
public:
	WORD pos;
	HWND hscroll;
	WORD pos_max,inc;
	void Create(HWND hwnd,HINSTANCE g_hin,UINT id,int x,int y,WORD _max=255,int width=200){
	hscroll=CreateWindow(L"scrollbar",0,WS_CHILD|WS_VISIBLE|
		SBS_HORZ,x,y,width,20,hwnd,(HMENU)id,g_hin,0);
	SetScrollRange(hscroll,SB_CTL,0,_max,true);
	pos_max=_max;
	inc=max(1,pos_max/10);
	SetScrollPos(hscroll,SB_CTL,0,true);
	}
	void Process(WPARAM wp,LPARAM lp){
		WORD msgtype=LOWORD(wp),
			temp=HIWORD(wp);
		switch(msgtype){
		case SB_LINELEFT:pos=max(0,pos-1);break;
		case SB_LINERIGHT:pos=min(pos_max,pos+1);break;
		case SB_PAGELEFT:pos=max(0,pos-inc);break;
		case SB_PAGERIGHT:pos=min(pos_max,pos+inc);break;
		case SB_THUMBTRACK:pos=temp;break;
		}
		SetScrollPos((HWND)lp,SB_CTL,pos,true);
	}
};

class CWindow{
public:
	HINSTANCE g_hin;
	CScroll scroll,scroll1,scroll2;
	int color;
	LOGFONT lf;
	COLORREF fontcol;
	CHOOSEFONT fnt;
	CWindow(){ZeroMemory(&fnt,sizeof(CHOOSEFONT));color=RGB(255,255,255);}
	virtual void Create(HWND hwnd);
	virtual void Paint(HWND hwnd);
	virtual void Command(HWND hwnd,WPARAM wp,LPARAM lp);
	virtual void Hscroll(HWND hwnd,WPARAM wp,LPARAM lp);
	virtual void Lbtndown(HWND hwnd,WPARAM wp,LPARAM lp){
	
	}
	virtual void Mousewheel(HWND hwnd,WPARAM wp,LPARAM lp){
		SendMessage(scroll1.hscroll,WM_HSCROLL,SB_LINERIGHT,0);
	}
};
