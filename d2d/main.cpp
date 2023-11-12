#include <Windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <dwrite.h>

#pragma comment(lib,"dwrite")
#pragma comment(lib,"WindowsCodecs.lib")
#pragma comment(lib, "d2d1.lib")
using namespace D2D1;

typedef ID2D1Factory D2DFactory;
typedef ID2D1HwndRenderTarget D2DRender;
typedef ID2D1SolidColorBrush SolidBrush;
typedef ID2D1Bitmap D2DBitmap;
typedef D2D1_POINT_2F POINT_2F;

class D2D {
private:
	D2DFactory* factory;
	D2DRender* render;
	SolidBrush* solidbrush;
	ID2D1GradientStopCollection* gradcollect;
	ID2D1LinearGradientBrush* gradbrush;
	ID2D1RadialGradientBrush* radgradbrush;
	IWICImagingFactory* wicfactory;

	IDWriteFactory* dfactory;
	IDWriteTextFormat* dtextform;
public:
	D2D() {
		factory = 0;
		render = 0;
		solidbrush = 0;
		gradcollect = 0;
	}
	~D2D() {
		if (factory)factory->Release();
		if (render)render->Release();
		if (solidbrush)solidbrush->Release();
		if (wicfactory)wicfactory->Release();
		CoUninitialize();
	}

	bool init(HWND hwnd) {
		CoInitializeEx(0, COINIT_APARTMENTTHREADED);
		CoCreateInstance(CLSID_WICImagingFactory, 0,
			CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void**>(&wicfactory));
		if (S_OK != D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))
			return 0;
		RECT r;
		GetClientRect(hwnd, &r);
		factory->CreateHwndRenderTarget(
			RenderTargetProperties(),
			HwndRenderTargetProperties(hwnd, SizeU(r.right, r.bottom)),
			&render);
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&dfactory)
		);
		return true;
	}
	void Begin() {
		render->BeginDraw();
	}
	void Clear(float r, float g, float b) {
		render->Clear(ColorF(r, g, b));
	}
	void End() {
		render->EndDraw();
	}
	void CreateText() {
		dfactory->CreateTextFormat(L"Consolas",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			60.f, L"ko-kr", &dtextform
			);
		dtextform->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		dtextform->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	void PrintText(float r,float g,float b) {
		render->CreateSolidColorBrush(ColorF(r, g, b), &solidbrush);
		render->DrawTextW(
			L"hell world",10,dtextform,
			RectF(0,0,300,300),solidbrush
		);
		solidbrush->Release();
	}
	void CreateStroke(ID2D1StrokeStyle** stroke) {
		float dashes[] = { 1.0f, 2.0f, 2.0f, 3.0f, 2.0f, 2.0f };
		auto strokestyle=StrokeStyleProperties(
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_ROUND,
			D2D1_LINE_JOIN_MITER,
			10.f, D2D1_DASH_STYLE_CUSTOM,
			0.0f
		);
		factory->CreateStrokeStyle(
			strokestyle,
			dashes,
			ARRAYSIZE(dashes),
			stroke
		);
	}
	void DrawLine(D2D1_POINT_2F p1, D2D1_POINT_2F p2, ColorF col=ColorF(1,1,1),float num=1.0f,ID2D1StrokeStyle* strokestyle=0) {
		render->CreateSolidColorBrush(col, &solidbrush);

		render->DrawLine(p1, p2, solidbrush, num,strokestyle);
		solidbrush->Release();
	}
	void DrawSquare(D2D1_RECT_F rect, float r, float g, float b) {
		render->CreateSolidColorBrush(ColorF(r, g, b), &solidbrush);
		render->FillRectangle(rect, solidbrush);
		//render->DrawRectangle(rect, solidbrush);
		solidbrush->Release();
	}
	void DrawCircle(D2D1_ELLIPSE ellipse, float r, float g, float b) {
		render->CreateSolidColorBrush(ColorF(r, g, b), &solidbrush);
		render->FillEllipse(ellipse, solidbrush);
		//render->DrawRectangle(rect, solidbrush);
		solidbrush->Release();
	}
	void DrawRoundRect(float r,float g,float b) {
		render->CreateSolidColorBrush(ColorF(r, g, b), &solidbrush);
		D2D1_ROUNDED_RECT roundrect = RoundedRect(
			D2D1::RectF(200, 0, 400, 100),
			20.f, 20.f
		);
		render->FillRoundedRectangle(roundrect, solidbrush);
		solidbrush->Release();
	}
	void DrawGradSquare(D2D1_RECT_F rect, D2D1_GRADIENT_STOP* gradstop,int count=2) {
		render->CreateGradientStopCollection(
			gradstop, count, D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&gradcollect
		);
		render->CreateLinearGradientBrush(
			LinearGradientBrushProperties(
				Point2F(0, 0),
				Point2F(150, 150)),
			gradcollect,
			&gradbrush
		);
		render->FillRectangle(rect, gradbrush);
		gradbrush->Release();
	}
	void DrawGradCircle(D2D1_ELLIPSE ellipse, D2D1_GRADIENT_STOP* gradstop, int count=2) {
		render->CreateGradientStopCollection(
			gradstop, count, D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&gradcollect
		);
		render->CreateRadialGradientBrush(
			RadialGradientBrushProperties(
				Point2F(ellipse.point.x, ellipse.point.y),
				Point2F(0, 0), ellipse.radiusX, ellipse.radiusY),
			gradcollect,
			&radgradbrush
		);
		render->FillEllipse(ellipse, radgradbrush);
		radgradbrush->Release();
	}
	void LoadWICImage(const TCHAR* imgdir,ID2D1Bitmap** mybit) {
		IWICBitmapDecoder* bitdec=0;
		IWICBitmapFrameDecode* bitframe = 0;
		IWICFormatConverter* formconv = 0;
		ID2D1Bitmap* bitmap = 0;

		wicfactory->CreateDecoderFromFilename(
			imgdir, 0, GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&bitdec
		);

		bitdec->GetFrame(0, &bitframe);
		wicfactory->CreateFormatConverter(&formconv);
		formconv->Initialize(
			bitframe, GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			0, 1.0f, WICBitmapPaletteTypeMedianCut
		);
		render->CreateBitmapFromWicBitmap(formconv, 0, &bitmap);

		*mybit = bitmap;
	}
	void DrawImage(D2DBitmap* bitmap,D2D1_RECT_F rect) {
		render->DrawBitmap(bitmap, rect);
	}
};
class MainGame {
public:
	D2D* d2d;
	D2DBitmap* bitmap;
	MainGame(HWND hwnd) {
		d2d = new D2D();
		if (!d2d->init(hwnd)) {
			MessageBox(hwnd, L"응 실패함", L"오류", MB_ICONERROR);
		}

		d2d->LoadWICImage(L"icon_32.png", &bitmap);
		d2d->CreateText();
	}
	~MainGame() {
		if (d2d) {
			delete d2d; d2d = nullptr;
		}
	}

	
	void Render() {
		d2d->Begin();
		d2d->Clear(0.f, 0.f, 0.f);

		d2d->DrawSquare(RectF(300.f, 100.f, 450.f, 200.f), 0.f, 0.7, 0.2f);
		D2D1_GRADIENT_STOP gradstop[3];
		gradstop[0].color = ColorF(ColorF::ForestGreen, 1.f);
		gradstop[0].position = 1.f;
		gradstop[1].color = ColorF(ColorF::Blue, 1.f);
		gradstop[1].position = 0.5f;
		gradstop[2].color = ColorF(ColorF::White, 1.f);
		gradstop[2].position = 0.f;
		
		
		d2d->DrawGradSquare(RectF(0.f, 0.f, 200.f, 200.f), gradstop,3);
		d2d->DrawCircle(Ellipse(Point2F(300.f, 200.f), 75.f, 50.f), 1.f, 0.5f, 0.5f);

		d2d->DrawGradCircle(Ellipse(Point2F(100.f, 150.f), 75.f, 50.f), gradstop,3);
		d2d->DrawRoundRect(0.5f, 0.3f, 0.6f);
		d2d->DrawImage(bitmap,RectF(200,200,400,400));

		ID2D1StrokeStyle* stroke;
		d2d->CreateStroke(&stroke);
		d2d->DrawLine(Point2F(0, 100), Point2F(300, 200),ColorF(1.f,0.5f,0.5f),10.f,stroke);
		d2d->End();
	}
	void Shit(int x,int y) {
		d2d->Begin();
		d2d->PrintText(1.f,0.3f,0.f);
		ID2D1StrokeStyle* stroke;
		d2d->CreateStroke(&stroke);
		d2d->DrawLine(Point2F(0, 0), Point2F(x, y), ColorF(1.f, 0.f, 0.f), 5.f,stroke);
		d2d->End();
	}
};
MainGame* game;
HINSTANCE g_hin;
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {
	case WM_CREATE:
		/*CreateWindow(L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
			ES_AUTOHSCROLL, 300, 100, 200, 25, hwnd, (HMENU)10, g_hin, NULL);
		*/break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		game->Render();
		EndPaint(hwnd, &ps);
	}
				 break;
	case WM_LBUTTONDOWN:
		InvalidateRect(hwnd, 0, false);
		break;
	case WM_MOUSEMOVE:
	{
		HDC hdc = GetDC(hwnd);
		::Ellipse(hdc, 0, 0, 100, 100);
		game->Shit(LOWORD(lParam),HIWORD(lParam));
		//game->Render();
		ReleaseDC(hwnd, hdc);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hwnd, iMessage, wParam, lParam));
}
HWND CreateAdjustWindow(HINSTANCE hInstance) {
	HWND hWnd;
	WNDCLASS WndClass;
	TCHAR Title[100] = L"D2D";
	//g_hInst = hInstance;;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = Title;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	RECT rect = { 0,0,800,600 };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, 0, WS_EX_OVERLAPPEDWINDOW);

	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, Title, Title,
		WS_OVERLAPPEDWINDOW, 100, 100,
		rect.right - rect.left, rect.bottom - rect.top,
		0, 0, hInstance, 0);


	return hWnd;
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	g_hin = hInstance;
	HWND hWnd = CreateAdjustWindow(hInstance);
	game = new MainGame(hWnd);
	ShowWindow(hWnd, 1);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		//else game->Render();
	}/*
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/
	delete game;
	return (int)msg.wParam;
}
#ifdef OLD
#include <d2d1.h>
#include <Windows.h>
using namespace D2D1;

typedef ID2D1Factory* Factory;
typedef ID2D1HwndRenderTarget* Target;
typedef ID2D1SolidColorBrush* SolidBrush;
typedef ID2D1LinearGradientBrush* LineGradBrush;
typedef ID2D1RadialGradientBrush* RadGradBrush;

typedef D2D1_POINT_2F Point_2F;

class D2D {
private:
	Factory factory;
	Target target;
	SolidBrush brush;
	
public:
	enum class LineGradType {
		Top,
		Bottom,
		Left,
		Right,
		LeftTop,
		RightBottom
	};

	D2D() {
		factory = 0;
		target = 0;
		brush = 0;
	}
	bool init(HWND hwnd) {
		CoInitializeEx(0, COINIT_APARTMENTTHREADED);
		if (S_OK != D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))
			return 0;
		RECT r; 
		GetClientRect(hwnd, &r);
		factory->CreateHwndRenderTarget(
			RenderTargetProperties(),
			HwndRenderTargetProperties(hwnd, SizeU(r.right, r.bottom)),
			&target);
		return 1;
	}
	void Begin() {
		target->BeginDraw();
	}
	void Clear(float r, float g, float b)
	{
		target->Clear(ColorF(r, g, b));
	}
	void DrawSquare(D2D1_RECT_F rect, ColorF linecol,ColorF fillcol,float bold=1.f) {
		SolidBrush fbrush;
		target->CreateSolidColorBrush(linecol, &brush);
		target->CreateSolidColorBrush(fillcol, &fbrush);
		target->FillRectangle(rect, fbrush);
		target->DrawRectangle(rect, brush,bold);
		brush->Release();
		fbrush->Release();
	}
	void CreateSolidBrush(SolidBrush& brush,float r,float g,float b) {
		target->CreateSolidColorBrush(ColorF(r, g, b), &brush);
	}
	void DrawSquareBrush(D2D1_RECT_F rect,SolidBrush lbrush,SolidBrush fbrush,float bold=1.f) {
		target->FillRectangle(rect, fbrush);
		target->DrawRectangle(rect, lbrush, bold);
	}
	void DrawGradSquare(D2D1_RECT_F rect,ColorF g1,ColorF g2,LineGradType type,bool radial=false) {
		LineGradBrush lbrush;
		RadGradBrush rbrush;
		D2D1_GRADIENT_STOP gradstop[2];
		gradstop[0].color = g1;
		gradstop[0].position = 0.f;
		gradstop[1].color = g2;
		gradstop[1].position = 1.f;
		ID2D1GradientStopCollection* gradcollect=0;
		target->CreateGradientStopCollection(
			gradstop,
			2,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&gradcollect
		);
		
		Point_2F c1, c2;
		
		if (radial) {
			c1 = Point2((rect.right+rect.left)/2.f-20.f,(rect.bottom + rect.top)/2.f-20.f);
			c2 = c1;
			target->CreateRadialGradientBrush(
				RadialGradientBrushProperties(
					c1, c2, 100.f,//(rect.right - rect.left)/2.f,
					100.f//(rect.bottom - rect.top)/2.f
				),
				gradcollect,
				&rbrush
			);
			target->FillRectangle(rect, rbrush);
			rbrush->Release();
		}
		else {
			switch (type) {
			case LineGradType::LeftTop:
				c1 = Point2(rect.left, rect.top);
				c2 = Point2(rect.right, rect.bottom);
				break;
			case LineGradType::Left:
				c1 = Point2(rect.left, 0.f);
				c2 = Point2(rect.right, 0.f);
				break;
			}
			target->CreateLinearGradientBrush(
				LinearGradientBrushProperties(c1, c2),
				gradcollect,
				&lbrush
			);
			target->FillRectangle(rect, lbrush);
			lbrush->Release();
		}
		
	}
	void End(){
		target->EndDraw();
	}
	~D2D() {
		if (factory)factory->Release();
		if (target)target->Release();
		if(brush)brush->Release();
		CoUninitialize();
	}
};
D2D* d2d;
UINT mycolor;
SolidBrush brush_fill, brush_line;
void Begin() {
	d2d->CreateSolidBrush(brush_fill,1.f,0.3f,0.4f);
	d2d->CreateSolidBrush(brush_line,1.f,0.f,0.f);
}
void render(){
	static int x = 0;
	mycolor--;
	
	d2d->Begin();
	d2d->Clear(1.f,1.f,1.f);
	d2d->DrawSquareBrush(Rect(0, 0, 100, 150),  brush_line, brush_fill, 3.f);
	d2d->DrawSquare(
		RectF(100, 100, 400, 400), //사각형 범위
		ColorF(0.f, 0.5f, 0.6f),	//브러쉬 색상
		ColorF((float)(GetRValue(mycolor))/256.f
			, (float)(GetGValue(mycolor)) / 256.f
			, (float)(GetBValue(mycolor))/ 256.f
		), 5.f
	);
	d2d->DrawGradSquare(
		RectF((float)x++, 300, 500, 700),
		ColorF(0.f, 0.5f, 0.6f),
		ColorF(0.3f, 1.f, 1.f),
		D2D::LineGradType::Left
	);
	d2d->DrawGradSquare(
		RectF(200, 0, 400, 200) ,
		ColorF(1.0f, 1.0f, 1.0f),
		ColorF(0.f, 0.f, 0.f),
		D2D::LineGradType::Left,
		true
	);
	d2d->DrawGradSquare(
		RectF(400, 0, 600, 200),
		ColorF(0.2f, 0.2f, 0.2f),
		ColorF(1.f, 1.f, 1.f),
		D2D::LineGradType::Left
	);
	if (x > 500)x = 0;
	

	d2d->End();
}
void Release() {
	brush_fill->Release();
	brush_line->Release();
}

class APIBrush {
	HBRUSH brush,oldbrush;
	HDC hdc;
public:
	APIBrush(HDC _hdc,UCHAR r,UCHAR g, UCHAR b) {
		hdc = _hdc;
		brush=CreateSolidBrush(RGB(r, g, b));
		oldbrush = (HBRUSH)SelectObject(hdc, brush);
	}
	APIBrush(HDC _hdc, UINT color) {
		hdc = _hdc;
		brush = CreateSolidBrush(color);
		oldbrush = (HBRUSH)SelectObject(hdc, brush);
	}
	~APIBrush() {
		SelectObject(hdc,oldbrush);
		DeleteObject(brush);
	}
};
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		
		PAINTSTRUCT ps;
		HDC hdc=BeginPaint(hwnd, &ps);
		APIBrush brush(hdc, mycolor);
		::Ellipse(hdc, 0, 0, 500, 500);
		EndPaint(hwnd, &ps);
	}
		break;
	case WM_LBUTTONDOWN:
	{
		CHOOSECOLOR col;
		COLORREF coltemp[16];
		memset(&col, 0, sizeof(CHOOSECOLOR));
		col.lStructSize = sizeof(CHOOSECOLOR);
		col.hwndOwner = hwnd;
		col.lpCustColors = coltemp;
		col.Flags = 0;
		if (ChooseColor(&col) != 0) {
			mycolor = col.rgbResult;
			//brush_fill->Release();
			brush_fill->SetColor(ColorF(GetRValue(mycolor) / 256.f, GetGValue(mycolor) / 256.f,
				GetBValue(mycolor) / 256.f));
			InvalidateRect(hwnd, 0, 0);
		}
	}break;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hin, HINSTANCE, char*, int) {
	d2d = new D2D;
	MSG msg = { 0 };

	WNDCLASSEX win = { 0 };
	win.cbSize = sizeof(WNDCLASSEX);
	win.style = CS_HREDRAW | CS_VREDRAW;
	win.lpfnWndProc = WndProc;
	win.hInstance = hin;
	win.lpszClassName = L"D2D";
	win.hCursor = LoadCursor(0, IDC_ARROW);
	RegisterClassEx(&win);
	
	RECT rect = { 0,0,800,600 };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, 0, WS_EX_OVERLAPPEDWINDOW);
	
	HWND hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"D2D", L"D2D", 
		WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX, 100, 100,
		rect.right - rect.left, rect.bottom - rect.top, 
		0, 0, hin, 0);

	if (!d2d->init(hwnd)) {
		MessageBox(hwnd, L"응 실패함", L"오류", MB_ICONERROR);
		return -1;
	}
	ShowWindow(hwnd, 1);
	Begin();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);
		else render();
	}
	Release();
	delete d2d;
	return (int)msg.wParam;
}
#endif
