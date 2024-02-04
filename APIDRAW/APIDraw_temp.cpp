
#include "winclass.h"
#include <gdiplus.h>
#include <commctrl.h>
#include <crtdbg.h>
#pragma comment(lib,"gdiplus")
using namespace Gdiplus;
#pragma comment( \
    linker, \
    "/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls' \
    version='6.0.0.0' \
    processorArchitecture='*' \
    publicKeyToken='6595b64144ccf1df' \
    language='*'\"")
#ifdef RESOURCE
#include "resource.h"
#endif
enum {

	DrawType_None, DrawType_Circle, DrawType_Square, DrawType_Line,
	Menu_Delete, Menu_Clear, Menu_Prop,Menu_ImgOption,Menu_TextOption,
	Menu_Forward, Menu_FarForward, Menu_Backward, Menu_FarBack,
	Menu_Exit, ID_Scroll, DrawType_Polygon, 
	DrawType_Bitmap,DrawType_Meta, DrawType_Text,
};
enum {
	DID_ScrollLine = 4, DID_ComboLine, DID_BtnSet,
	DID_ScrollAlpha, DID_BtnPlane2, DID_RadioNoBrush, DID_RadioPlane, DID_RadioHatch, DID_RadioGrad,
	DID_ComboGrad, DID_BtnPaste,
};
enum {
	Drag_None = 0, Drag_Draw = 1, Drag_Move, Drag_Size
};
enum {
	Brush_None = 0, Brush_Plane, Brush_Hatch, Brush_Grad
};
inline int LP_GetMouseX(LPARAM lp) { return LOWORD(lp); }
inline int LP_GetMouseY(LPARAM lp) { return HIWORD(lp); }

LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ImgDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
struct ShapeText {
	WCHAR fontface[32];
	WCHAR* text;
	USHORT fontsize, len;
};
struct ShapeList {
	struct ShapeData {
		RECT rt;
		union {
			POINT* point;
			ShapeText* t;
			BYTE* image;
		};
		USHORT type;
		COLORREF linecolor;
		COLORREF planecolor;
		COLORREF planecolor2;
		UCHAR alpha;
		UCHAR thick;
		UCHAR point_len, linetype, brushtype, brushmode;
		ShapeData() {
			brushmode = Brush_Plane;
			alpha = 255;
			point_len = linetype = 0;
			brushtype = 0;
			planecolor = RGB(50, 200, 75);
			planecolor2 = RGB(255, 255, 255);
			linecolor = RGB(0, 0, 0);
			thick = 1;
			type = 0;
			point = 0;
			rt = { 0 };
		}
	};
	ShapeData* data;
	ShapeData tempshape;


	UINT capa, len;
	ShapeList() {
		capa = 20; len = 0; tempshape.thick = 1;
		data = (ShapeData*)malloc(sizeof(ShapeData) * capa);
	}
	void append(USHORT type, RECT* mrt,BYTE* buf=0,DWORD size=0) {
		if (len >= capa)return;
		data[len] = tempshape;
		data[len].type = type;
		if (type != DrawType_Line)AdjustRect(mrt);
		if (type == DrawType_Bitmap|| type == DrawType_Meta) {
			data[len].image = buf;
			data[len].point_len = size;
		}
		memcpy(&data[len].rt, mrt, sizeof(RECT));


		wchar_t str[128];
		wsprintf(str, L"append shape[%d] type:%d \n", len, type);
		OutputDebugString(str);
		len++;
	}
	void setPolyOffset(ShapeData* sh, long x, long y) {
		for (auto d = sh->point; d < sh->point + sh->point_len; d++) {
			d->x += x;
			d->y += y;
		}
	}
	void append_polygon(POINT* _point, UCHAR point_len) {
		if (len >= capa)return;
		data[len] = tempshape;
		RECT mrt;
		SetRect(&mrt, _point->x, _point->y, _point->x, _point->y);
		for (auto d = _point; d < _point + point_len; d++) {
			if (mrt.left > d->x)mrt.left = d->x;
			else if (mrt.right < d->x)mrt.right = d->x;
			if (mrt.top > d->y)mrt.top = d->y;
			else if (mrt.bottom < d->y)mrt.bottom = d->y;

		}
		data[len].rt = mrt;
		data[len].type = DrawType_Polygon;

		data[len].point = (POINT*)malloc(sizeof(POINT) * point_len);
		data[len].point_len = point_len;
		memcpy(data[len].point, _point, sizeof(POINT) * point_len);

		wchar_t str[128];
		wsprintf(str, L"append shape[%d] type:%d \n", len, DrawType_Polygon);
		OutputDebugString(str);
		len++;
	}
	ShapeData* forward_shape(ShapeData* d, bool isfar = false) {
		ShapeData tmp = *d;
		if (isfar) {
			for (ShapeData* p = d + 1; p < data + len; p++) {
				*(p - 1) = *p;
			}
			*(data + len - 1) = tmp;
			return (data + len - 1);
		}
		else if (d + 1 < data + len) {
			*d = *(d + 1);
			*(d + 1) = tmp;
			return d + 1;
		}
		return d;
	}
	ShapeData* backward_shape(ShapeData* d, bool isfar = false) {
		ShapeData tmp = *d;
		if (isfar) {
			for (ShapeData* p = d - 1; p >= data; p--) {
				*(p + 1) = *p;
			}
			*(data) = tmp;
			return (data);
		}
		else if (d - 1 >= data) {
			*d = *(d - 1);
			*(d - 1) = tmp;
			return d - 1;
		}
		return d;
	}
	void delete_shape(ShapeData* d) {
		if (d->type == DrawType_Polygon && d->point) {
			free(d->point);
			d->point = 0;
		}
		else if (d->type >= DrawType_Bitmap) {
			if (d->image) { free(d->image); d->image = 0; }
		}
		for (ShapeData* p = d + 1; p < data + len; p++) {
			*(p - 1) = *p;
		}
		len--;


		/*if (len > 0 && d < data + len)
			*d = data[--len];*/
	}
	void clear() {
		for (ShapeData* p = data; p < data + len; p++) {
			if (p->type == DrawType_Polygon && p->point) {
				free(p->point);
				p->point = 0;
			}
			else if (p->type >= DrawType_Bitmap)
				if (p->image) { free(p->image); p->image = 0; }
		}
		len = 0;
	}
	void AdjustRect(RECT* r) {
		long t;
		if (r->left > r->right) {
			t = r->right;
			r->right = r->left;
			r->left = t;
		}
		if (r->top > r->bottom) {
			t = r->bottom;
			r->bottom = r->top;
			r->top = t;
		}
	}
	ShapeData* find(long x, long y) {
		POINT pt = { x,y };
		RECT temprt;

		for (auto d = data + len - 1; d >= data; d--) {
			{
				if (d->type == DrawType_Line) {
					SetRect(&temprt, d->rt.left, d->rt.top, d->rt.right, d->rt.bottom);
					AdjustRect(&temprt);
					if (PtInRect(&temprt, pt)) return d;
				}
				else if (d->type == DrawType_Circle) {
					{
						HRGN hcircle = CreateEllipticRgn(
							d->rt.left, d->rt.top,
							d->rt.right, d->rt.bottom
						);
						bool iscircle = PtInRegion(hcircle, x, y);
						DeleteObject(hcircle);
						if (iscircle)return d;
					}
				}
				else if (d->type == DrawType_Polygon)
				{
					{
						HRGN hpoly = CreatePolygonRgn(
							d->point, d->point_len, WINDING
						);
						bool ispoly = PtInRegion(hpoly, x, y);
						DeleteObject(hpoly);
						if (ispoly)return d;
					}
					//ALTERNATE
				}
				else {
					AdjustRect(&d->rt); if (PtInRect(&d->rt, pt)) return d;

				}
			}
		}
		return 0;
	}
	~ShapeList() {
		clear();
		if (data) {
			free(data); data = 0;
		}
	}
};
CScroll dlg_scroll, dlg_scroll_line, dlg_scroll_alpha;
//CCheckBox dlg_check;
CComboBox dlg_combo, dlg_combo_grad;
CRadioButton dlg_radio[4];
HWND hwnd_plane2, hwnd_plane, hwnd_static[3];
class APIClass {
private:
	long mx, my, oldx, oldy;
	CHOOSECOLOR col;
	COLORREF coltemp[16];
	POINT point[10];
	HBITMAP hBuffer;
	UCHAR point_idx;

	bool pointmode = false;
	bool isctrl = false, isshift = false;
	inline void DrawOneTrack(HDC hdc, long x, long y) {
		Rectangle(hdc, x - 4, y - 4, x + 4, y + 4);
	}
	inline void DrawPointTrack(HDC hdc, long x, long y) {
		Ellipse(hdc, x - 4, y - 4, x + 4, y + 4);
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
	void AdjustToGrid(long& x, long& y) {
		x = x / gridw * gridw;
		y = y / gridw * gridw;
	}
	int TrackerHit(long x, long y) {
		POINT pt;
		pt.x = x;
		pt.y = y;
		if (!shapenum)return 0;
		RECT trackrt;
		/*if (shapenum->type == DrawType_Polygon) {
			{
				long tx, ty;
				for (int i = 0; i < shapenum->point_len; i++) {
					tx = shapenum->point[i].x;
					ty = shapenum->point[i].y;
					SetRect(&trackrt, tx - 4, ty - 4, tx + 4, ty + 4);
					if (PtInRect(&trackrt, pt))return i;
				}
			}
			return 0;
		}
		else */if (shapenum->type == DrawType_Line)
		{
			GetTracker(1, &trackrt);
			if (PtInRect(&trackrt, pt))return 1;
			GetTracker(8, &trackrt);
			if (PtInRect(&trackrt, pt))return 8;
			return 0;
		}
		for (int i = 1; i <= 8; i++) {
			GetTracker(i, &trackrt);
			if (PtInRect(&trackrt, pt))return i;
		}
		return 0;
	}
	void DrawTemp() {
		HDC hdc = GetDC(api_hwnd);
		SetROP2(hdc, R2_XORPEN);
		HPEN hpen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
		HPEN oldpen = (HPEN)SelectObject(hdc, hpen);

		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

		RECT* rt = &shapenum->rt;
		switch (shapenum->type) {
		case DrawType_Line:

			MoveToEx(hdc, rt->left, rt->top, 0);
			LineTo(hdc, rt->right, rt->bottom);
			break;
		case DrawType_Circle:
			Ellipse(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Square:case DrawType_Bitmap:case DrawType_Meta:case DrawType_Text:
			Rectangle(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Polygon:
			Polygon(hdc, shapenum->point, shapenum->point_len);
			break;
		}
		DeleteObject(SelectObject(hdc, oldpen));
		SelectObject(hdc, oldbrush);
		ReleaseDC(api_hwnd, hdc);
	}
	void DrawTracker(HDC hdc) {
		RECT rt = shapenum->rt;

		if (shapenum->type == DrawType_Polygon) {
			for (int i = 0; i < shapenum->point_len; i++)
				DrawPointTrack(hdc, shapenum->point[i].x, shapenum->point[i].y);
		}
		if (shapenum->type == DrawType_Line) {


			DrawOneTrack(hdc, rt.left, rt.top);
			DrawOneTrack(hdc, rt.right, rt.bottom);
		}
		else {

			DrawOneTrack(hdc, rt.left, rt.top);
			DrawOneTrack(hdc, (rt.left + rt.right) / 2, rt.top);
			DrawOneTrack(hdc, rt.right, rt.top);

			DrawOneTrack(hdc, rt.left, (rt.top + rt.bottom) / 2);
			DrawOneTrack(hdc, rt.right, (rt.top + rt.bottom) / 2);

			DrawOneTrack(hdc, rt.left, rt.bottom);
			DrawOneTrack(hdc, (rt.left + rt.right) / 2, rt.bottom);
			DrawOneTrack(hdc, rt.right, rt.bottom);
		}
	}
	WCHAR* TrackCursor[10] = { 0,
		IDC_SIZENWSE,IDC_SIZENS,IDC_SIZENESW,IDC_SIZEWE,
		IDC_SIZEWE,IDC_SIZENESW,IDC_SIZENS,IDC_SIZENWSE };
public:
	ShapeList* shapelist;
	HWND api_hwnd, dialog_hwnd,dlg2_hwnd;
	HINSTANCE g_hin;
	ShapeList::ShapeData* shapenum;
	USHORT isDrag;
	UCHAR tracknum;
	USHORT drawtype;

	int gridw;
	bool isgrid;


	ShapeList::ShapeData* GetTempShape() {
		return &shapelist->tempshape;
	}
	APIClass(HINSTANCE hin) :g_hin(hin) {
		dlg2_hwnd = 0;
		gridw = 10;
		isgrid = true;

		point_idx = 0;
		shapenum = 0;
		shapelist = new ShapeList();
		isDrag = 0;

		memset(&col, 0, sizeof(CHOOSECOLOR));
		col.lStructSize = sizeof(CHOOSECOLOR);
		col.lpCustColors = coltemp;
		drawtype = DrawType_None;
	}
	void Create(HWND hwnd) {
		hBuffer = 0;
		api_hwnd = hwnd;
		for (int i = 0; i < 16; i++)
			coltemp[i] = RGB(rand(), rand(), rand());
	}
	void KillFocus() { isctrl = isshift = false; }
	void KeyDown(WPARAM);
	void KeyUp(WPARAM);
	void Paint(HWND hwnd);
	void Command(USHORT, WPARAM);
	void MouseMove(long, long);
	void LButtonDown(long, long);
	bool OnSetCursor() {
		POINT pt;
		int hit = tracknum;
		if (isDrag == Drag_Size) {
			SetCursor(LoadCursor(0, TrackCursor[hit]));
			return true;
		}

		else if (isDrag == Drag_Draw) {
			SetCursor(LoadCursor(0, IDC_CROSS));
			return true;
		}
		else if (isDrag == Drag_Move) {
			SetCursor(LoadCursor(0, IDC_SIZEALL));
			return true;
		}
		else if (drawtype == DrawType_None && shapenum) {
			GetCursorPos(&pt);
			ScreenToClient(api_hwnd, &pt);
			hit = TrackerHit(pt.x, pt.y);
			if (hit) {
				SetWindowText(api_hwnd, L"선택");
				SetCursor(LoadCursor(0, TrackCursor[hit]));
				return true;
			}
		}
		return false;
	}
	void LButtonUp();
	void OnSize(WPARAM wp) {
		if (wp != SIZE_MINIMIZED) {
			if (hBuffer) {
				DeleteObject(hBuffer);
				hBuffer = 0;
			}

		}
	}
	void RButtonUp(long, long);
	void Destroy() {
		if (hBuffer) {
			DeleteObject(hBuffer);
			hBuffer = 0;
		}
		PostQuitMessage(0);
	}
	void InsertBitmap(long, long);
	void InsertMeta(long, long);
	void Set_ShapeColor(bool isline = false) {
		if (ChooseColor(&col)) {
			if (shapenum) {
				if (!isline)shapenum->planecolor = col.rgbResult;
				else shapenum->linecolor = col.rgbResult;
				InvalidateRect(api_hwnd, 0, true);
			}
			else {
				if (!isline)GetTempShape()->planecolor = col.rgbResult;
				else GetTempShape()->linecolor = col.rgbResult;
			}
			InvalidateRect(dialog_hwnd, 0, false);
		}
	}
	void Set_PlaneColor2() {
		if (ChooseColor(&col)) {
			if (shapenum) {
				shapenum->planecolor2 = col.rgbResult;
				InvalidateRect(api_hwnd, 0, true);
			}
			else {
				GetTempShape()->planecolor2 = col.rgbResult;

			}
			InvalidateRect(dialog_hwnd, 0, false);
		}
	}
	void Set_RadioPlaneEnable(bool flag) {
		if (!flag) {
			dlg_radio[0].Set(true);
			dlg_radio[1].Set(false);
			dlg_radio[2].Set(false);
			dlg_radio[3].Set(false);

			EnableWindow(dlg_radio[1].hcheck, false);
			EnableWindow(dlg_radio[2].hcheck, false);
			EnableWindow(dlg_radio[3].hcheck, false);
			EnableWindow(hwnd_plane, false);
		}
		else {
			EnableWindow(dlg_radio[1].hcheck, true);
			EnableWindow(dlg_radio[2].hcheck, true);
			EnableWindow(dlg_radio[3].hcheck, true);
			EnableWindow(hwnd_plane, true);
		}
	}
	void Set_DialogEnable() {
		EnableWindow(hwnd_plane, !dlg_radio[0].ischeck);
		EnableWindow(dlg_combo.hcombo, dlg_radio[2].ischeck);
		EnableWindow(dlg_combo_grad.hcombo, dlg_radio[3].ischeck);
		EnableWindow(hwnd_plane2, dlg_radio[2].ischeck || dlg_radio[3].ischeck);
		InvalidateRect(dialog_hwnd, 0, false);
	}
	void Set_RadioBrush(UCHAR n) {
		dlg_radio[0].Process();
		dlg_radio[1].Process();
		dlg_radio[2].Process();
		dlg_radio[3].Process();
		if (shapenum) {
			shapenum->brushmode = n;
			InvalidateRect(api_hwnd, 0, true);
		}
		else GetTempShape()->brushmode = n;
		Set_DialogEnable();
	}
	/*UCHAR IndextoHatch(UCHAR i) {
		switch (i) {
		case 0:return 12;
		case 1:return HS_BDIAGONAL;
		case 2:return HS_CROSS;
		case 3:return HS_DIAGCROSS;
		case 4:return HS_FDIAGONAL;
		case 5:return HS_HORIZONTAL;
		case 6:return HS_VERTICAL;
		}
		return 12;
	};*/
	void Set_BrushType(UCHAR i) {
		if (shapenum) {
			shapenum->brushtype = i;
			InvalidateRect(api_hwnd, 0, true);
		}
		else {
			GetTempShape()->brushtype = i;
		}
		Set_DialogEnable();
	}
	//속성창 스태틱 컨트롤 수치
	void Set_StaticText(ShapeList::ShapeData* shape) {
		wchar_t str[128];
		wsprintf(str, L"%d", shape->thick);
		SetWindowText(hwnd_static[0], str);
		wsprintf(str, L"%d", shape->linetype);
		SetWindowText(hwnd_static[1], str);
		wsprintf(str, L"%d", shape->alpha);
		SetWindowText(hwnd_static[2], str);

	}
	void SetDialogControl() {//속성 창 컨트롤 초기화
		if (!IsWindow(dialog_hwnd))return;
		auto shape = shapenum ? shapenum : GetTempShape();
		dlg_radio[0].Set(false);
		dlg_radio[1].Set(false);
		dlg_radio[2].Set(false);
		dlg_radio[3].Set(false);

		dlg_scroll.Set(shape->thick);
		dlg_scroll_line.Set(shape->linetype);
		dlg_combo.SetIndex(shape->brushtype);
		dlg_combo_grad.SetIndex(shape->brushtype);
		dlg_scroll_alpha.Set(shape->alpha);
		dlg_radio[shape->brushmode].Set(true);
		wchar_t str[128];

		Set_StaticText(shape);

		if (shapenum) {
			wsprintf(str, L"도형 속성 - 선택 %d", shapenum - shapelist->data);
			SetWindowText(dialog_hwnd, str);
			Set_RadioPlaneEnable(shapenum->type != DrawType_Line);
		}

		else {
			SetWindowText(dialog_hwnd, L"도형 속성");
			Set_RadioPlaneEnable(drawtype != DrawType_Line);
		}
		Set_DialogEnable();
	}
	inline ShapeList::ShapeData* GetNowShape() {
		return (shapenum ? shapenum : GetTempShape());
	}
	void Set_ScrollData(CScroll& scroll, UCHAR* dt, ShapeList::ShapeData* shape, WPARAM wp, LPARAM lp) {
		if ((HWND)lp == scroll.hscroll) {
			scroll.Process(wp, lp);
			*dt = (UCHAR)scroll.pos;
			Set_StaticText(shape);

			InvalidateRect(api_hwnd, 0, true);
		}
	}
	~APIClass() { delete shapelist; }

};
void APIClass::KeyDown(WPARAM wp) {
	//bool isshift, isctrl;
	if (wp == VK_SHIFT)isshift = true;
	else if (wp == VK_CONTROL)isctrl = true;
	long m = isctrl ? 1 : gridw;

	bool action = false;

	RECT trt;

	if (wp == VK_DELETE) {
		SendMessage(api_hwnd, WM_COMMAND, isctrl ? Menu_Clear : Menu_Delete, 0);
		return;
	}

	else if (wp == VK_TAB) {
		shapenum =
			shapelist->len ?
			(shapenum ?
			(shapenum + 1 < shapelist->data + shapelist->len ?
				shapenum + 1 : 0)
				: shapelist->data)
			: 0;
		SetDialogControl();
		InvalidateRect(api_hwnd, 0, true);
		return;
	}
	else if (wp == VK_ESCAPE) {
		shapenum = 0;
		SetDialogControl();
		InvalidateRect(api_hwnd, 0, true);
		return;
	}
	switch (wp) {
	case '1':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_None, 0);
		return;
	case '2':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Circle, 0);
		return;
	case '3':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Square, 0);
		return;
	case '4':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Line, 0);
		return;
	case '5':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Polygon, 0);
		return;
	case '6':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Bitmap, 0);
		return;
	case '7':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Meta, 0);
		return;
	case '8':
		SendMessage(api_hwnd, WM_COMMAND, DrawType_Text, 0);
		return;
	case'P':
		if (isctrl)
			SendMessage(api_hwnd, WM_COMMAND, Menu_Prop, 0);
		return;
	case'I':
		if (isctrl)
			SendMessage(api_hwnd, WM_COMMAND, Menu_Prop, 0);
		return;
	}
	if (!shapenum)return;

	trt = shapenum->rt;
	if (!isshift) {
		if (wp == VK_LEFT) {
			OffsetRect(&trt, -m, 0); action = true;
			if(shapenum->type==DrawType_Polygon)shapelist->setPolyOffset(shapenum, -m, 0);
		}
		else if (wp == VK_RIGHT) {
			action = true; OffsetRect(&trt, +m, 0);
			if (shapenum->type == DrawType_Polygon)shapelist->setPolyOffset(shapenum, +m, 0);
		}
		if (wp == VK_UP) {
			OffsetRect(&trt, 0, -m);
			if (shapenum->type == DrawType_Polygon)shapelist->setPolyOffset(shapenum, 0, -m);
			action = true;
		}
		else if (wp == VK_DOWN) {
			OffsetRect(&trt, 0, m);
			if (shapenum->type == DrawType_Polygon)shapelist->setPolyOffset(shapenum, 0, m);
			action = true;
		}


	}
	else {
		if (wp == VK_LEFT) {
			trt.right -= 10;
			action = true;
		}
		else if (wp == VK_RIGHT) {
			action = true; trt.right += 10;
		}
		if (wp == VK_UP) {
			trt.bottom -= 10;
			action = true;
		}
		else if (wp == VK_DOWN) {
			trt.bottom += 10;
			action = true;
		}
	}
	if (action) {
		shapenum->rt = trt;
		InvalidateRect(api_hwnd, 0, true);
	}

}
void APIClass::KeyUp(WPARAM wp) {
	if (wp == VK_CONTROL)isctrl = false;
	else if (wp == VK_SHIFT)isshift = false;
}
void APIClass::LButtonDown(long x, long y) {
	long rx = x, ry = y;
	AdjustToGrid(x, y);
	ShapeList::ShapeData* FindData = 0;
	if (drawtype == DrawType_None) {
		if (shapenum) {
			tracknum = TrackerHit(rx, ry);
			if (tracknum) {
				isDrag = Drag_Size;
				oldx = x;
				oldy = y;
				goto END;
			}
		}

		FindData = shapelist->find(rx, ry);
		shapenum = FindData;

		if (FindData) {
			oldx = x;
			oldy = y;
			DrawTemp();
			isDrag = Drag_Move;
		}
		else {
			isDrag = 0;
		}
		InvalidateRect(api_hwnd, 0, true);
		//UpdateWindow(api_hwnd);
	}
	else {
		mx = x; my = y;
		shapenum = 0;
		if (drawtype == DrawType_Polygon) {
			if (point_idx) {
				mx = point[point_idx - 1].x;
				my = point[point_idx - 1].y;
			}
		}
		oldx = x; oldy = y;
		isDrag = Drag_Draw;
	}
END:;
	SetDialogControl();
	SetCapture(api_hwnd);
}
void APIClass::InsertBitmap(long x, long y) {
	OPENFILENAME ofn = { 0 };
	wchar_t filename[256] = L"";
	RECT rt;
	HANDLE hfile;
	DWORD filesize, dwread;
	BYTE* buffer;
	BITMAPINFOHEADER* ih;

	ofn.hwndOwner = api_hwnd;
	ofn.lpstrFilter = L"비트맵 파일(.bmp)\0*.bmp\0모든 파일(*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 256;
	ofn.lStructSize = sizeof(OPENFILENAME);
	if (GetOpenFileName(&ofn) != 0) {
		hfile = CreateFile(filename, GENERIC_READ, 0, 0,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hfile != INVALID_HANDLE_VALUE) {
			filesize = GetFileSize(hfile, 0);
			buffer = (BYTE*)malloc(filesize);
			ReadFile(hfile, buffer, filesize, &dwread, 0);
			CloseHandle(hfile);
			if (*buffer != 0x42 || *(buffer + 1) != 0x4d) {
				free(buffer);
				return;
			}
			ih = (BITMAPINFOHEADER*)
				(buffer+sizeof(BITMAPFILEHEADER));
			rt.left = x; rt.top = y;
			rt.right = x + ih->biWidth;
			rt.bottom = y + ih->biHeight;

			
			shapelist->append(DrawType_Bitmap,&rt,buffer,filesize);
			shapenum = shapelist->data + (shapelist->len - 1);
			InvalidateRect(api_hwnd, 0, false);
		}
	}
}
void APIClass::InsertMeta(long x, long y) {
	OPENFILENAME ofn = { 0 };
	wchar_t filename[256] = L"";
	RECT rt;
	SetRect(&rt, mx, my, oldx, oldy);


	HANDLE hfile;
	DWORD filesize, dwread;
	BYTE* buffer;
	BITMAPINFOHEADER* ih;

	ofn.hwndOwner = api_hwnd;
	ofn.lpstrFilter = L"메타 파일(.wmf)\0*.wmf\0모든 파일(*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 256;
	ofn.lStructSize = sizeof(OPENFILENAME);
	if (GetOpenFileName(&ofn) != 0) {
		hfile = CreateFile(filename, GENERIC_READ, 0, 0,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hfile != INVALID_HANDLE_VALUE) {
			filesize = GetFileSize(hfile, 0);
			buffer = (BYTE*)malloc(filesize);
			ReadFile(hfile, buffer, filesize, &dwread, 0);
			CloseHandle(hfile);
			if (*((DWORD*)buffer) != 0x9ac6cdd7) {
				free(buffer);
				return;
			}
			SetRect(&rt, x, y, x + 100, y + 100);
			
			shapelist->append(DrawType_Meta,&rt,buffer,filesize);
			shapenum = shapelist->data + (shapelist->len - 1);
			InvalidateRect(api_hwnd, 0, false);
		}
	}
}

void PlayPlaceableMeta(HDC hdc, BYTE* pMeta, int len, RECT* rt){
	HENHMETAFILE hEnh;
	PAPMHEADER pHeader = (PAPMHEADER)pMeta;
	METAFILEPICT mp;

	if (pHeader->nKey == 0x9ac6cdd7) {
		mp.mm = MM_ANISOTROPIC;
		mp.xExt = pHeader->bbox.Right - pHeader->bbox.Left;
		mp.xExt = (mp.xExt * 2540l) / (DWORD)(pHeader->inch);
		mp.yExt = pHeader->bbox.Bottom - pHeader->bbox.Top;
		mp.yExt = (mp.yExt * 2540l) / (DWORD)(pHeader->inch);
		mp.hMF = NULL;

		hEnh = SetWinMetaFileBits(len, &(pMeta[sizeof(APMHEADER)]), hdc, &mp);

		PlayEnhMetaFile(hdc, hEnh, rt);
		DeleteEnhMetaFile(hEnh);

	}

}


void APIClass::MouseMove(long x, long y) {
	long mun = -1;
	if (x < 0) {
		
	}
	AdjustToGrid(x, y);
	long ex = x, ey = y;
	HDC hdc;
	RECT* rt = &shapenum->rt;
	switch (isDrag) {
	case Drag_Draw:
		hdc = GetDC(api_hwnd);
		SetROP2(hdc, R2_NOTXORPEN);
		switch (drawtype) {
		case DrawType_Polygon:case DrawType_Line:
			MoveToEx(hdc, mx, my, 0);
			LineTo(hdc, oldx, oldy);
			MoveToEx(hdc, mx, my, 0);
			LineTo(hdc, ex, ey);
			break;
		case DrawType_Circle:
			Ellipse(hdc, mx, my, oldx, oldy);
			Ellipse(hdc, mx, my, ex, ey);
			break;
		case DrawType_Square:case DrawType_Text:case DrawType_Bitmap:
		case DrawType_Meta:
			Rectangle(hdc, mx, my, oldx, oldy);
			Rectangle(hdc, mx, my, ex, ey);
			break;
		}
		oldx = ex; oldy = ey;
		ReleaseDC(api_hwnd, hdc);
		break;
	case Drag_Move:
		/*SetRect(rt, rt->left + ex - oldx, rt->top + ex - oldx,
			rt->right + ex - oldx, rt->bottom + ex - oldx);*/
		DrawTemp();


		if (shapenum->type == DrawType_Polygon) {
			shapelist->setPolyOffset(shapenum, ex - oldx, ey - oldy);
		}
		OffsetRect(rt, ex - oldx, ey - oldy);
		oldx = ex;
		oldy = ey;
		DrawTemp();
		break;
	case Drag_Size:
		DrawTemp();
		switch (tracknum) {
		case 1:
			rt->left = x;
			rt->top = y;
			break;
		case 2:rt->top = y; break;
		case 3:rt->right = x; rt->top = y; break;

		case 4:rt->left = x; break;
		case 5:rt->right = x; break;

		case 6:rt->left = x; rt->bottom = y; break;
		case 7:rt->bottom = y; break;
		case 8:
			rt->right = x;
			rt->bottom = y;
			//oldx = x; oldy = y;

			break;
		}
		DrawTemp();
		break;
	}
}

void APIClass::LButtonUp() {

	if (isDrag == Drag_Draw) {
		RECT rt; isDrag = 0;
		switch (drawtype) {
		case DrawType_Line:
			if ((mx - oldx) + (my - oldy) != 0) {
				SetRect(&rt, mx, my, oldx, oldy);
				shapelist->append(drawtype, &rt);
				shapenum = shapelist->data + (shapelist->len - 1);
			}
			break;
		case DrawType_Polygon: {
			//long prevx, prevy;
			if (point_idx)
			{
				if ((mx == oldx && my == oldy) ||
					(oldx == point[0].x && oldy == point[0].y)) goto ADD;
			}
			else {
				point[0].x = mx;
				point[point_idx++].y = my;
			}
			point[point_idx].x = oldx;
			point[point_idx++].y = oldy;
			if (point_idx == 9)goto ADD;

			mx = oldx; my = oldy;
			InvalidateRect(api_hwnd, 0, false);
			/*{
				HDC hdc= GetDC(api_hwnd);
				Polygon(hdc, point, point_idx);
				ReleaseDC(api_hwnd, hdc);
			}*/
			return;
		ADD:
			shapelist->append_polygon(point, point_idx);
			shapenum = shapelist->data + (shapelist->len - 1);
			point_idx = 0;
			isDrag = 0;

			InvalidateRect(api_hwnd, 0, false);
		}
		return;
		case DrawType_Bitmap:
			InsertBitmap(mx, my);
			break;
		case DrawType_Meta:
			InsertMeta(mx, my);
			break;
		case DrawType_Text:
			//InsertBitmap(mx, my);
			break;
		default:
			if (abs((mx - oldx) * (my - oldy)) > 100) {
				SetRect(&rt, mx, my, oldx, oldy);
				shapelist->append(drawtype, &rt);
				shapenum = shapelist->data + (shapelist->len - 1);
			}
			break;
		}
	}
	else if (isDrag == Drag_Move) {
		//SetRect(&shapenum->rt,mx, my;
	}
	else if (isDrag == Drag_Size) {
		tracknum = 0;
		if (shapenum->type != DrawType_Line)
			shapelist->AdjustRect(&shapenum->rt);
		
	}
	isDrag = 0;
	ReleaseCapture();
	InvalidateRect(api_hwnd, 0, false);
}
inline Color SetColor2(WORD alpha, DWORD color) {
	return Color(alpha,
		GetRValue(color),
		GetGValue(color),
		GetBValue(color)
	);
}
void APIClass::Paint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	RECT* rt;

	RECT bitrt;
	HDC hmemdc;
	HBITMAP holdbit;

	hmemdc = CreateCompatibleDC(hdc);

	GetClientRect(hwnd, &bitrt);
	if (hBuffer == 0)
		hBuffer = CreateCompatibleBitmap(hdc,
			bitrt.right, bitrt.bottom);
	holdbit = (HBITMAP)SelectObject(hmemdc, hBuffer);
	FillRect(hmemdc, &bitrt, GetSysColorBrush(COLOR_WINDOW));
	HPEN oldpen;
	if (isgrid) {
		oldpen = (HPEN)SelectObject(hmemdc,
			CreatePen(PS_SOLID, 1, RGB(200, 200, 200))
		);
		for (int y = 0; y < bitrt.bottom; y += gridw) {
			MoveToEx(hmemdc, 0, y, 0);
			LineTo(hmemdc, bitrt.right, y);
		}
		for (int x = 0; x < bitrt.right; x += gridw) {
			MoveToEx(hmemdc, x, 0, 0);
			LineTo(hmemdc, x, bitrt.bottom);
		}
		DeleteObject(SelectObject(hmemdc, oldpen));
	}
	Graphics gp(hmemdc);
	Color basiccol = Color(0, 255, 255, 255);
	Pen gpen(basiccol);
	SolidBrush gbrush(basiccol);
	Brush* allbrush;
	Point p[10];
	Image I(L"D:/ext.jpg");
	//gp.DrawEllipse(&gpen, 10, 10, 200, 200);
	gp.SetSmoothingMode(SmoothingModeAntiAlias);
	Gdiplus::Rect grt;
	for (auto d = shapelist->data; d != shapelist->data + shapelist->len; d++) {
		if (d->type == DrawType_Polygon)
			for (int i = 0; i < d->point_len; i++) {
			p[i].X = d->point[i].x;
			p[i].Y = d->point[i].y;
		}
		rt = &d->rt;
		grt.X = rt->left;
		grt.Y = rt->top;
		grt.Width = rt->right - rt->left;
		grt.Height = rt->bottom - rt->top;
		gpen.SetColor(
			SetColor2(d->alpha * (d->thick ? 1 : 0), d->linecolor)
		);
		//pen.SetStartCap(LineCapRoundAnchor);
		//gpen.SetEndCap(LineCapArrowAnchor);

		gpen.SetDashStyle((DashStyle)d->linetype);
		gpen.SetWidth(d->thick);
		{
			allbrush = &gbrush;

			if (d->brushmode == Brush_None) {
				gbrush.SetColor(basiccol);
			}
			else if (d->brushmode == Brush_Plane) {
				gbrush.SetColor(
					SetColor2(d->alpha,d->planecolor)
				);
			}
			else if (d->brushmode == Brush_Hatch) {
				allbrush = new HatchBrush(
					(HatchStyle)d->brushtype,
					SetColor2(d->alpha, d->planecolor),
					SetColor2(d->alpha, d->planecolor2));
			}
			else if (d->brushmode == Brush_Grad) {
				if (d->brushtype < 4) {
					allbrush = new LinearGradientBrush(
						grt,
						SetColor2(d->alpha, d->planecolor),
						SetColor2(d->alpha, d->planecolor2),
						d->brushtype * 90.0f
					);
				}
				else {
					GraphicsPath path;
					if (d->type == DrawType_Square) {
						path.AddRectangle(grt);

					}
					else if (d->type == DrawType_Circle) {
						path.AddEllipse(grt);

					}
					else if (d->type == DrawType_Polygon) {
						path.AddPolygon(p, d->point_len);
					}
					//path.AddEllipse(grt);
					auto pathbrush = new PathGradientBrush(&path);
					Color c1, c2;
					if (d->brushtype == 4) {
						c1 = SetColor2(d->alpha, d->planecolor);
						c2 = SetColor2(d->alpha, d->planecolor2);

					}
					else {
						c2= SetColor2(d->alpha, d->planecolor);
						c1 = SetColor2(d->alpha, d->planecolor2);
					}
					pathbrush->SetCenterColor(c1);
					Color cols[] = { c2 };
					int i = 1;
					pathbrush->SetSurroundColors(cols, &i);
					allbrush = pathbrush;
				}
			}
			switch (d->type) {
			case DrawType_Polygon:
			{
				gp.DrawPolygon(&gpen, p, d->point_len);
				gp.FillPolygon(allbrush, p, d->point_len);
			}
			break;
			case DrawType_Circle:
				gp.DrawEllipse(&gpen, grt);
				gp.FillEllipse(allbrush, grt);
				break;
			case DrawType_Square:
				gp.DrawRectangle(&gpen, grt);
				gp.FillRectangle(allbrush, grt);
				break;
			case DrawType_Line:
				gp.DrawLine(&gpen, Point(rt->left, rt->top), Point(rt->right, rt->bottom));
				break;
			case DrawType_Bitmap: {
				BITMAPFILEHEADER* fh;
				BITMAPINFOHEADER* ih;
				BYTE* pbuf;
				fh = (BITMAPFILEHEADER*)d->image;
				pbuf = (PBYTE)fh + fh->bfOffBits;
				ih = (BITMAPINFOHEADER*)((PBYTE)fh + sizeof(BITMAPFILEHEADER));
				long bx = ih->biWidth;
				long by = ih->biHeight;
				StretchDIBits(hmemdc,
					d->rt.left, d->rt.top,
					d->rt.right - d->rt.left,
					d->rt.bottom - d->rt.top, 0, 0,
					bx, by, pbuf,
					(BITMAPINFO*)ih,
					DIB_RGB_COLORS, SRCCOPY
				);
			}break;
			case DrawType_Meta:
				PlayPlaceableMeta(hmemdc,
					d->image,
					d->point_len,
					&d->rt);
				break;
			}
		}
		if (d->brushmode >= Brush_Hatch)delete allbrush;

	}
	if (shapenum)DrawTracker(hmemdc);
	if (drawtype == DrawType_Polygon) {
		Polygon(hmemdc, point, point_idx);
	}
	BitBlt(hdc, 0, 0, bitrt.right, bitrt.bottom, hmemdc, 0, 0, SRCCOPY);
	SelectObject(hmemdc, holdbit);
	DeleteDC(hmemdc);

	EndPaint(hwnd, &ps);
}
void APIClass::Command(USHORT type, WPARAM wp) {
	switch (type) {
	case DrawType_None:
		drawtype = type;
		isDrag = 0;
		CheckMenuItem((HMENU)wp, DrawType_None, MF_BYCOMMAND | MF_CHECKED);
		break;
	case DrawType_Polygon:
	case DrawType_Circle:
	case DrawType_Square:
	case DrawType_Line:
	case DrawType_Text:
	case DrawType_Bitmap:
	case DrawType_Meta:
		drawtype = type;
		Set_RadioPlaneEnable((drawtype != DrawType_Line)&&(drawtype!=DrawType_Bitmap)&&(drawtype!=DrawType_Meta));
		break;
	case Menu_FarForward:
		if (drawtype == DrawType_None && shapenum) {
			shapenum = shapelist->forward_shape(shapenum, true);
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_FarBack:
		if (drawtype == DrawType_None && shapenum) {
			shapenum = shapelist->backward_shape(shapenum, true);
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_Forward:
		if (drawtype == DrawType_None && shapenum) {
			shapenum = shapelist->forward_shape(shapenum);
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_Backward:
		if (drawtype == DrawType_None && shapenum) {
			shapenum = shapelist->backward_shape(shapenum);
			InvalidateRect(api_hwnd, 0, true);
		}
		break;
	case Menu_Prop: {
		if (!IsWindow(dialog_hwnd)) {
			WNDCLASSEXW wc = { 0 };
			wc.cbSize = sizeof(WNDCLASSEXW);
			wc.lpfnWndProc = (WNDPROC)DialogProc;
			wc.hInstance = g_hin;
			wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
			wc.lpszClassName = L"DialogClass";

			RegisterClassExW(&wc);

			dialog_hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME,//| WS_EX_TOPMOST,
				L"DialogClass", L"도형 속성",
				WS_VISIBLE | WS_SYSMENU | WS_CAPTION, 100, 100, 400, 300,
				api_hwnd, 0, g_hin, NULL);
			this->col.hwndOwner = dialog_hwnd;
		}

	}break;
	case Menu_ImgOption: {
		if (!IsWindow(dlg2_hwnd)) {
			WNDCLASSEXW wc = { 0 };
			wc.cbSize = sizeof(WNDCLASSEXW);
			wc.lpfnWndProc = (WNDPROC)ImgDialogProc;
			wc.hInstance = g_hin;
			wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
			wc.lpszClassName = L"DialogClass2";

			RegisterClassExW(&wc);

			dlg2_hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME,//| WS_EX_TOPMOST,
				L"DialogClass2", L"이미지 속성",
				WS_VISIBLE | WS_SYSMENU | WS_CAPTION, 100, 100, 400, 300,
				api_hwnd, 0, g_hin, NULL);
		}

	}
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
void APIClass::RButtonUp(long x, long y) {

	HMENU hpop;
	HMENU hsubmenu;
	POINT point = { x ,y };
	//if (isDrag)SendMessage(api_hwnd, WM_LBUTTONUP, 0, 0);
	if (!isDrag) {
		hpop = CreatePopupMenu();
		
		ClientToScreen(api_hwnd, &point);
		HMENU hsubmenu2 = CreatePopupMenu();
		AppendMenu(hpop, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenu2, L"추가");

		AppendMenu(hsubmenu2, MF_STRING, DrawType_None, L"선택\t1");
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Circle, L"원\t2");
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Square, L"사각형\t3");
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Line, L"직선\t4");
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Polygon, L"다각형\t5");
		AppendMenu(hsubmenu2, MF_SEPARATOR, 0, 0);
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Bitmap, L"이미지\t6");
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Meta, L"메타 파일\t7");
		AppendMenu(hsubmenu2, MF_STRING, DrawType_Text, L"텍스트\t8");

		hsubmenu = CreatePopupMenu();
		AppendMenu(hpop, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenu, L"정렬/삭제");
		AppendMenu(hsubmenu, MF_STRING, Menu_Forward, L"앞으로 이동");
		AppendMenu(hsubmenu, MF_STRING, Menu_FarForward, L"맨 앞으로 이동");
		AppendMenu(hsubmenu, MF_STRING, Menu_Backward, L"뒤로 이동");
		AppendMenu(hsubmenu, MF_STRING, Menu_FarBack, L"맨 뒤로 이동");
		AppendMenu(hsubmenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hsubmenu, MF_STRING, Menu_Delete, L"선택한 도형 삭제\tDel");
		AppendMenu(hsubmenu, MF_STRING, Menu_Clear, L"전체 삭제\tCtrl+Del");

		AppendMenu(hpop, MF_SEPARATOR, 0, 0);
		AppendMenu(hpop, MF_STRING, Menu_Prop, L"도형 속성\tCtrl+P");
		AppendMenu(hpop, MF_STRING, Menu_ImgOption, L"이미지 속성\tCtrl+I");
		AppendMenu(hpop, MF_SEPARATOR, 0, 0);
		AppendMenu(hpop, MF_STRING, Menu_Exit, L"QUIT");
		CheckMenuItem(hpop, drawtype, MF_BYCOMMAND | MF_CHECKED);
		TrackPopupMenu(hpop, TPM_RIGHTBUTTON, point.x, point.y,
			0, api_hwnd, 0);
		DestroyMenu(hpop);
	}
}
APIClass* api;
HFONT hfont = 0;
LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		/*if (hfont == 0)hfont = CreateFont(22, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET,
			0, 0, 0, 0, L"맑은 고딕");*/
	{
		USHORT y = 100;
		CreateWindowW(L"button", L"선 색상",
			WS_VISIBLE | WS_CHILD,
			180, y, 80, 25, hwnd, (HMENU)1, NULL, NULL);
		y += 30;
		hwnd_plane = CreateWindowW(L"button", L"내부 색상",
			WS_VISIBLE | WS_CHILD,
			180, y, 80, 25, hwnd, (HMENU)2, NULL, NULL);
		y += 30;
		hwnd_plane2 = CreateWindowW(L"button", L"내부 색상2",
			WS_VISIBLE | WS_CHILD,
			180, y, 80, 25, hwnd, (HMENU)DID_BtnPlane2, NULL, NULL);
		y += 30;
		CreateWindowW(L"button", L"설정 복사",
			WS_VISIBLE | WS_CHILD,
			150, y, 80, 25, hwnd, (HMENU)DID_BtnSet, NULL, NULL);
		y += 30;
		CreateWindowW(L"button", L"설정 붙여넣기",
			WS_VISIBLE | WS_CHILD,
			150, y, 120, 25, hwnd, (HMENU)DID_BtnPaste, NULL, NULL);
		CreateWindow(L"button", L"색상 설정",
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			140, 80, 150, 170, hwnd, (HMENU)0, 0, 0);
	}
	CreateWindow(L"button", L"채우기 모드",
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		5, 80, 120, 125, hwnd, (HMENU)0, 0, 0);

	dlg_radio[0].hcheck = CreateWindow(L"button", L"색상 없음", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON
		| WS_GROUP,
		15, 100, 100, 20, hwnd, (HMENU)DID_RadioNoBrush, 0, 0);

	dlg_radio[1].hcheck = CreateWindow(L"button", L"일반", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		15, 125, 100, 20, hwnd, (HMENU)DID_RadioPlane, 0, 0);
	dlg_radio[1].ischeck = true;
	dlg_radio[2].hcheck = CreateWindow(L"button", L"해치", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON
		, 15, 150, 100, 20, hwnd, (HMENU)DID_RadioHatch, 0, 0);
	dlg_radio[3].hcheck = CreateWindow(L"button", L"그라데이션", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON
		, 15, 175, 100, 20, hwnd, (HMENU)DID_RadioGrad, 0, 0);
	CheckRadioButton(hwnd, DID_RadioNoBrush, DID_RadioGrad, DID_RadioPlane);


	CreateWindow(L"static", L"선 두께", WS_CHILD | WS_VISIBLE,
		10, 10, 80, 25, hwnd, (HMENU)-1, 0, NULL);
	CreateWindow(L"static", L"선 타입", WS_CHILD | WS_VISIBLE,
		10, 30, 80, 25, hwnd, (HMENU)-1, 0, NULL);
	CreateWindow(L"static", L"알파값", WS_CHILD | WS_VISIBLE,
		10, 50, 80, 25, hwnd, (HMENU)-1, 0, NULL);

	hwnd_static[0] = CreateWindow(L"static", L"0", WS_CHILD | WS_VISIBLE,
		190, 10, 30, 25, hwnd, (HMENU)-1, 0, NULL);
	hwnd_static[1] = CreateWindow(L"static", L"0", WS_CHILD | WS_VISIBLE,
		190, 30, 30, 25, hwnd, (HMENU)-1, 0, NULL);
	hwnd_static[2] = CreateWindow(L"static", L"0", WS_CHILD | WS_VISIBLE,
		190, 50, 30, 25, hwnd, (HMENU)-1, 0, NULL);



	dlg_scroll.Create(hwnd, api->g_hin, ID_Scroll, 70, 10, 10, 100);
	dlg_scroll_line.Create(hwnd, api->g_hin, DID_ScrollLine, 70, 30, 7, 100);
	dlg_scroll_alpha.Create(hwnd, api->g_hin, DID_ScrollAlpha, 70, 50, 255, 100);

	dlg_combo.Create(hwnd, 230, 20, DID_ComboLine);
	dlg_combo_grad.Create(hwnd, 230, 50, DID_ComboGrad);


	/*dlg_combo.Add(L"일반");
	dlg_combo.Add(L"줄무니 ↙");
	dlg_combo.Add(L"바둑판");
	dlg_combo.Add(L"줄무니 ※");
	dlg_combo.Add(L"줄무니 ↘");
	dlg_combo.Add(L"수평선");
	dlg_combo.Add(L"수직선");*/
	{
	}
	for (int i = 0; i < HatchStyleMax; i++) {
		{
			wchar_t ts[128];
			wsprintf(ts, L"Style %d", i);
			dlg_combo.Add(ts);
		}
	}

	dlg_combo_grad.Add(L"→");
	dlg_combo_grad.Add(L"↓");
	dlg_combo_grad.Add(L"←");
	dlg_combo_grad.Add(L"↑");
	dlg_combo_grad.Add(L"원형1");
	dlg_combo_grad.Add(L"원형2");

	api->SetDialogControl();
	//SendMessage(hwnd_static[0], WM_SETFONT, (WPARAM)hfont, MAKELPARAM(1, 0));
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1:api->Set_ShapeColor(true);  break;
		case 2:api->Set_ShapeColor(); break;
		case DID_BtnSet:
			if (api->shapenum)api->shapelist->tempshape = *api->shapenum;
			break;
		case DID_BtnPaste:
			if (api->shapenum) {
				{
					auto shp = api->shapenum;

					ShapeList::ShapeData bf = *shp;


					*shp = api->shapelist->tempshape;

					shp->image = bf.image;
					shp->point_len = bf.point_len;
					shp->point = bf.point;
					shp->rt = bf.rt;
					shp->type = bf.type;
					shp->t = bf.t;


					api->SetDialogControl();
					InvalidateRect(api->api_hwnd, 0, true);
				}
			}
			break;
		case DID_ComboLine:
			api->Set_BrushType(dlg_combo.GetIndex()); break;
		case DID_ComboGrad:
			api->Set_BrushType(dlg_combo_grad.GetIndex()); break;

		case DID_BtnPlane2:api->Set_PlaneColor2(); break;

		case DID_RadioNoBrush:api->Set_RadioBrush(0); break;
		case DID_RadioPlane:api->Set_RadioBrush(1); break;
		case DID_RadioHatch:api->Set_RadioBrush(2); break;
		case DID_RadioGrad:api->Set_RadioBrush(3); break;
		}
		//DestroyWindow(hwnd);
		break;
	case WM_DRAWITEM:
	{
		HBRUSH hbrush;
		auto lpdraw = (LPDRAWITEMSTRUCT)lParam;
		if (lpdraw->itemState & ODS_SELECTED) {
			hbrush = CreateSolidBrush(RGB(100, 100, 100));
		}
		else hbrush = CreateSolidBrush(RGB(255, 255, 255));
		FillRect(lpdraw->hDC, &lpdraw->rcItem, hbrush);
		if (lpdraw->itemState & ODS_FOCUS) {
			DrawFocusRect(lpdraw->hDC, &lpdraw->rcItem);
		}
		DeleteObject(hbrush);

	}
	case WM_HSCROLL:
		api->Set_ScrollData(dlg_scroll,
			&api->GetNowShape()->thick, api->GetNowShape(), wParam, lParam);
		api->Set_ScrollData(dlg_scroll_alpha,
			&api->GetNowShape()->alpha, api->GetNowShape(), wParam, lParam);
		api->Set_ScrollData(dlg_scroll_line,
			&api->GetNowShape()->linetype, api->GetNowShape(), wParam, lParam);		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		auto shape = api->shapenum;
		if (!shape)shape = api->GetTempShape();
		HBRUSH hbrush = (HBRUSH)CreateSolidBrush(shape->linecolor);
		HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, hbrush);
		Rectangle(hdc, 150, 100, 170, 120);
		DeleteObject(hbrush);

		hbrush = CreateSolidBrush(shape->planecolor);
		SelectObject(hdc, hbrush);
		Rectangle(hdc, 150, 130, 170, 150);
		DeleteObject(hbrush);

		hbrush = CreateSolidBrush(shape->planecolor2);
		SelectObject(hdc, hbrush);
		Rectangle(hdc, 150, 160, 170, 180);
		DeleteObject(hbrush);

		SelectObject(hdc, oldbrush);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_CLOSE:
		if (hfont) {
			DeleteObject(hfont); hfont = 0;
		}
		DestroyWindow(hwnd);
		break;
	}
	return (DefWindowProcW(hwnd, msg, wParam, lParam));
}
LRESULT CALLBACK ImgDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
	{
	}break;
	case WM_CLOSE:DestroyWindow(hwnd);
		break;
	}
	return(DefWindowProcW(hwnd, msg, wParam, lParam));
}
#include <windowsx.h>
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_KILLFOCUS:api->KillFocus(); return 0;
	case WM_KEYDOWN:api->KeyDown(wp); return 0;
	case WM_KEYUP:api->KeyUp(wp); return 0;
	case WM_SIZE:api->OnSize(wp); return 0;
	case WM_CREATE:api->Create(hwnd); return 0;
	case WM_COMMAND:api->Command(LOWORD(wp), wp); return 0;
	case WM_PAINT:api->Paint(hwnd); return 0;
	case WM_MOUSEMOVE:api->MouseMove((short)LOWORD(lp), (short)HIWORD(lp)); return 0;
		return 0;
		//case WM_CHAR:api->OnChar(wp); return 0;
	case WM_LBUTTONDOWN:
		api->LButtonDown((short)LOWORD(lp), (short)HIWORD(lp)); return 0;
	case WM_LBUTTONUP:
		api->LButtonUp(); return 0;
	case WM_RBUTTONUP:api->RButtonUp((short)LOWORD(lp), (short)HIWORD(lp));
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
	//use it when memory leaked 
	//_CrtSetBreakAlloc(160);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	HWND hwnd;
	WNDCLASS win = { 0 };
	api = new APIClass(hin);
	win.hbrBackground = 0;//(HBRUSH)GetStockObject(WHITE_BRUSH);
	//CreateHatchBrush(HS_DIAGCROSS,RGB(0, 150, 123));
	win.hIcon = LoadIcon(0, IDI_APPLICATION);
	win.hCursor = LoadCursor(0, IDC_ARROW);
	win.hInstance = hin;
	win.lpfnWndProc = WndProc;
	win.lpszClassName = L"WINDRAW";
	win.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&win);
	ULONG_PTR gdiptr;
	GdiplusStartupInput gdistart;
	if (GdiplusStartup(&gdiptr, &gdistart, 0) != Gdiplus::Ok) {
		MessageBox(0, L"GDIPLUS 실패", L"jotdi", MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindow(L"WINDRAW", L"WINDRAW", WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		//CW_USEDEFAULT, CW_USEDEFAULT,
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
	GdiplusShutdown(gdiptr);
	return 0;
}

/*

https://stpetrus27.wordpress.com/2018/06/04/vc-vc-dialog-without-the-resource-file/
*/
