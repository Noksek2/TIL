#include <Windows.h>
LRESULT CALLBACK WndProc(HWND hwnd,UINT msg, WPARAM wp,LPARAM lp){
	switch (msg) {
	case WM_PAINT:

		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}
int WINAPI WinMain(HINSTANCE hin, HINSTANCE, LPSTR, int) {
	WNDCLASS win = { 0 };
	win.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win.hInstance = hin;
	win.hCursor = LoadCursor(NULL, IDC_ARROW);
	win.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win.lpfnWndProc = WndProc;
	win.lpszClassName = L"class";
	win.lpszMenuName = NULL;
	win.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&win);
	HWND hwnd=CreateWindow(
		L"class",L"title",WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,800,600,0,0,hin,0
	);
	ShowWindow(hwnd, true);

	MSG msg = { 0 };
	while (GetMessage(&msg, hwnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
