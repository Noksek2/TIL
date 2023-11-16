#include <Windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#ifdef RESOURCE
#include "resource.h"
#endif

enum {
	DrawType_None, DrawType_Circle, DrawType_Square, DrawType_Line,
	Menu_Delete, Menu_Clear, Menu_Prop,
	Menu_Forward, Menu_FarForward, Menu_Backward, Menu_FarBack,
	Menu_Exit, ID_Scroll, DrawType_Polygon,
};
enum {
	DID_ColorCheck = 3, DID_ScrollLine = 4,DID_ComboLine,DID_BtnSet
};
enum {
	Drag_None = 0, Drag_Draw = 1, Drag_Move, Drag_Size
};
inline int LP_GetMouseX(LPARAM lp) { return LOWORD(lp); }
inline int LP_GetMouseY(LPARAM lp) { return HIWORD(lp); }

LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct ShapeList {
	struct ShapeData {
		RECT rt;
		POINT* point;
		USHORT type;
		USHORT thick;
		COLORREF linecolor;
		COLORREF planecolor;
		UCHAR point_len,linetype,brushtype;
		bool show;
		ShapeData() {
			point_len = linetype =0;
			brushtype = 0;
			planecolor = RGB(50,200,75);
			linecolor = RGB(0,0,0);
			thick = 1;
			type = 0;
			point = 0;
			rt = { 0 };
			show = true;
		}
	};
	ShapeData* data;
	ShapeData tempshape;


	UINT capa, len;
	/*COLORREF nowcol[2];
	USHORT defthick;
	bool isshow;*/
	ShapeList() {
		tempshape.show = true;
		capa = 10; len = 0; tempshape.thick= 1;
		data = (ShapeData*)malloc(sizeof(ShapeData) * capa);
	}
	void append(USHORT type, RECT* mrt) {
		if (len >= capa)return;
		data[len] = tempshape;
		data[len].type = type;
		//data[len].thick = defthick;
		//data[len].linecolor = nowcol[0];
		//data[len].planecolor = nowcol[1];
		//data[len].show = isshow;
		//data[len].linetype = 0;
		if (type != DrawType_Line)AdjustRect(mrt);
		memcpy(&data[len].rt, mrt, sizeof(RECT));


		wchar_t str[128];
		wsprintf(str, L"append shape[%d] type:%d \n", len, type);
		OutputDebugString(str);
		len++;
	}
	void append_polygon(POINT* _point, UCHAR point_len) {
		if (len >= capa)return;
		data[len].type = DrawType_Polygon;
		data[len] = tempshape;/*
		data[len].thick = defthick;
		data[len].linecolor = nowcol[0];
		data[len].planecolor = nowcol[1];
		data[len].show = isshow;*/
		data[len].point = (POINT*)malloc(sizeof(POINT) * point_len);
		data[len].point_len = point_len;
		memcpy(data[len].point, _point, sizeof(POINT) * point_len);

		
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
	ShapeData* find(USHORT x, USHORT y) {
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
class CScroll {
public:
	WORD pos;
	HWND hscroll;
	WORD pos_max, inc;
	void Create(HWND hwnd, HINSTANCE g_hin, UINT id, int x, int y, WORD _max = 255, int width = 200) {
		hscroll = CreateWindow(L"scrollbar", 0, WS_CHILD | WS_VISIBLE |
			SBS_HORZ, x, y, width, 20, hwnd, (HMENU)id, g_hin, 0);
		SetScrollRange(hscroll, SB_CTL, 0, _max, true);
		pos_max = _max;
		inc = max(1, pos_max / 10);
		SetScrollPos(hscroll, SB_CTL, 0, true);
	}
	void Process(WPARAM wp, LPARAM lp) {
		WORD msgtype = LOWORD(wp),
			temp = HIWORD(wp);
		switch (msgtype) {
		case SB_LINELEFT:pos = max(0, pos - 1); break;
		case SB_LINERIGHT:pos = min(pos_max, pos + 1); break;
		case SB_PAGELEFT:pos = max(0, pos - inc); break;
		case SB_PAGERIGHT:pos = min(pos_max, pos + inc); break;
		case SB_THUMBTRACK:pos = temp; break;
		}
		SetScrollPos((HWND)lp, SB_CTL, pos, true);
	}
	void Set(WORD _pos) {
		pos = _pos;
		SetScrollPos(hscroll, SB_CTL, pos, true);
	}
};
class CCheckBox {
public:
	bool ischeck;
	HWND hcheck;
	void Create(HWND hwnd, UINT id, const TCHAR* name, bool check = true) {
		ischeck = check;
		hcheck = CreateWindow(
			TEXT("button"),
			name,
			WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
			20, 80, 100, 25,
			hwnd,
			(HMENU)id,
			0, NULL);
	}
	void Set(bool check) {
		ischeck = check;
		if (check) {
			SendMessage(hcheck, BM_SETCHECK, BST_CHECKED, 0);
		}
		else SendMessage(hcheck, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	void Process(WPARAM wp) {
		switch (HIWORD(wp)) {
		case BN_CLICKED:
			if (SendMessage(hcheck, BM_GETCHECK, 0, 0) == BST_CHECKED) {
				Set(false);
			}
			else {
				Set(true);
			}
			break;
		}
	}
};
class CComboBox {
public:
	HWND hcombo;
	void Create(HWND hwnd,int x,int y,int id) {
		hcombo = CreateWindow(
			L"combobox", 0, WS_CHILD | WS_VISIBLE
			| CBS_DROPDOWNLIST, x, y, 100, 500, hwnd, (HMENU)id, 0, 0
		);
	}
	void Add(const TCHAR* lists){
		SendMessage(hcombo, CB_ADDSTRING, 0, (LPARAM)lists);
	}
	void Process(WPARAM wp)
	{
		int idx;
		switch (HIWORD(wp)) {
		case CBN_SELCHANGE:
			idx=SendMessage(hcombo, CB_GETCURSEL, 0, 0);
			break;
		//case CBN_EDITCHANGE:break;
		}
	}
	inline int GetIndex() {
		return SendMessage(hcombo, CB_GETCURSEL, 0, 0);
	}
	inline int SetIndex(int idx) {
		return SendMessage(hcombo, CB_SETCURSEL, idx,0);
	}
};
CScroll dlg_scroll,dlg_scroll_line;
CCheckBox dlg_check;
CComboBox dlg_combo;

class APIClass {
private:
	USHORT mx, my, oldx, oldy;
	CHOOSECOLOR col;
	COLORREF coltemp[16];
	POINT point[10];
	UCHAR point_idx;
	HBITMAP hBuffer;
	inline void DrawOneTrack(HDC hdc, long x, long y) {
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
	int TrackerHit(long x, long y) {
		POINT pt;
		pt.x = x;
		pt.y = y;
		if (!shapenum)return 0;
		RECT trackrt;
		if (shapenum->type == DrawType_Polygon) {
			{
				long tx, ty;
				for(int i=0;i<shapenum->point_len;i++){
					tx = shapenum->point[i].x;
					ty = shapenum->point[i].y;
					SetRect(&trackrt, tx - 5, ty - 5, tx + 5, ty + 5);
					if (PtInRect(&trackrt, pt))return i;
				}
			}
			return 0;
		}
		else if (shapenum->type == DrawType_Line)
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
		case DrawType_Square:
			Rectangle(hdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Polygon:
			Polygon(hdc, point, point_idx);
			break;
		}
		DeleteObject(SelectObject(hdc, oldpen));
		SelectObject(hdc, oldbrush);
		ReleaseDC(api_hwnd, hdc);
	}
	void DrawTracker(HDC hdc) {
		RECT rt = shapenum->rt;
		if (shapenum->type == DrawType_Line) {


			DrawOneTrack(hdc, rt.left, rt.top);
			DrawOneTrack(hdc, rt.right, rt.bottom);
		}
		else if (shapenum->type == DrawType_Polygon) {
			for(int i=0;i<shapenum->point_len;i++)
				DrawOneTrack(hdc, shapenum->point[i].x, shapenum->point[i].y);
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
	HWND api_hwnd, dialog_hwnd;
	HINSTANCE g_hin;
	ShapeList::ShapeData* shapenum;
	USHORT isDrag;
	UCHAR tracknum;
	USHORT drawtype;

	APIClass(HINSTANCE hin) :g_hin(hin) {
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
		col.hwndOwner = hwnd;
		for (int i = 0; i < 16; i++)
			coltemp[i] = RGB(255, 255, 255);
	}
	void Paint(HWND hwnd);
	void Command(USHORT, WPARAM);
	void MouseMove(USHORT, USHORT);
	void LButtonDown(USHORT, USHORT);
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
	void RButtonUp(USHORT, USHORT);
	void Destroy() { 
		if (hBuffer) {
		DeleteObject(hBuffer);
		hBuffer = 0;
	}
	PostQuitMessage(0);
	}

	void Set_ShapeColor(bool isline = false) {
		if (ChooseColor(&col)) {
			if (shapenum) {
				if (!isline)shapenum->planecolor = col.rgbResult;
				else shapenum->linecolor = col.rgbResult;
				InvalidateRect(api_hwnd, 0, true);
			}
			else {
				if (!isline)shapelist->tempshape.planecolor = col.rgbResult;
				else shapelist->tempshape.linecolor = col.rgbResult;
			}
		}
	}
	void Set_AlphaCheck(bool isshow) {

		if (shapenum) {
			shapenum->show = isshow;
			InvalidateRect(api_hwnd, 0, true);
		}
		else {
			shapelist->tempshape.show = isshow;
		}

	}
	UCHAR IndextoHatch(UCHAR i){
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
	};
	void Set_BrushType(UCHAR i) {
		if (shapenum) {
			shapenum->brushtype = i;
			InvalidateRect(api_hwnd, 0, true);
		}
		else {
			shapelist->tempshape.brushtype = i;
		}
	}
	~APIClass() { delete shapelist; }
	void SetDialogControl() {

		if (shapenum) {
			dlg_scroll.Set(shapenum->thick);
			dlg_check.Set(shapenum->show);
			dlg_scroll_line.Set(shapenum->linetype);
			dlg_combo.SetIndex(shapenum->brushtype);
			wchar_t str[128];
			wsprintf(str, L"도형 속성 - 선택 %d",shapenum-shapelist->data);
			SetWindowText(dialog_hwnd, str);
		}

		else {
			dlg_scroll.Set(shapelist->tempshape.thick);
			dlg_check.Set(shapelist->tempshape.show);
			dlg_scroll_line.Set(shapelist->tempshape.linetype);
			dlg_combo.SetIndex(shapelist->tempshape.brushtype);
			SetWindowText(dialog_hwnd, L"도형 속성");
			

		
		}
		if (dlg_check.ischeck)EnableWindow(dlg_combo.hcombo, true);
		else EnableWindow(dlg_combo.hcombo, false);
	}

};
void APIClass::LButtonDown(USHORT x, USHORT y) {
	ShapeList::ShapeData* FindData = 0;
	if (drawtype == DrawType_None) {
		if (shapenum) {
			tracknum = TrackerHit(x, y);
			if (tracknum) {
				isDrag = Drag_Size;
				oldx = x;
				oldy = y;
				goto END;
			}
		}
		FindData = shapelist->find(x, y);
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
		if (drawtype == DrawType_Polygon) {
			if (point_idx) {
				mx = point[point_idx - 1].x;
				my = point[point_idx - 1].y;
			}
		}
		oldx = x; oldy = y;
		isDrag = Drag_Draw;
	}
END: 
	SetDialogControl();
//SetCapture(api_hwnd);
}

void APIClass::MouseMove(USHORT x, USHORT y) {
	USHORT ex = x, ey = y;
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
		case DrawType_Square:
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
				if (mx == oldx && my == oldy) goto ADD;
				
			}
			else {
				point[point_idx].x = oldx;
				point[point_idx++].y = oldy;
				if (point_idx == 9)goto ADD;
			}
			mx = oldx; my = oldy;
			InvalidateRect(api_hwnd, 0, true);
		return;
		ADD:
				shapelist->append_polygon(point, point_idx);
				shapenum = shapelist->data + (shapelist->len - 1);
				point_idx = 0;
				isDrag = 0;
			
			InvalidateRect(api_hwnd, 0, true);
		}
							 return;
		default:
			if (abs((mx - oldx) * (my - oldy)) > 100) {
				SetRect(&rt, mx, my, oldx, oldy);
				shapelist->append(drawtype, &rt);
				shapenum = shapelist->data + (shapelist->len - 1);
			}
			break;
		}
		InvalidateRect(api_hwnd, 0, true);
	}
	else if (isDrag == Drag_Move) {
		//SetRect(&shapenum->rt,mx, my;
		InvalidateRect(api_hwnd, 0, true);
	}
	else if (isDrag == Drag_Size) {
		tracknum = 0;
		if (shapenum->type != DrawType_Line)
			shapelist->AdjustRect(&shapenum->rt);
		InvalidateRect(api_hwnd, 0, true);
	}
	isDrag = 0;
}
void APIClass::Paint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	RECT* rt;
	HPEN hpen, oldpen;
	HBRUSH hbrush, oldbrush;

	RECT bitrt;
	HDC hmemdc;
	HBITMAP holdbit;
	hmemdc = CreateCompatibleDC(hdc); SetGraphicsMode(hmemdc, GM_ADVANCED);
	GetClientRect(hwnd, &bitrt);
	if (hBuffer == 0)
		hBuffer = CreateCompatibleBitmap(hdc,
			bitrt.right, bitrt.bottom);
	holdbit = (HBITMAP)SelectObject(hmemdc,hBuffer);
	FillRect(hmemdc, &bitrt, GetSysColorBrush(COLOR_WINDOW));
	for (auto d = shapelist->data; d != shapelist->data + shapelist->len; d++) {
		rt = &d->rt;
		if (d->thick == 0)hpen = (HPEN)GetStockObject(NULL_PEN);
		else hpen = CreatePen(d->linetype, d->thick, d->linecolor);
		oldpen = (HPEN)SelectObject(hmemdc, hpen);

		if (!d->show)hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		else {
			if(d->brushtype==0)
				hbrush = (HBRUSH)CreateSolidBrush(d->planecolor);
			else 
				hbrush = (HBRUSH)CreateHatchBrush(IndextoHatch(d->brushtype),d->planecolor);
		}
		oldbrush = (HBRUSH)SelectObject(hmemdc, hbrush);

		switch (d->type) {
		case DrawType_Polygon:
			Polygon(hmemdc, d->point, d->point_len);
			break;
		case DrawType_Circle:
			Ellipse(hmemdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Square:
			Rectangle(hmemdc, rt->left, rt->top, rt->right, rt->bottom);
			break;
		case DrawType_Line:
			MoveToEx(hmemdc, rt->left, rt->top, 0);
			LineTo(hmemdc, rt->right, rt->bottom);
			break;
		}
		SelectObject(hmemdc, oldpen);
		if (d->thick)DeleteObject(hpen);

		SelectObject(hmemdc, oldbrush);
		if (d->show)DeleteObject(hbrush);
	}
	if (shapenum)DrawTracker(hmemdc);
	
	if (drawtype == DrawType_Polygon)
	{
		Polygon(hmemdc, point, point_idx);
	}
	{
		//SetGraphicsMode(hmemdc, GM_ADVANCED);
		XFORM xForm;
		xForm.eM11 = (FLOAT)1.0;
		xForm.eM12 = (FLOAT)0.0;
		xForm.eM21 = (FLOAT)0;
		xForm.eM22 = (FLOAT)1.0;
		xForm.eDx = (FLOAT)75.0;
		xForm.eDy = (FLOAT)0.0;
		//SetWorldTransform(hDC, &xForm);
		//SetMapMode(hmemdc, MM_LOENGLISH);
		//SetGraphicsMode(hdc, GM_ADVANCED);
		SetWorldTransform(hmemdc, &xForm);
		Rectangle(hmemdc, 0, 0, 300, 300);
		xForm = { 0 };
		//SetWorldTransform(hmemdc, &xForm);
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
		drawtype = type;
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
				WS_VISIBLE | WS_SYSMENU | WS_CAPTION, 100, 100, 300, 200,
				api_hwnd, 0, g_hin, NULL);
			this->col.hwndOwner = dialog_hwnd;
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
void APIClass::RButtonUp(USHORT x, USHORT y) {

	HMENU hpop;
	HMENU hsubmenu;
	POINT point = { x ,y };
	//if (isDrag)SendMessage(api_hwnd, WM_LBUTTONUP, 0, 0);
	if (!isDrag) {
		hpop = CreatePopupMenu();
		hsubmenu = CreatePopupMenu();
		ClientToScreen(api_hwnd, &point);
		AppendMenu(hpop, MF_STRING, DrawType_None, L"선택");
		AppendMenu(hpop, MF_STRING, DrawType_Circle, L"원");
		AppendMenu(hpop, MF_STRING, DrawType_Square, L"사각형");
		AppendMenu(hpop, MF_STRING, DrawType_Line, L"직선");
		AppendMenu(hpop, MF_STRING, DrawType_Polygon, L"다각형");
		AppendMenu(hpop, MF_SEPARATOR, 0, 0);

		AppendMenu(hpop, MF_STRING | MF_POPUP, (UINT_PTR)hsubmenu, L"정렬/삭제");
		AppendMenu(hsubmenu, MF_STRING, Menu_Forward, L"앞으로 이동");
		AppendMenu(hsubmenu, MF_STRING, Menu_FarForward, L"맨 앞으로 이동");
		AppendMenu(hsubmenu, MF_STRING, Menu_Backward, L"뒤로 이동");
		AppendMenu(hsubmenu, MF_STRING, Menu_FarBack, L"맨 뒤로 이동");
		AppendMenu(hsubmenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hsubmenu, MF_STRING, Menu_Delete, L"선택한 도형 삭제\tDel");
		AppendMenu(hsubmenu, MF_STRING, Menu_Clear, L"전체 삭제\tCtrl+Del");

		AppendMenu(hpop, MF_SEPARATOR, 0, 0);
		AppendMenu(hpop, MF_STRING, Menu_Prop, L"도형 속성");
		AppendMenu(hpop, MF_SEPARATOR, 0, 0);
		AppendMenu(hpop, MF_STRING, Menu_Exit, L"QUIT");
		CheckMenuItem(hpop, drawtype, MF_BYCOMMAND | MF_CHECKED);
		TrackPopupMenu(hpop, TPM_RIGHTBUTTON, point.x, point.y,
			0, api_hwnd, 0);
		DestroyMenu(hpop);
	}
}
APIClass* api;
LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		dlg_scroll.Create(hwnd, api->g_hin, ID_Scroll, 10, 10, 10, 100);
		CreateWindowW(L"button", L"선 색상",
			WS_VISIBLE | WS_CHILD,
			180, 50, 80, 25, hwnd, (HMENU)1, NULL, NULL);
		CreateWindowW(L"button", L"내부 색상",
			WS_VISIBLE | WS_CHILD,
			180, 80, 80, 25, hwnd, (HMENU)2, NULL, NULL);
		CreateWindowW(L"button", L"설정 복사",
			WS_VISIBLE | WS_CHILD,
			180, 110, 80, 25, hwnd, (HMENU)DID_BtnSet, NULL, NULL);
		dlg_scroll_line.Create(hwnd, api->g_hin, DID_ScrollLine, 10, 30, 7, 100);
		dlg_check.Create(hwnd, DID_ColorCheck, L"색상 채우기",true);
		dlg_combo.Create(hwnd, 100, 20, DID_ComboLine);
		dlg_combo.Add(L"일반");
		dlg_combo.Add(L"줄무니 ↙");
		dlg_combo.Add(L"바둑판");
		dlg_combo.Add(L"줄무니 ※");
		dlg_combo.Add(L"줄무니 ↘");
		dlg_combo.Add(L"수평선");
		dlg_combo.Add(L"수직선");
		api->SetDialogControl();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1:api->Set_ShapeColor(true); break;
		case 2:api->Set_ShapeColor(); break;
		case DID_ColorCheck:
			dlg_check.Process(wParam);
			api->Set_AlphaCheck(dlg_check.ischeck);
			if (dlg_check.ischeck)EnableWindow(dlg_combo.hcombo, true);
			else EnableWindow(dlg_combo.hcombo, false);

;			break;
		case DID_BtnSet:
			if (api->shapenum)api->shapelist->tempshape = *api->shapenum;
			break;
		case DID_ComboLine:
			api->Set_BrushType(dlg_combo.GetIndex());
			break;
		}
		//DestroyWindow(hwnd);
		break;
	case WM_HSCROLL:
		if ((HWND)lParam == dlg_scroll.hscroll) {
			dlg_scroll.Process(wParam, lParam);
			if (api->shapenum)api->shapenum->thick = dlg_scroll.pos;
			else api->shapelist->tempshape.thick = dlg_scroll.pos;
			InvalidateRect(api->api_hwnd, 0, true);
		}
		else if ((HWND)lParam == dlg_scroll_line.hscroll) {
			dlg_scroll_line.Process(wParam, lParam);
			if (api->shapenum)api->shapenum->linetype = dlg_scroll_line.pos;
			else api->shapelist->tempshape.linetype = dlg_scroll_line.pos;
			InvalidateRect(api->api_hwnd, 0, true);
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		Ellipse(hdc, 100, 20, 150, 70);
		EndPaint(hwnd, &ps);
	}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	}
	return (DefWindowProcW(hwnd, msg, wParam, lParam));
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	static HCURSOR hcursor;
	switch (msg) {
	case WM_SIZE:api->OnSize(wp);return 0;
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

	hwnd = CreateWindow(L"WINDRAW", L"WINDRAW", WS_OVERLAPPEDWINDOW | WS_VSCROLL,
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
/*

https://stpetrus27.wordpress.com/2018/06/04/vc-vc-dialog-without-the-resource-file/
23/11/16 19:17
*/
