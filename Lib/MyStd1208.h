
#pragma once
#define _CRT_SECURE_NO_WARNINGS
//#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <conio.h>
#define TMP template<typename T>
#define VTMP template<>
#define MyMalloc(T,SZ) (T*)malloc(sizeof(T)*(SZ))
#define MyCalloc(T,SZ) (T*)calloc(SZ,sizeof(T))
#define MyRealloc(DAT,T,SZ) (T*)realloc(DAT,sizeof(T)*(SZ))

namespace MyStd {

	TMP struct MyContain
	{
		T* _v;
		struct {
			size_t len : 24;
			size_t capa : 8;
		};
		MyContain() {
			len = capa = 0u;
		}
		MyContain(T* vt, size_t _len) {
			len = capa = 0u;
			_v = vt;
			len = _len;
			capa = _len;
		}
		void Add(const T& t) {
			_v[len++] = t;
		}
		T& At(size_t idx) {
			return _v[idx];
		}
		~MyContain() {
			if (_v) {
				delete _v;
				_v = 0;
			}
		}
	};
	TMP class MyVec {
	protected:
		MyContain<T>* _p;
	public:
		MyVec() {
			_p = 0;
		}
		MyVec(MyVec& val) {
			this->_p = val._p;
			val._p = 0;
		}
		MyVec(MyContain<T>* other) {
			_p = other;
		}
		virtual ~MyVec() {
			if (_p) {
				delete _p;
				_p = 0;
			}
		}
		T& operator[](size_t index) {
			return _p->At(index);
		}
		virtual void Print() {
		}
		class Iter {
			MyContain<T>* _p;
			T* _ptr;
			T* Begin() {
				return _p->_v;
			}
			T* End() {
				return _p->_v + _p->len;
			}
		public:
			Iter(MyVec<T>& vec) {
				_p = vec._p;
				_ptr = _p->_v;
			}
			Iter(MyContain<T>* vec) {
				_p = vec;
				_ptr = _p->_v;
			}
			Iter(const Iter& it) {
				_p = it._p;
				_ptr = _p->_v;
			}
			Iter& operator++() {
				if (_ptr < _p->_v + _p->len)_ptr++;
				return *this;
			}
			Iter& operator--() {
				if (_ptr >= _p->_v)_ptr--;
				return *this;
			}
			void ToBegin() { _ptr = _p->_v; }
			void ToEnd() { _ptr = _p->_v + _p->len - 1; }
			T& operator*() {
				return *_ptr;
			}
			bool IsAble() {
				return _ptr >= (_p->_v) && _ptr < (_p->_v + _p->len);
			}
		};
		Iter ToIter() {
			return Iter(this->_p);
		}
	};
	class MyMulStr :public MyVec<char> {
	public:
		MyMulStr(const char* _str) {
			char* str = new char[strlen(_str) + 1];
			strcpy(str, _str);
			_p = 0;
			_p = new MyContain<char>(str, strlen(_str));
		}
		void Print() {
			puts(_p->_v);
		}

	};

	class MyUniStr :public MyVec<wchar_t> {
	public:
		MyUniStr(const wchar_t* _wstr) {
			wchar_t* wstr = new wchar_t[wcslen(_wstr) + 1];
			wcscpy(wstr, _wstr);
			_p = 0;
			_p = new MyContain<wchar_t>(wstr, wcslen(_wstr));
		}
		void Print() {
			_putws(_p->_v);
		}

	};



	TMP static void Std_Free(T& t) {
		if (t) {
			free(t);
			t = 0;
		}
	}
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


	class MyFile {
	public:
		enum class FileType {
			Create = CREATE_ALWAYS,
			Open = OPEN_ALWAYS,
		};
	private:
		FileType _ftype;
		HANDLE _hfile;
		wchar_t* _fdir;
		void* _fdata;
		size_t _fsize;
	public:
		MyFile(const wchar_t* dir, FileType ftype = FileType::Open) {

			_ftype = ftype;
			_fdata = 0;
			_fsize = 0u;

			size_t len = wcslen(dir);
			_fdir = MyMalloc(wchar_t, len + 1);
			memcpy(_fdir, dir, sizeof(wchar_t) * len + 1);

			_fdir[len] = 0;
			_hfile = CreateFile(_fdir, GENERIC_ALL, 0, 0,
				(DWORD)_ftype, FILE_ATTRIBUTE_NORMAL, 0);
			if (_hfile == INVALID_HANDLE_VALUE)_hfile = 0;
		}
		TMP void Write(T* dat, size_t sz) {
			::WriteFile(_hfile, dat, sizeof(T) * sz, 0, 0);
			_fsize = sizeof(T) * sz;
		}
		TMP void WriteUtf8(T* dat, size_t sz) {
			::WriteFile(_hfile, dat, sizeof(T) * sz, 0, 0);
			_fsize = sizeof(T) * sz;
		}
		VTMP void WriteUtf8(wchar_t* dat, size_t sz) {
			size_t len;
			auto buf = ToMultiStr(dat, len, true);
			Write(buf, len);
			free(buf);
		}
		void ReadAll() {
			size_t size = ::GetFileSize(_hfile, 0);
			if (_fdata)Std_Free(_fdata);
			_fdata = MyMalloc(char, size);
			_fsize = size;
			::ReadFile(_hfile, _fdata, size, 0, 0);
		}
		TMP const T* GetData() {
			return (T*)_fdata;
		}
		const size_t FileSize() {
			return _fsize;
		}
		~MyFile() {
			::free(_fdir);
			Std_Free(_fdata);
		}
	};

#undef BASE
};
