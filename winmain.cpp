
#include "apiclass.h"
//#include <vector>
LRESULT CALLBACK MainWinProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("APIDRAW");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = MainWinProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

enum{
	ID_MENU1=1,
	ID_MENU2=2,
	ID_MENU3,
	ID_SCROLL,
	ID_SCROLL1,
	ID_SCROLL2,
	ID_EDIT1,
	ID_TOOLBAR,
	ID_BTN1,
	ID_OWNBTN,
	ID_FONT,
	//ID_BTN2,
};
void CWindow::Create(HWND hwnd){
	lf.lfHeight=20;
	lf.lfFaceName[0]=L'굴';
	lf.lfFaceName[1]=L'림';
	//wcscpy(lf.lfFaceName,L"굴림");
	//lf.lfFaceName[1]=L'림';
	g_hin=(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
	HWND mybtn=CreateWindow(L"button",L"",
		WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,
		100,150,100,100,hwnd,(HMENU)ID_OWNBTN,g_hin,0);
	
	HWND edit= CreateWindow(L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
			ES_AUTOHSCROLL, 100, 100, 200, 25, hwnd, (HMENU)ID_EDIT1, g_hin, NULL);
	HWND btn1= CreateWindow(L"button", L"파일 열기", WS_CHILD | 
		WS_VISIBLE |BS_PUSHBUTTON, 
		250, 50, 100, 50, hwnd, (HMENU)ID_BTN1, g_hin, NULL);
	HWND btn2= CreateWindow(L"button", L"색상 변경", WS_CHILD | 
		WS_VISIBLE |BS_PUSHBUTTON, 
		250, 00, 100, 50, hwnd, (HMENU)ID_MENU3, g_hin, NULL);
	
	CreateWindow(L"button", L"폰트 변경", WS_CHILD | 
		WS_VISIBLE |BS_PUSHBUTTON, 
		400, 00, 100, 50, hwnd, (HMENU)ID_FONT, g_hin, NULL);
	
	HMENU menu=CreateMenu(),mainmenu=CreateMenu();
	AppendMenu(menu,MF_STRING,ID_MENU1,L"I Love API 하악하악 너무좋아");
	AppendMenu(menu,MF_STRING,ID_MENU3,L"색상");
	AppendMenu(menu,MF_SEPARATOR,0,0);
	AppendMenu(menu,MF_STRING,ID_MENU2,L"닫아 시발");
	AppendMenu(mainmenu,MF_POPUP,(UINT_PTR)menu,L"쓸데없는거");
	SetMenu(hwnd,mainmenu);

	scroll.Create(hwnd,g_hin,ID_SCROLL,0,0);
	scroll1.Create(hwnd,g_hin,ID_SCROLL1,0,20,10);
	scroll2.Create(hwnd,g_hin,ID_SCROLL2,0,40);
}

void CWindow::Paint(HWND hwnd){
	PAINTSTRUCT ps;
	HDC hdc=BeginPaint(hwnd,&ps);
		//SetMapMode(hdc,MM_LOENGLISH);
		//SetViewportOrgEx(hdc,200,150,NULL);
	CBrush brush(hdc,color);
	CPen pen(hdc,0,scroll1.pos);
	Ellipse(hdc,0+scroll2.pos,0,scroll.pos+scroll2.pos,scroll.pos);
	CFont font(hdc,&lf);
	SetTextColor(hdc,fontcol);
	SetBkMode(hdc,TRANSPARENT);
	TextOut(hdc,50,200,L"그걸 자랑이라고 말한다",12);
	
	RECT rect;
	GetClientRect(hwnd,&rect);
	DrawText(hdc,L"I love youI love youI love youI lo"
		L"ve youI love youI love youI love youI love"
		L"youI love youI love youI love youI love yo"
		L"uI love youI love youI love youI love youI love youI love"
		L"youI love youI love youI love youI love youI love youI lov"
		L"e youI love youI love you",-1,
		&rect,DT_BOTTOM|DT_WORDBREAK);
	EndPaint(hwnd,&ps);
}
void CWindow::Hscroll(HWND hwnd,WPARAM wp,LPARAM lp){
	if((HWND)lp==scroll.hscroll)scroll.Process(wp,lp);
	else if((HWND)lp==scroll1.hscroll)scroll1.Process(wp,lp);
	else if((HWND)lp==scroll2.hscroll)scroll2.Process(wp,lp);
	else return ;
	//scroll1.Process(wp,lp);
	RECT rt={0,0,800,300};
	InvalidateRect(hwnd,&rt,true);
}
void CWindow::Command(HWND hwnd,WPARAM wp,LPARAM lp){
	switch(LOWORD(wp)){
	case ID_FONT:
		{
			CHOOSEFONT fnt={0};
			fnt.lStructSize=sizeof(CHOOSEFONT);
			fnt.hwndOwner=hwnd;
			fnt.lpLogFont=&lf;
			fnt.Flags=CF_EFFECTS|CF_SCREENFONTS;
			if(ChooseFont(&fnt)){
				fontcol=fnt.rgbColors;
				InvalidateRect(hwnd,0,true);
			}
		}
		break;
	case ID_BTN1:{
		OPENFILENAME ofn={0};
		WCHAR filename[MAX_PATH]=L"";
		ofn.lStructSize=sizeof(OPENFILENAME);
		ofn.hwndOwner=hwnd;
		ofn.lpstrFilter=L"모든 파일(*.*)\0*.*\0텍스트 파일\0*.txt;*.doc\0";
		ofn.nMaxFile=256;
		ofn.lpstrFile=filename;
		ofn.lpstrInitialDir=L"c:\\";
		if(GetOpenFileName(&ofn)!=0){
			MessageBox(hwnd,filename,L"열렸나?",MB_OK);
		}
		}
		break;
	case ID_MENU1:
		MessageBox(hwnd,L"아 뭐야",L"짜증나",MB_OK);
		break;
	case ID_MENU3:
		{
			CHOOSECOLOR col={0};
			COLORREF temp[16];
			col.lStructSize=sizeof(CHOOSECOLOR);
			col.hwndOwner=hwnd;
			col.lpCustColors=temp;
			if(ChooseColor(&col)!=0){
				color=col.rgbResult;
				InvalidateRect(hwnd,0,true);
			}
		}
		break;
	case ID_MENU2:
		SendMessage(hwnd,WM_DESTROY,0,0);
		break;
	}
}
LRESULT CALLBACK MainWinProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	
	static CWindow window;
	switch (iMessage) {
	case WM_CTLCOLORBTN:
		SetTextColor((HDC)wParam,RGB(100,100,255));
		SetBkColor((HDC)wParam,RGB(100,255,100));
		return (LRESULT)CreateSolidBrush(RGB(0,0,30));
	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdraw=(LPDRAWITEMSTRUCT)lParam;
		if(lpdraw->itemState&ODS_SELECTED){
			TextOut(lpdraw->hDC,2,30,L"헤으윽 오니짱",7);
		}
		else TextOut(lpdraw->hDC,2,2,L"뷁",1);
	}
		break;
	case WM_CREATE:	
	{/*InitCommonControls();
	TBBUTTON ToolBtn[1]={{
		STD_FILENEW,10,
			TBSTATE_ENABLED,TBSTYLE_BUTTON,
		0,0,0,0
	}};
	HWND hTool=CreateToolbarEx(hwnd,WS_CHILD|WS_VISIBLE|WS_BORDER,
		ID_TOOLBAR,1,HINST_COMMCTRL,
		IDB_STD_SMALL_COLOR,ToolBtn,20,
		16,16,16,16,sizeof(TBBUTTON));
	*/
	window.Create(hwnd);}break;
	case WM_PAINT:	window.Paint(hwnd);break;
	case WM_COMMAND:window.Command(hwnd,wParam,lParam);break;
	case WM_HSCROLL:window.Hscroll(hwnd,wParam,lParam);break;
	case WM_LBUTTONDOWN:window.Lbtndown(hwnd,wParam,lParam);break;
	case WM_MOUSEWHEEL:
		window.Mousewheel(hwnd,wParam,lParam);
		break;
	case WM_RBUTTONUP:
		{
			HMENU hpop=CreatePopupMenu();
		POINT point = { LOWORD(lParam) ,HIWORD(lParam) };

		ClientToScreen(hwnd, &point);
		AppendMenu(hpop, MF_STRING, ID_FONT, L"폰트");
		AppendMenu(hpop, MF_STRING, ID_MENU3, L"색상");
		AppendMenu(hpop, MF_SEPARATOR, 0, 0);
		AppendMenu(hpop, MF_STRING, ID_MENU2, L"QUIT");
		TrackPopupMenu(hpop, TPM_RIGHTBUTTON, point.x, point.y,
			0, hwnd, 0);
		DestroyMenu(hpop);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,iMessage,wParam,lParam);
}

#ifdef PROC1
enum {
	BTN1=1,
	BTN2=2,
	MENU_COLOR,
	BTN_EDIT,
	RADIO1,
	RADIO2,
	RADIO3,
	EDIT,
	LISTBOX1,
	COMBO1,
	SCROLL1,
	MENU_END=100,
};
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static UCHAR style = PS_SOLID;
	static USHORT point_len=0;
	static bool draw=false,check=false;
	static CPoint points[256*256];
	static HWND hbutton[5],hedit,hlist,hcombo,hscroll;
	static HMENU hmenu,hmenubar;
	static LOGFONT logfont;
	static DWORD color=0;
	switch (iMessage) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:
	{
		auto hin = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

		hedit= CreateWindow(L"edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
			ES_AUTOHSCROLL, 300, 100, 200, 25, hwnd, (HMENU)EDIT, hin, NULL);
		hbutton[0] = CreateWindow(
			L"button", L"버튼1",
			WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 100, 100, 200, 50, hwnd,
			(HMENU)BTN1,
			hin, 0);
		hbutton[1] = CreateWindow(
			L"button", L"버튼2",
			WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 100, 200, 200, 50, hwnd,
			(HMENU)BTN2,
			hin, 0);

		CreateWindow(L"button", L"ANG", WS_CHILD | WS_VISIBLE |
			BS_GROUPBOX,
			100, 0, 200, 150, hwnd, 0, hin, NULL);

		hbutton[2]= CreateWindow(L"button", L"Black", WS_CHILD | WS_VISIBLE |
			BS_AUTORADIOBUTTON | WS_GROUP,
			150, 20, 100, 30, hwnd, (HMENU)RADIO1,hin, NULL);
		hbutton[2] = CreateWindow(L"button", L"Red", WS_CHILD | WS_VISIBLE |
			BS_AUTORADIOBUTTON,
			150, 50, 100, 30, hwnd, (HMENU)RADIO2, hin, NULL);
		hbutton[2] = CreateWindow(L"button", L"Blue", WS_CHILD | WS_VISIBLE |
			BS_AUTORADIOBUTTON|WS_GROUP,
			150, 80, 100, 30, hwnd, (HMENU)RADIO3, hin, NULL);

		hbutton[2] = CreateWindow(L"button", L"ADD STRING", WS_CHILD | WS_VISIBLE |
			BS_PUSHBUTTON,
			350, 80, 100, 30, hwnd, (HMENU)BTN_EDIT, hin, NULL);
		hlist= CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
			LBS_NOTIFY, 500, 100, 100, 200, hwnd, (HMENU)LISTBOX1, hin, NULL);

		hcombo= CreateWindow(L"combobox", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN
			, 400, 200, 100, 200, hwnd, (HMENU)COMBO1, hin, NULL);

		hscroll = CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ
			, 10, 500, 200, 20, hwnd, (HMENU)SCROLL1, hin, NULL);
		SetScrollRange(hscroll,SB_CTL,0,255,true);
		SetScrollPos(hscroll, SB_CTL, 0, true);



		hmenu = CreateMenu();
		hmenubar = CreateMenu();
		AppendMenu(hmenu, MF_STRING, 1, L"뷁1");
		AppendMenu(hmenu, MF_STRING, 2, L"폰트");
		AppendMenu(hmenu, MF_STRING, MENU_COLOR, L"색상");

		AppendMenu(hmenu, MF_STRING, MENU_END, L"프로그램 종료");

		AppendMenu(hmenubar, MF_POPUP, (UINT_PTR)hmenu, L"뷁3");

		SetMenu(hwnd, hmenubar);
	}
		return 0;
	case WM_HSCROLL:
		if ((HWND)lParam == hscroll);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		CPen cpen(hdc, RGB(180, 50, 50), 1,style);
		CBrush cbrush(hdc, color);
		CFont cfont(hdc, &logfont);
		
		WCHAR buf[100] = L"";
		MoveToEx(hdc, points[0].x, points[0].y, 0);
		for (int i = 1;i < point_len;i++) {
			wsprintf(buf, L"point[%d]=(%d, %d)", i, points[i].x, points[i].y);
			TextOut(hdc, 0, i*(-logfont.lfHeight), buf, wcslen(buf));
			LineTo(hdc, points[i].x, points[i].y);
			
		}
		if(check)Ellipse(hdc, 10, 10, 300, 300);
		
		EndPaint(hwnd, &ps);
	}
	return 0;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case EDIT:
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
				/*WCHAR buf[128];
				GetWindowText(hedit,buf,128);
				SetWindowText(hwnd, buf);*/
				break;
			}
			break;
		case COMBO1: {
			WCHAR buf[128];
			switch (HIWORD(wParam)) {

			case CBN_SELCHANGE: {

				int i = SendMessage(hcombo, CB_GETCURSEL, 0, 0);
				//GetWindowText(hedit, buf, 128);
				SendMessage(hcombo, CB_GETLBTEXT, i, (LPARAM)buf);
				SetWindowText(hwnd, buf);
			}break;
			case CBN_EDITCHANGE:
				GetWindowText(hcombo, buf, 128);
				SetWindowText(hwnd, buf);
				break;
			}
			break;
		}
		case LISTBOX1:

			switch (HIWORD(wParam)) {
			case LBN_SETFOCUS:
				break;
			}
			break;
		case BTN_EDIT:
			WCHAR buf[128];
			GetWindowText(hedit, buf, 128);
			//SetWindowText(hwnd, buf);
			SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)buf);
			SendMessage(hcombo, CB_ADDSTRING, 0, (LPARAM)buf);
			break;
		case BTN1:
			if (SendMessage(hbutton[1], BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
				SendMessage(hbutton[1], BM_SETCHECK, BST_CHECKED, 0);
				check = true;
			}
			else {
				SendMessage(hbutton[1], BM_SETCHECK, BST_UNCHECKED, 0);
				check = false;
			}
			InvalidateRect(hwnd, 0, true);
			break;
		case BTN2:
		{
			CHOOSEFONT cft = { 0 };
			cft.lStructSize = sizeof(CHOOSEFONT);
			cft.hwndOwner = hwnd;
			cft.lpLogFont = &logfont;
			cft.Flags = CF_EFFECTS | CF_SCREENFONTS;
			if(ChooseFont(&cft)){
				InvalidateRect(hwnd, 0, true);
			}

		}
		break;
		case MENU_COLOR:
		{
			CHOOSECOLOR col = { 0 };
			COLORREF coltemp[16];
			col.lStructSize = sizeof(CHOOSECOLOR);
			col.hwndOwner = hwnd;;
			col.lpCustColors = coltemp;
			if (ChooseColor(&col) != 0) {
				color = col.rgbResult;
				InvalidateRect(hwnd, 0, true);
			}
		}
			break;
		case MENU_END:
		{

			if(MessageBox(hwnd,L"끌거?",L"ㅇㄴ",MB_ICONQUESTION|MB_YESNO)==IDYES)
				SendMessage(hwnd, WM_DESTROY, 0, 0);
		}break;
		}
	}
		return 0;
	case WM_RBUTTONUP:
	{
		;HIWORD(lParam);
		HMENU hpop=CreatePopupMenu();
		POINT point = { LOWORD(lParam) ,HIWORD(lParam) };

		ClientToScreen(hwnd, &point);
		AppendMenu(hpop, MF_STRING, BTN2, L"폰트");
		AppendMenu(hpop, MF_STRING, MENU_COLOR, L"색상");
		AppendMenu(hpop, MF_SEPARATOR, 0, 0);
		AppendMenu(hpop, MF_STRING, MENU_END, L"QUIT");
		TrackPopupMenu(hpop, TPM_RIGHTBUTTON, point.x, point.y,
			0, hwnd, 0);
		DestroyMenu(hpop);
	}
		break;
	case WM_LBUTTONDOWN:
	{
		//HDC hdc = GetDC(hwnd);
		draw = true;
		//point_len = 0;
		points[point_len].y=HIWORD(lParam);
		points[point_len++].x=LOWORD(lParam);
		
		//MoveWindow(hedit, LOWORD(lParam), HIWORD(lParam), 200, 50, true);
		//ReleaseDC(hwnd, hdc);
	}break;
	case WM_MOUSEMOVE:
	{
		if (draw) {
			HDC hdc = GetDC(hwnd);
			points[point_len].y=HIWORD(lParam);
			points[point_len++].x=LOWORD(lParam);
			ReleaseDC(hwnd, hdc);
			InvalidateRect(hwnd, nullptr, true);
		}
	}break;
	case WM_LBUTTONUP:
		draw = false;
		break;
	}
	return(DefWindowProc(hwnd, iMessage, wParam, lParam));
}
#endif
/*
* menu ; https://zetcode.com/gui/winapi/menus/
*/
