#pragma once
#include <Windows.h>
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
	void Create(HWND hwnd, int x, int y, int id) {
		hcombo = CreateWindow(
			L"combobox", 0, WS_CHILD | WS_VISIBLE
			| CBS_DROPDOWNLIST | CBS_AUTOHSCROLL |
			WS_VSCROLL
			, x, y, 100, 500, hwnd, (HMENU)id, 0, 0
		);
	}
	void Add(const TCHAR* lists) {
		SendMessage(hcombo, CB_ADDSTRING, 0, (LPARAM)lists);
	}
	void Process(WPARAM wp)
	{
		int idx;
		switch (HIWORD(wp)) {
		case CBN_SELCHANGE:
			idx = (int)SendMessage(hcombo, CB_GETCURSEL, 0, 0);
			break;
			//case CBN_EDITCHANGE:break;
		}
	}
	inline int GetIndex() {
		return SendMessage(hcombo, CB_GETCURSEL, 0, 0);
	}
	inline int SetIndex(int idx) {
		return SendMessage(hcombo, CB_SETCURSEL, idx, 0);
	}
};
class CRadioButton {
public:
	HWND hcheck;
	bool ischeck;
	void Create(HWND hwnd, UINT id, const TCHAR* name, int x, int y, bool mainbtn = false, bool check = true) {
		ischeck = check;
		hcheck = CreateWindow(
			TEXT("button"),
			name,
			(mainbtn ? WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP
				: WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON)
			,
			x, y, 100, 25,
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
	void Process() {
		ischeck = (SendMessage(hcheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
		/*switch (HIWORD(wp)) {
		case BN_CLICKED:
			if (SendMessage(hcheck, BM_GETCHECK, 0, 0) == BST_CHECKED) {
				Set(false);
			}
			else {
				Set(true);
			}
			break;
		}*/
	}
};
