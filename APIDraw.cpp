#include <Windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#define RESOURCE
#ifdef RESOURCE
#include "resource.h"
#endif

//#define DIALOG
#ifdef DIALOG
using namespace Gdiplus;
#pragma comment(lib,"gdiplus")
#pragma comment(lib,"comctl32.lib")
HWND hRed, hGreen, hBlue;
HINSTANCE g_hin;
bool CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_INITDIALOG:
		return true;
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wp));
		return true;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wp));
			return true;
		}
		break;
	}
	return false;
}
LRESULT DisplayMessage(HINSTANCE hin, HWND hwnd) {
	HGLOBAL hg;
	LPDLGTEMPLATE plate;
	LPDLGITEMTEMPLATE item;
	LPWORD pw;
	LPWSTR pwsz;
	int n;
	hg = GlobalAlloc(GMEM_ZEROINIT, 1024);
	plate = (LPDLGTEMPLATE)GlobalLock(hg);

	plate->style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION;
	plate->cdit = 1;
	plate->x = 10;
	plate->y = 10;
	plate->cx = 200;
	plate->cy = 100;


	pw = (LPWORD)(plate + 1);
	*pw++ = 0;
	*pw++ = 0;
	pwsz = (LPWSTR)pw;
	n = MultiByteToWideChar(CP_ACP, 0, "김대중 Runtime", -1, pwsz, 50) + 1;
	pw += n;

	pw = (LPWORD)((ULONG)pw + 3 & 0xfffffffc);
	item = (LPDLGITEMTEMPLATE)pw;
	item->x = 50; item->y = 50;
	item->cx = 100; item->cy = 20;
	item->id = IDOK; item->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;

	pw = (LPWORD)(item + 1);
	*pw++ = 0xFFFF;
	*pw++ = 0x0080;
	pwsz = (LPWSTR)pw;
	n = MultiByteToWideChar(CP_ACP, 0, "확인", -1, pwsz, 50) + 1;
	pw += n;
	pw = (LPWORD)((ULONG)pw + 3 & 0xfffffffc);
	*pw++ = 0;

	LRESULT res;
	GlobalUnlock(hg);
	res = DialogBoxIndirect(hin, (LPDLGTEMPLATE)hg,
		hwnd, (DLGPROC)DialogProc);
	GlobalFree(hg);
	return res;
}
#endif
enum {
	DrawType_None, DrawType_Circle, DrawType_Square, DrawType_Line,
	Menu_Delete, Menu_Clear, Menu_Exit
};
enum {
	Drag_None = 0, Drag_Draw = 1, Drag_Move, Drag_Size
};
enum {
	Line_Normal,Line_Reverse
};
inline int LP_GetMouseX(LPARAM lp) { return LOWORD(lp); }
inline int LP_GetMouseY(LPARAM lp) { return HIWORD(lp); }
struct ShapeList {
	struct ShapeData {
		RECT rt;
		UINT depth;
		USHORT type;
		UCHAR linetype;
		
	};
	ShapeData* data;
	UINT capa, len;
	ShapeList() {
		capa = 100; len = 0;
		data = (ShapeData*)malloc(sizeof(ShapeData) * capa);
		//for(int i=0;i<capa;i++)
		memset(data, 0, sizeof(ShapeData) * capa);
	}
	void AdjustRect(USHORT type,RECT *rt) {
		long r;
		if (type == DrawType_Line) {
			if (rt->top > rt->bottom) {
				r = rt->bottom;
				rt->bottom = rt->top;
				rt->top = r;

				r = rt->right;
				rt->right = rt->left;
				rt->left = r;
			}
			return;
		}
		if (rt->top > rt->bottom) {
			r = rt->bottom;
			rt->bottom = rt->top;
			rt->top = r;
		}
		if (rt->left > rt->right) {
			r = rt->right;
			rt->right = rt->left;
			rt->left = r;
		}
	}
	bool AdjustPtInRect(USHORT type,RECT *rt,POINT pt) {
		long r;
		RECT rrt;
		SetRect(&rrt, rt->left, rt->top, rt->right, rt->bottom);
		if (type==DrawType_Line && rrt.left > rrt.right) {
			r = rrt.right;
			rrt.right = rrt.left;
			rrt.left = r;
		}
		return PtInRect(&rrt, pt);
	}
	void append(USHORT type, RECT* mrt) {
		if (len >= capa)return;
		data[len].type = type;
		AdjustRect(type, mrt);
		memcpy(&data[len++].rt, mrt, sizeof(RECT));
	}
	void delete_shape(ShapeData* d) {
		if (len > 0 && d < data + len)
			*d = data[--len];
	}
	void clear() {
		len = 0;
	}
	ShapeData* find(USHORT x, USHORT y) {
		POINT pt = { x,y };
		for (auto d = data + len - 1; d >= data; d--) {
			if (AdjustPtInRect(d->type,&d->rt, pt))
				return d;
		}
		return 0;
	}
	~ShapeList() {
		if (data) {
			free(data); data = 0;
		}
	}
};
class APIClass {
private:
	USHORT mx, my, oldx, oldy;
	inline void DrawOneTrack(HDC hdc, USHORT x, USHORT y) {
		Rectangle(hdc, x - 5, y - 5, x + 5, y + 5);
	}
	void GetTracker(UCHAR tracknum, RECT* rt) {
		RECT* srt = &shapenum->rt;
		int x, y;
		switch (tracknum) {
		case 1:x = srt->left; y = srt->top; break;
		case 2:x = (srt->left + srt->right) / 2; y = srt->top; break;
		case 3:x = srt->right; y = srt->top; break;
		case 4:x = srt->left; y = (srt->top + srt->bottom) / 2; break;
		case 5:x = srt->right; y = (srt->top + srt->bottom) / 2; break;
		case 6:x = srt->left; y = srt->bottom; break;
		case 7:x = (srt->left + srt->right) / 2; y = srt->bottom; break;
		case 8:
			x = srt->right; y = srt->bottom;
			break;
		}
		SetRect(rt, x - 5, y - 5, x + 5, y + 5);
	}
	int TrackerHit(USHORT x, USHORT y) {
		POINT pt;
		pt.x = x;
		pt.y = y;
		if (!shapenum)return 0;
		RECT trackrt;
		if (shapenum->type != DrawType_Line) {
			for (int i = 1; i <= 8; i++) {
				GetTracker(i, &trackrt);
				if (PtInRect(&trackrt, pt))return i;
			}
		}
		else {
			GetTracker(1, &trackrt);
			if (PtInRect(&trackrt, pt))return 1;
			GetTracker(8, &trackrt);
			if (PtInRect(&trackrt, pt))return 8;
		}
		return 0;
	}
	void DrawTemp(USHORT type,RECT* rt) {
		HDC hdc = GetDC(api_hwnd);
		SetROP2(hdc, R2_XORPEN);
		HPEN hpen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
		HPEN oldpen = (HPEN)SelectObject(hdc, hpen);

		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

		//RECT* rt = &shapenum->rt;
		switch (type) {
		case DrawType_Line:

			MoveToEx(hdc, rt->left, rt->top, 0);
			LineTo(hdc, rt->right, rt->bottom);
			break;
		case DrawType_Circle:
			Ellipse(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Square:
			Rectangle(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		}
		DeleteObject(SelectObject(hdc, oldpen));
		SelectObject(hdc, oldbrush);
		ReleaseDC(api_hwnd, hdc);
	}
	void DrawTracker(HDC hdc) {
		RECT rt = shapenum->rt;
		if (shapenum->type != DrawType_Line) {
			DrawOneTrack(hdc, rt.left, rt.top);
			DrawOneTrack(hdc, (rt.left + rt.right) / 2, rt.top);
			DrawOneTrack(hdc, rt.right, rt.top);

			DrawOneTrack(hdc, rt.left, (rt.top + rt.bottom) / 2);
			DrawOneTrack(hdc, rt.right, (rt.top + rt.bottom) / 2);

			DrawOneTrack(hdc, rt.left, rt.bottom);
			DrawOneTrack(hdc, (rt.left + rt.right) / 2, rt.bottom);
			DrawOneTrack(hdc, rt.right, rt.bottom);
		}
		else {
			DrawOneTrack(hdc, rt.left, rt.top);
			DrawOneTrack(hdc, rt.right, rt.bottom);
		}
	}
	WCHAR* TrackCursor[10] = { 0,
		IDC_SIZENWSE,IDC_SIZENS,IDC_SIZENESW,IDC_SIZEWE,
		IDC_SIZEWE,IDC_SIZENESW,IDC_SIZENS,IDC_SIZENWSE };
public:
	ShapeList* shapelist;
	USHORT tracknum;
	HWND api_hwnd;

	ShapeList::ShapeData* shapenum;
	ShapeList::ShapeData shapebuf;
	USHORT isDrag;
	
	USHORT drawtype;
	APIClass() {
		shapenum = 0;
		shapelist = new ShapeList();
		isDrag = 0;
		drawtype = DrawType_None;
	}
	void Create(HWND hwnd) {
		api_hwnd = hwnd;
	}
	void Paint(HWND hwnd);
	void Command(USHORT, WPARAM);
	void MouseMove(USHORT, USHORT);
	void LButtonDown(USHORT, USHORT);
	bool OnSetCursor() {
		POINT pt;
		int hit;
		if (drawtype == DrawType_None && shapenum) {
			hit = tracknum;
			if (isDrag != Drag_Size) {
				GetCursorPos(&pt);
				ScreenToClient(api_hwnd, &pt);
				hit = TrackerHit(pt.x, pt.y);
			}
			if (hit) {
				SetWindowText(api_hwnd, L"sizing");
				SetCursor(LoadCursor(0, TrackCursor[hit]));
				return true;
			}
		}
		return false;
	}
	void LButtonUp();
	void RButtonUp(USHORT, USHORT);
	void Destroy() { PostQuitMessage(0); }
	~APIClass() { delete shapelist; }
};
void APIClass::LButtonDown(USHORT x, USHORT y) {
	ShapeList::ShapeData* FindData;
	if (drawtype == DrawType_None) {
		if (shapenum) {
			tracknum=TrackerHit(x, y);
			if (tracknum) {
				shapebuf = *shapenum;
				isDrag = Drag_Size;
				return;
			}
		}
		FindData = shapelist->find(x, y);
		if (FindData != shapenum) {
			shapenum = FindData;
			InvalidateRect(api_hwnd, 0, true);
			UpdateWindow(api_hwnd);
			{
				wchar_t str[128];
				wsprintf(str, L"%#X", shapenum);
				SetWindowText(api_hwnd, str);
			}
		}
		if (FindData) {
			oldx = x;
			oldy = y;
			DrawTemp(shapenum->type,&shapenum->rt);
			isDrag = 2;
		}
	}
	else {
		//shapenum = 0;
		mx = x; my = y;
		oldx = x; oldy = y;
		isDrag = 1;
	}
	//SetCapture(api_hwnd);
}
void APIClass::MouseMove(USHORT x, USHORT y) {
	USHORT ex = x, ey = y;
	HDC hdc;
	RECT* rt = &shapenum->rt;
	if (isDrag == 1) {
		{
			RECT oldrt;
			SetRect(&oldrt, mx, my, oldx, oldy);
			DrawTemp(drawtype, &oldrt);
			oldrt.right = ex; oldrt.bottom = ey;
			DrawTemp(drawtype, &oldrt);
		}
		oldx = ex; oldy = ey;
	}
	else if (isDrag == 2) {
		DrawTemp(shapenum->type, rt);
		OffsetRect(rt, ex - oldx, ey - oldy);
		oldx = ex;
		oldy = ey;
		DrawTemp(shapenum->type, rt);
	}
	else if (isDrag == Drag_Size) {
		{
			DrawTemp(shapenum->type, rt);
			switch (tracknum) {
			case 1:rt->left = x; rt->top = y; break;
			case 2:rt->top = y; break;
			case 3:rt->right = x; rt->top = y; break;

			case 4:rt->left = x;  break;
			case 5:rt->right = x;break;

			case 6:rt->left = x; rt->bottom = y; break;
			case 7:rt->bottom = y; break;
			case 8:rt->right = x; rt->bottom = y; break;
			}
			DrawTemp(shapenum->type, rt);
		}
	}
}
int getArea(int x, int y,UCHAR sz=100) {
	x = x<0?-x:x;
	y = y < 0 ? -y : y;
	return x * y > sz;
}
void APIClass::LButtonUp() {
	switch (isDrag) {
	case Drag_Draw:
		RECT rt; isDrag = 0;
		if (getArea((mx - oldx),(my - oldy))) {
			SetRect(&rt, mx, my, oldx, oldy);
			shapelist->append(drawtype, &rt);
			shapenum = shapelist->data + (shapelist->len - 1);
		}
		InvalidateRect(api_hwnd, 0, true);
		break;
	case Drag_Move:
		//SetRect(&shapenum->rt,mx, my;
		InvalidateRect(api_hwnd, 0, true);
		break;
	case Drag_Size:
		tracknum = 0; {
			RECT *srt=&shapenum->rt;
			if (getArea((srt->left-srt->right),(srt->top - srt->bottom)))
				shapelist->AdjustRect(shapenum->type, &shapenum->rt);
			else memcpy(&shapenum->rt, &shapebuf.rt, sizeof(RECT));
			
		}
		InvalidateRect(api_hwnd, 0, true);
		break;
	}
	isDrag = 0;
}
void APIClass::Paint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	RECT* rt;
	for (auto d = shapelist->data; d != shapelist->data + shapelist->len; d++) {
		rt = &d->rt;
		switch (d->type) {
		case DrawType_Circle:
			Ellipse(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Square:
			Rectangle(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Line:
			MoveToEx(hdc, rt->left, rt->top, 0);
			LineTo(hdc, rt->right, rt->bottom);
			break;
		}
	}
	if (shapenum)DrawTracker(hdc);
	//
	EndPaint(hwnd, &ps);
}
void APIClass::Command(USHORT type, WPARAM wp) {
	switch (type) {
	case DrawType_None:
		drawtype = type;
		isDrag = 0;
		CheckMenuItem((HMENU)wp, DrawType_None, MF_BYCOMMAND | MF_CHECKED);
		break;
	case DrawType_Circle:
	case DrawType_Square:
	case DrawType_Line:
		drawtype = type;
		break;
	case Menu_Delete:
		if (isDrag==0&&shapenum) {
			shapelist->delete_shape(shapenum);
			shapenum = 0;
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_Clear:
		if (isDrag == 0) {
			shapelist->clear();
			shapenum = 0;
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_Exit:
		DestroyWindow(api_hwnd);
		break;
	}
}
void APIClass::RButtonUp(USHORT x, USHORT y) {
	HMENU hpop = CreatePopupMenu();
	POINT point = { x ,y };
	if (isDrag)SendMessage(api_hwnd, WM_LBUTTONUP, 0, 0);
	ClientToScreen(api_hwnd, &point);
	AppendMenu(hpop, MF_STRING, DrawType_None, L"선택");
	AppendMenu(hpop, MF_STRING, DrawType_Circle, L"원");
	AppendMenu(hpop, MF_STRING, DrawType_Square, L"사각형");
	AppendMenu(hpop, MF_STRING, DrawType_Line, L"직선");

	AppendMenu(hpop, MF_STRING, Menu_Delete, L"선택 도형 삭제\tDel");
	AppendMenu(hpop, MF_STRING, Menu_Clear, L"전체 삭제\tCtrl+Del");
	AppendMenu(hpop, MF_SEPARATOR, 0, 0);
	AppendMenu(hpop, MF_STRING, Menu_Exit, L"QUIT");
	CheckMenuItem(hpop, drawtype, MF_BYCOMMAND | MF_CHECKED);
	TrackPopupMenu(hpop, TPM_RIGHTBUTTON, point.x, point.y,
		0, api_hwnd, 0);
	DestroyMenu(hpop);
}
APIClass* api;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	static HCURSOR hcursor;
	/*
	switch (msg) {

	case WM_CREATE:
	{
		TCITEM tie;
		InitCommonControls();
		hTab = CreateWindow(WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE

			| WS_CLIPSIBLINGS,

			0, 0, 0, 0, hwnd, (HMENU)10, g_hin, NULL);

		hStatic = CreateWindow(L"static", L"뭔 짓을 했길래 안 되는거야", WS_CHILD | WS_VISIBLE,

			0, 0, 0, 0, hwnd, (HMENU)9, g_hin, NULL);

		tie.mask = TCIF_TEXT;
		tie.pszText = (WCHAR*)L"GUU";
		TabCtrl_InsertItem(hTab, 0, &tie);
		tie.pszText = (WCHAR*)L"MUU";
		TabCtrl_InsertItem(hTab, 1, &tie);
	}
	return 0;
	case WM_SIZE:

		MoveWindow(hTab, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);

		MoveWindow(hStatic, LOWORD(lp) / 2 - 250, HIWORD(lp) / 2, 500, 25, TRUE);
		GetClientRect(hwnd, &rt);
		InvalidateRect(hwnd, 0, true);
		return 0;
	case WM_NOTIFY:

		switch (((LPNMHDR)lp)->code) {

		case TCN_SELCHANGE:

			SetWindowText(hStatic, L"SHIT THE FUCK");

			break;

		}

		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
	}
		return 0;*/
	switch (msg) {
	case WM_CREATE:api->Create(hwnd); return 0;
	case WM_COMMAND:api->Command(LOWORD(wp), wp); return 0;
	case WM_PAINT:api->Paint(hwnd); return 0;
	case WM_MOUSEMOVE:api->MouseMove(LOWORD(lp), HIWORD(lp)); return 0;
		return 0;
	case WM_LBUTTONDOWN:hcursor = LoadCursor(0, IDC_HAND);
		api->LButtonDown(LOWORD(lp), HIWORD(lp)); return 0;
	case WM_LBUTTONUP:
		hcursor = LoadCursor(0, IDC_ARROW);
		api->LButtonUp(); return 0;
	case WM_RBUTTONUP:api->RButtonUp(LOWORD(lp), HIWORD(lp));
		return 0;
	case WM_SETCURSOR:
		if (!api->OnSetCursor())return(DefWindowProc(hwnd, WM_SETCURSOR, wp, lp));
		return 0;

	case WM_DESTROY:
		api->Destroy();
		return 0;
	}
	return (DefWindowProc(hwnd, msg, wp, lp));
}
int WINAPI WinMain(HINSTANCE hin, HINSTANCE, LPSTR, int) {
	HWND hwnd;
	WNDCLASS win = { 0 };
	api = new APIClass();
	win.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	//CreateHatchBrush(HS_DIAGCROSS,RGB(0, 150, 123));
	win.hIcon = LoadIcon(0, IDI_APPLICATION);
	win.hCursor = LoadCursor(0, IDC_ARROW);
	win.hInstance = hin;
	win.lpfnWndProc = WndProc;
	win.lpszClassName = L"unjy";
	win.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&win);

	hwnd = CreateWindow(L"unjy", L"unjy", WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, (HMENU)0, hin, 0);
	ShowWindow(hwnd, 1);
	hin;
	MSG msg;
#ifdef RESOURCE
	HACCEL hAccel = LoadAccelerators(
		hin, MAKEINTRESOURCE(IDR_ACCELERATOR1)
	);
#endif
	while (GetMessage(&msg, 0, 0, 0)) {
#ifdef RESOURCE
		if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
#endif
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#ifdef RESOURCE
	}
#endif
	delete api;
	return 0;
}
