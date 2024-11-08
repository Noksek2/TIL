#include <iostream>
#include <Windows.h>
#include <locale.h>
#define TMP template<typename T>
#define VTMP template<>
#define MyMalloc(T,SZ) (T*)malloc(sizeof(T)*(SZ))
#define MyRealloc(DAT,T,SZ) (T*)realloc(DAT,sizeof(T)*(SZ))

namespace MyStd {
	template<typename T=wchar_t> class MyString {
	private:
		void _Alloc(size_t len) {
			_len = len;
			_buf = MyMalloc(T, _len);
		}
		void _CopyStr(const T* cs, size_t len) {
			_Alloc(len);
			memcpy(_buf, cs, sizeof(T) * _len);
		}
		void _Expand(size_t target_len) {
			T* _rebuf = MyRealloc(_buf, T, target_len);
			if (_rebuf) {
				_buf = _rebuf;
				_len = target_len;
			}
		}
	public:
		T* _buf;
		size_t _len;
		MyString(size_t len = 0) { _Alloc(len); }
		MyString(const T* cs, size_t len = 0) {
			if (len == 0)len = wcslen(cs);
			_CopyStr(cs, len);
		}
		MyString(const MyString<T>& ms) {
			_CopyStr(ms._buf, ms._len);
		}
		void Print() {
			_putws(_buf);
		}
		~MyString() {
			if (_buf) {
				free(_buf); _buf = 0; _len = 0;
			}
		}
	};
	VTMP class MyString<char>{
	private:
		void _Alloc(size_t len) {
			_len = len;
			_buf = MyMalloc(char, _len);
		}
		void _CopyStr(const char* cs, size_t len) {
			_Alloc(len);
			memcpy(_buf, cs, sizeof(char) * _len);
		}
		void _Expand(size_t target_len) {
			char* _rebuf = MyRealloc(_buf, char, target_len);
			if (_rebuf) {
				_buf = _rebuf;
				_len = target_len;
			}
		}
	public:
		char* _buf;
		size_t _len;
		MyString(size_t len = 0) { _Alloc(len); }
		MyString(const char* cs, size_t len = 0) {
			if (len == 0)len = strlen(cs);
			_CopyStr(cs, len);
		}
		MyString(const MyString<char>& ms) {
			_CopyStr(ms._buf, ms._len);
		}
		void Print() {
			puts(_buf);
		}
		~MyString() {
			if (_buf) {
				free(_buf); _buf = 0; _len = 0;
			}
		}
	};
	static void SetInit() {
		setlocale(LC_ALL, "kor");
	}
	wchar_t* ToUniStr(const char* buf,size_t len,bool isUTF8=false) {
		size_t ulen = MultiByteToWideChar(isUTF8 ? CP_UTF8 : CP_ACP, 0, buf, len, 0, 0);

		wchar_t* ubuf=MyMalloc(wchar_t,ulen);
		MultiByteToWideChar(isUTF8 ? CP_UTF8 : CP_ACP, 0, buf, len, ubuf, ulen);
		return ubuf;
	}
	char* ToMultiStr(const wchar_t* ubuf, size_t ulen, bool isUTF8 = false) {
		size_t len = WideCharToMultiByte(isUTF8 ? CP_UTF8 : CP_ACP, 0, ubuf, -1, 0, 0, 0, 0);
		char* buf=MyMalloc(char, len);
		WideCharToMultiByte(isUTF8 ? CP_UTF8 : CP_ACP, 0, ubuf, -1, buf, len, 0, 0);
		return buf;
	}
	class MyFile {
	private:
		HANDLE _hfile;
	public:
		MyFile(){}
	};

	TMP class MyArray {
	public:
	private:
		T* _v;
		size_t _size, _capa;
		bool _isBasic;
		bool _isDyn;
		void _Expand() {
		}
	public:
		inline const size_t Size() {
			return _size;
		}
		MyArray(size_t sz=4U, bool isDynamic = true, bool isBasic = false){
			_v = 0; _size = _capa = 0;
			_isDyn=isDynamic;
			_isBasic = isBasic; 
			if(!_isDyn){
				if (_isBasic)_v = MyMalloc(T, _size);
				else _v = new T[_size];

				_size = _capa=sz;
			}
			else{
				_size = sz;
				_capa = _size;
			}
		}
		const T operator[](size_t index) const {
			if (index < _size)return _v[index];
			return nullptr;
		}
		T& operator[](size_t index) {
			if (index < _size)return _v[index];
		}
		~MyArray() {
			if (_v) {
				if (_isBasic) {
					free(_v);
					_v = 0;
				}
				else {
					
				}
			}delete[] _v;
			_v = 0;
		}
		
	};
};
using namespace MyStd;
int main() {
	char* s = "sdfa";
	s = "asfsf";
	SetInit();
	MyString<wchar_t>mystr(L"뷁둥둥");
	mystr.Print();

	MyString<char>mstr("귁귁analdkf");
		MyString<char> mstr2(mstr);
		mstr2.Print();
	mstr.Print();

	MyArray<int>ints(30,false,false);
	for (size_t i = 0; i < ints.Size(); i++) {
		ints[i] = i+1;
		printf("%d번 ", ints[i]);
	}
	
	return 0;
}
