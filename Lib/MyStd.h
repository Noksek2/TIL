#include <iostream>
#include <Windows.h>
#include <locale.h>
#include <conio.h>
#define TMP template<typename T>
#define VTMP template<>
#define MyMalloc(T,SZ) (T*)malloc(sizeof(T)*(SZ))
#define MyRealloc(DAT,T,SZ) (T*)realloc(DAT,sizeof(T)*(SZ))

namespace MyStd {
	static void SetInit() {
		setlocale(LC_ALL, "kor");
	}
	wchar_t* ToUniStr(const char* buf, size_t len, bool isUTF8 = false) {
		size_t ulen = MultiByteToWideChar(isUTF8 ? CP_UTF8 : CP_ACP, 0, buf, len, 0, 0);

		wchar_t* ubuf = MyMalloc(wchar_t, ulen);
		MultiByteToWideChar(isUTF8 ? CP_UTF8 : CP_ACP, 0, buf, len, ubuf, ulen);
		return ubuf;
	}
	char* ToMultiStr(const wchar_t* ubuf, size_t ulen, bool isUTF8 = false) {
		size_t len = WideCharToMultiByte(isUTF8 ? CP_UTF8 : CP_ACP, 0, ubuf, -1, 0, 0, 0, 0);
		char* buf = MyMalloc(char, len);
		WideCharToMultiByte(isUTF8 ? CP_UTF8 : CP_ACP, 0, ubuf, -1, buf, len, 0, 0);
		return buf;
	}
	class MyFile {
	private:
		HANDLE _hfile;
	public:
		MyFile() {}
	};

	class MyParent {
	public:
		virtual ~MyParent() {}
	};
	TMP class MyContain :public MyParent {
	protected:
		T* _v;
		size_t _size;//_size는 stack에선 건드리지 말아야 됨.
		size_t _capa;
		inline void _Alloc(size_t sz) { _size = sz; _v = MyMalloc(T, sz + 1); }
		inline bool _Expand(size_t nowCapa, size_t expSize) {
			return _ExpandTo(nowCapa, expSize + _size,isDouble);
		}
		inline bool _ExpandTo(size_t nowCapa, size_t toSize) {
			size_t tc;
			bool flag = false;
			for (tc = nowCapa; tc < toSize; tc <<= 1) {
				flag = true;
			}
			_size = toSize; _capa = tc;
			return flag;
		}
	public:
		const T& operator[](size_t index) const {
			//if (index < _size)
			return this->_v[index];

		}
		T& operator[](size_t index) {
			//if (index < _size)
			return this->_v[index];
		}
		inline const T* End() { return this->_v + this->_size; }
		inline const T* Get() { return _v; }
		inline const size_t Size() { return _size; }
		inline const size_t Capa() { return _capa; }
		inline T& At(size_t index) {
			return this->_v[index];
		}
		inline const T& At(size_t index) const {
			return this->_v[index];
		}
		/*inline void Add(const T t) {
			if (this->_size + 1 >= this->_capa)_Expand();
			this->_v[this->_size++] = t;
		}*/
		virtual ~MyContain() { puts("~Contain"); }
	};

#define BASE MyContain<T>
	class MyMultiStr : public MyContain<char> {
	public:
		MyMultiStr(size_t sz) {
			_Alloc(sz);
		}
		MyMultiStr(char* str, size_t sz) {
			_Alloc(sz);
			memcpy(_v, str, sizeof(char) * sz);
		}
		virtual ~MyMultiStr() {
			free(_v);
			puts("~multistr");
		}
	};
	class MyUniStr :public MyContain<wchar_t> {
	public:
		MyUniStr(size_t sz) {
			_Alloc(sz);
		}
		MyUniStr(const wchar_t* wstr, size_t sz) {
			_Alloc(sz);
			memcpy(_v, wstr, sizeof(wchar_t) * sz);
		}
		virtual ~MyUniStr() {
			free(_v);
			puts("~unistr");
		}
	};
	TMP class MyArray : public MyContain<T> {
	private:
		bool _isDyn,_isDouble;
		void _Realloc() {
			this->_v=MyRealloc(this->_v, T, this->_capa);
		}
	public:
		MyArray(size_t capa = 4U, bool isDynamic = true,bool isDouble=true) {
			_size = 0u;
			_isDouble = isDouble;

			if (_isDyn = isDynamic)BASE::_ExpandTo(4u, capa);
			else this->_capa = capa;
			this->_v = MyMalloc(T, this->_capa);
		}
		void Add(const T t) {
			if (BASE::_Expand(this->_capa, 1))_Realloc();
			this->_v[this->_size++] = t;
		}
		inline void AddSize(size_t sz) {
			if(BASE::_Expand(this->capa, sz))_Realloc();
			_size += sz;
		}
		virtual ~MyArray() {
			if (this->_v) {
				free(this->_v);
				this->_v = 0;
			}
		}
	};
	TMP class MyObjArray : public MyContain<T> {
	protected:
		bool _isDyn,_isObj;
		void _Realloc() {
			auto buf= new T[this->_capa];
			for (size_t s = 0u; s < this->_size; s++) {

			}
			this->_v = buf;
		}
	public:
		MyObjArray(size_t sz=4u, bool isObj=true,bool isDynamic = true) {
			_isObj = isObj;
			this->_v = 0; this->_size = this->_capa = 0;
			_isDyn = isDynamic;
			if (!_isDyn) {
				this->_capa = sz;
			}
			else {
				BASE::_ExpandTo(4u, sz);
			}
			this->_v = new T[this->_capa];
		}
		void Add(const T t) {
			if (BASE::_Expand(this->_capa, 1))_Realloc();
			this->_v[this->_size++] = t;
		}
		void AddSize() {
		}
		void SetSize(){}
		virtual ~MyObjArray() {
			if (this->_v) {
				if (_isObj) {
					for (auto p = this->_v; p != End(); p++)
						delete* p;
				}
				delete[] this->_v;
				this->_v = 0;
				puts("~ObjArray");
			}
		}
#undef BASE
	};
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
