
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <locale.h>
#include <conio.h>
#define TMP template<typename T>
#define VTMP template<>
#define MyMalloc(T,SZ) (T*)malloc(sizeof(T)*(SZ))
#define MyCalloc(T,SZ) (T*)calloc(SZ,sizeof(T))
#define MyRealloc(DAT,T,SZ) (T*)realloc(DAT,sizeof(T)*(SZ))

namespace MyStd {
	static wchar_t* ToUniStr(const char* buf, size_t len, bool isUTF8 = false) {
		size_t ulen = MultiByteToWideChar(isUTF8 ? CP_UTF8 : CP_ACP, 0, buf, len, 0, 0);
		wchar_t* ubuf = MyMalloc(wchar_t, ulen);
		MultiByteToWideChar(isUTF8 ? CP_UTF8 : CP_ACP, 0, buf, len, ubuf, ulen); return ubuf;
	}
	static char* ToMultiStr(const wchar_t* ubuf, size_t ulen, bool isUTF8 = false) {
		size_t len = WideCharToMultiByte(isUTF8 ? CP_UTF8 : CP_ACP, 0, ubuf, -1, 0, 0, 0, 0);
		char* buf = MyMalloc(char, len);
		WideCharToMultiByte(isUTF8 ? CP_UTF8 : CP_ACP, 0, ubuf, -1, buf, len, 0, 0);
		return buf;
	}
	static void SetInit() {
		setlocale(LC_ALL, "kor");
	}
	

	class MyParent {
	public:
		virtual ~MyParent() {}
	};

	TMP class MyContain :public MyParent {
	protected:
		T* _v;
		size_t _size;//_size는 stack에선 건드리지 말아야 됨.
		size_t _capa;//용량
		bool _isDyn,//동적여부 (잘 안 씀)
			_isDouble,//재할당 2배
			_isObj//new 사용한 객체여부
			;
		inline void _Alloc(size_t sz = 4u, bool isD = true, bool isO = true,bool isStr=false) {
			_isObj = isO;
			_isDouble = isD;
			if (_isDouble) {
				_capa = 4u;
				_ExpandTo(sz);
			}
			else _capa = sz;
			if (isStr)_size = sz;
			_v = MyCalloc(T, sz + (size_t)isStr);
		}
		inline bool _Expand(size_t expSize) {
			return _ExpandTo(expSize + _size);
		}
		inline bool _ExpandTo(size_t toSize) {
			size_t tc;
			bool flag = false;
			for (tc = _capa; tc < toSize; tc <<= 1) {
				flag = true;
			}
			if (flag)printf("EXPAND : %d\n", tc);
			_capa = tc;
			return flag;
		}
		void _Realloc() {
			if (_isObj)_ReallocObj();
			else this->_v = MyRealloc(this->_v, T, this->_capa);
		}
		void _ReallocObj() {
			auto buf = new T[this->_capa];
			/*for (size_t s = 0u; s < this->_size; s++) {
				buf[s] = this->_v[s];
			}
			this->_v = buf;*/
			memcpy(buf, _v, sizeof(T) * this->_capa);
			memset(_v, 0, sizeof(T) * this->_capa);
			delete _v;
			this->_v = buf;
		}
	public:
		MyContain() {
			puts("Contain");
		}

		const T& operator[](size_t index) const {
			//if (index < _size)
			return this->_v[index];

		}
		T& operator[](size_t index) {
			//if (index < _size)
			return this->_v[index];
		}
		inline T* End() { return this->_v + this->_size; }
		inline T* Get() { return _v; }
		inline size_t Size() { return _size; }
		inline size_t Capa() { return _capa; }
		inline T& At(size_t index) {
			return this->_v[index];
		}
		inline T& At(size_t index) const {
			return this->_v[index];
		}
		inline void Add(const T& t) {
			if (this->_size + 1 >= this->_capa)_Expand(1);
			this->_v[this->_size++] = t;
		}
		inline void AddSize(size_t sz) {
			if (_Expand(this->capa, sz))_Realloc();
			this->_size += sz;
		}
		inline void SetSize(size_t sz) {
			if (_ExpandTo(this->capa, sz))_Realloc();
			this->_size = sz;
		}
		void PrintInfo() {
			printf("(%s) CAPA : %d SIZE : %d\n", typeid(this).name(), Capa(), Size());
		}
		virtual ~MyContain() { puts("~Contain"); }


		class Iter {
			T* _beg, * _end, * p;
		public:
			Iter(MyContain<T>& f) {
				_beg = f.Get();
				_end = f.End();
				p = _beg;
			}
			Iter& operator++() {
				if (p < _end)p++;
				return *this;
			}
			Iter& operator--() {
				if (p >= _beg)p--;
				return *this;
			}
			void ToBegin() { p = _beg; }
			void ToEnd() { p = _end - 1; }
			wchar_t operator*() {
				return *p;
			}
			bool IsAble() {
				return p >= _beg && p < _end;
			}
		};
	};
	
#define BASE MyContain<T>
	class MyMultiStr : public MyContain<char> {
	public:
		MyMultiStr(size_t capa = 4u, bool isD = false) {
			_Alloc(capa, isD, false);
		}
		MyMultiStr(const char* str, bool isD = false) {
			_Alloc(strlen(str)+1, isD, false, true);
			strcpy(_v, str);
		}
		void Print() {
			puts(_v);
		}
		virtual ~MyMultiStr() {
			if (_v) { free(_v); _v = 0; }
			puts("~multistr");
		}
	};
	class MyUniStr :public MyContain<wchar_t> {
	protected:
	public:
		
		MyUniStr(size_t sz = 4u, bool isD = false) {
			_Alloc(sz, true,false,true); 
		}
		MyUniStr(const wchar_t* wstr, bool isD = false) {
			_Alloc(wcslen(wstr) + 1, true,false,true);
			wcscpy(this->_v, wstr);
		}


		

		void Print() {
			_putws(_v);
		}
		virtual ~MyUniStr() {
			if (_v) { free(_v); _v = 0; }
			puts("~unistr");
		}
	};
	TMP class MyArray : public MyContain<T> {
	protected:
	public:
		//capa:용량 isDynamic:가변/동적 isDouble:2배할당 isObj:객체
		MyArray(size_t capa = 4U, bool isDynamic = true, bool isDouble = true, bool isObj = true) {
			BASE::_Alloc(capa, isDouble, isObj);
		}
		virtual ~MyArray() {
			if (this->_v) {
				if (this->_isObj) { delete[] this->_v; }
				else free(this->_v);
				this->_v = 0;
			}
		}
	};
	TMP class MyObjArray : public MyContain<T> {
	protected:

	public:
		//capa:용량 isDynamic:가변/동적 
		MyObjArray(size_t capa = 4u, bool isDynamic = true) {
			BASE::_Alloc(capa);
		}
		virtual ~MyObjArray() {
			if (this->_v) {
				for (auto p = this->_v; p != this->End(); p++)
					if (*p != 0) {
						delete* p;
						*p = 0;
					}

				puts("~ObjArray");
			}
		}

	};

	class MyFile :public MyParent {
	protected:
		HANDLE _hfile;
	public:
		
		MyFile() {
			puts("MyFile");
		}
		~MyFile() {
			puts("~myfile");
		}
	};

#undef BASE
	class AutoRelease {
		MyObjArray<MyParent*>* obj;
	public:
		AutoRelease() { obj = new MyObjArray<MyParent*>(4u); }
		void Add(MyParent* o) {
			obj->Add(o);
		}
		~AutoRelease() {
			delete obj;
		}
	};

};
