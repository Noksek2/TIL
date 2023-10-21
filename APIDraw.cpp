#include <Windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include "resource.h"
using namespace Gdiplus;
#pragma comment(lib,"gdiplus")
#pragma comment(lib,"comctl32.lib")
HWND hRed, hGreen, hBlue;
HINSTANCE g_hin;
//#define DIALOG
#ifdef DIALOG
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

HINSTANCE g_hInst;
inline int LP_GetMouseX(LPARAM lp){ return LOWORD(lp); }
inline int LP_GetMouseY(LPARAM lp) { return HIWORD(lp); }
struct ShapeList{
	struct ShapeData {
		RECT rt;
		USHORT type;
	};
	ShapeData *data;
	UINT capa,len;
	ShapeList() {
		capa = 10; len = 0;
		data = (ShapeData*)malloc(sizeof(ShapeData)*capa);
	}
	void append(USHORT type,RECT *mrt) {
		if (len >= capa)return;
		data[len].type = type;
		memcpy(&data[len++].rt, mrt, sizeof(RECT));
	}
	void delete_shape(ShapeData* d) {
		if (len>0&&d<data+len)
			*d= data[--len];
	}
	void clear() {
		len = 0;
	}
	bool AdjustPointInRect(RECT *r,POINT pt) {
		RECT rr;
		long t;
		if (r->left > r->right) {
			rr.left = r->right;
			rr.right = r->left;
		}
		else{
			rr.left = r->left;
			rr.right = r->right;
		}
		if (r->top > r->bottom) {
			rr.top = r->bottom;
			rr.bottom = r->top;
		}
		else{
			rr.top = r->top;
			rr.bottom = r->bottom;
		}
		if (PtInRect(&rr, pt))return true;
		return false;
	}
	ShapeData* find(USHORT x,USHORT y) {
		POINT pt = {x,y};
		for (auto d = data+(len-1); d>=data; d--) {
			{
				if (AdjustPointInRect(&d->rt, pt)) return d;
			}
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
public:
	ShapeList* shapelist;
	HWND api_hwnd;
	USHORT mx, my, oldx, oldy;
	ShapeList::ShapeData* shapenum;
	bool isDrag;
	enum { DrawType_None,DrawType_Circle, DrawType_Square, DrawType_Line,
		Menu_Delete,Menu_Clear,Menu_Exit };
	USHORT drawtype;
	APIClass() {
		shapenum = 0;
		shapelist = new ShapeList();
		isDrag = false;
		drawtype = DrawType_None;
	}
	void Create(HWND hwnd) { api_hwnd = hwnd; }
	void Paint(HWND hwnd);
	void Command(USHORT,WPARAM);
	void MouseMove(USHORT, USHORT);
	void LButtonDown(USHORT,USHORT);
	void LButtonUp() { 
		if (isDrag) {
			isDrag = false; //ReleaseCapture();
			RECT rt;
			if (mx == oldx && my == oldy)return;
			SetRect(&rt, mx, my, oldx, oldy);
			shapelist->append(drawtype, &rt);
			shapenum = shapelist->data+(shapelist->len - 1);
			InvalidateRect(api_hwnd, 0, true);
		}
	}
	void RButtonUp(USHORT, USHORT);
	void Destroy(){ PostQuitMessage(0); }
	~APIClass() { delete shapelist; }

	inline void DrawOneTrack(HDC hdc,USHORT x, USHORT y){
		Rectangle(hdc, x - 5, y - 5, x + 5, y + 5);
	}
	void DrawTracker(HDC hdc) {
		RECT rt= shapenum->rt;
		
		
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
};
void APIClass::LButtonDown(USHORT x, USHORT y){
	ShapeList::ShapeData* FindData;
	if (drawtype == DrawType_None) {
		FindData=shapelist->find(x, y);
		if (FindData != shapenum)   {
			shapenum = FindData;
			InvalidateRect(api_hwnd, 0, true);
			UpdateWindow(api_hwnd);
			{
				wchar_t str[128];
				wsprintf(str, L"%#X", shapenum);
				SetWindowText(api_hwnd, str);
			}
		}
	}
	else {
		//shapenum = 0;
		mx = x; my = y;
		oldx = x; oldy = y;
		isDrag = true;
	}
	//SetCapture(api_hwnd);
}
void APIClass::MouseMove(USHORT x, USHORT y) {
	USHORT ex = x,ey=y;
	HDC hdc;
	if (isDrag) {
		hdc = GetDC(api_hwnd);
		SetROP2(hdc, R2_NOTXORPEN);
		switch (drawtype) {
		case DrawType_Line:
			MoveToEx(hdc, mx, my, 0);
			LineTo(hdc, oldx, oldy);
			MoveToEx(hdc, mx, my, 0);
			LineTo(hdc, ex, ey);
			break;
		case DrawType_Circle:
			Ellipse(hdc,mx,my,oldx,oldy);
			Ellipse(hdc, mx, my, ex, ey);
			break;
		case DrawType_Square:
			Rectangle(hdc, mx, my, oldx, oldy);
			Rectangle(hdc, mx, my, ex, ey);
			break;
		}
		oldx = ex; oldy = ey;
		ReleaseDC(api_hwnd,hdc);
	}
}
void APIClass::Paint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd,&ps);
	RECT *rt;
	for(auto d=shapelist->data;d!=shapelist->data+shapelist->len;d++){
		rt = &d->rt;
		switch (d->type) {
		case DrawType_Circle:
			Ellipse(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Square:
			Rectangle(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Line:
			MoveToEx(hdc, rt->left, rt->top,0);
			LineTo(hdc, rt->right, rt->bottom);
			break;
		}
	}
	if (shapenum)DrawTracker(hdc);
	//
	EndPaint(hwnd, &ps);
}
void APIClass::Command(USHORT type,WPARAM wp) {
	switch (type) {
	case DrawType_None:
		drawtype = type;
		isDrag = false;
		CheckMenuItem((HMENU)wp, DrawType_None, MF_BYCOMMAND | MF_CHECKED);
		break;
	case DrawType_Circle:
	case DrawType_Square:
	case DrawType_Line:
		drawtype = type;
		break;
	case Menu_Delete:
		if (shapenum) {
			shapelist->delete_shape(shapenum);
			shapenum = 0;
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_Clear:
		shapelist->clear();
		shapenum = 0;
		InvalidateRect(api_hwnd, 0, true);
		break;
	case Menu_Exit:
		DestroyWindow(api_hwnd);
		break;
	}
}
void APIClass::RButtonUp(USHORT x, USHORT y) {
	HMENU hpop = CreatePopupMenu();
	POINT point = { x ,y};
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
	switch(msg){
	case WM_CREATE:api->Create(hwnd); return 0;
	case WM_COMMAND:api->Command(LOWORD(wp),wp); return 0;
	case WM_PAINT:api->Paint(hwnd); return 0;
	case WM_MOUSEMOVE:api->MouseMove(LOWORD(lp), HIWORD(lp)); return 0;
		return 0;
	case WM_LBUTTONDOWN:
		api->LButtonDown(LOWORD(lp),HIWORD(lp)); return 0;
	case WM_LBUTTONUP:
		api->LButtonUp(); return 0;
	case WM_RBUTTONUP:api->RButtonUp(LOWORD(lp), HIWORD(lp));
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
	g_hInst=g_hin = hin;
	MSG msg;
	HACCEL hAccel=LoadAccelerators(
		g_hin,MAKEINTRESOURCE(IDR_ACCELERATOR1)
	);

	while (GetMessage(&msg, 0, 0, 0)) {
		if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	delete api;
	return 0;
}
