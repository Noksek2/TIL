#include <Windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")
using namespace D2D1;
typedef ID2D1Factory D2DFactory;
typedef ID2D1HwndRenderTarget D2DRender;
typedef ID2D1SolidColorBrush SolidBrush;
typedef D2D1_POINT_2F POINT_2F;
class D2D {
private:
	D2DFactory* factory;
	D2DRender* render;
	SolidBrush* solidbrush;
public:
	D2D() {
		factory = 0;
		render = 0;
		solidbrush = 0;
	}
	~D2D() {
		if (factory)factory->Release();
		if(render)render->Release();
		if(solidbrush)solidbrush->Release();
		CoUninitialize();
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
			&render);
		return 1;
	}
	void Begin() {
		render->BeginDraw();
	}
	void Clear(float r,float g,float b) {
		render->Clear(ColorF(r, g, b));
	}
	void End() {
		render->EndDraw();
	}
};
class MainGame {
public:
	D2D* d2d;
	MainGame(HWND hwnd) {
		d2d = new D2D();
		if (!d2d->init(hwnd)) {
			MessageBox(hwnd, L"응 실패함", L"오류", MB_ICONERROR);
		}
	}
	~MainGame() {
		if (d2d) {
			delete d2d; d2d = nullptr;
		}
	}
	void Render() {
		d2d->Begin();
		d2d->Clear(1.f,0.5f,1.f);
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
		EndPaint(hwnd,&ps);
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
	HWND hWnd=CreateAdjustWindow(hInstance);
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
