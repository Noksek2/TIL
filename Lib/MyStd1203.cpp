#include "mystd.h"
using namespace MyStd;
#include <vector>
#include <time.h>
using namespace std;

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
	MyContain(T* vt,size_t _len) {
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
class MyMulStr :public MyVec<char>{
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

class MyUniStr :public MyVec<wchar_t>{
public:
	MyUniStr(const wchar_t* _wstr) {
		wchar_t* wstr=new wchar_t[wcslen(_wstr)+1];
		wcscpy(wstr, _wstr);
		_p = 0;
		_p = new MyContain<wchar_t>(wstr, wcslen(_wstr));
	}
	void Print() {
		_putws(_p->_v);
	}
	
};

void main() {
	SetInit();
	MyUniStr uni(L"기분 안 좋다");
	MyUniStr uni2(uni);
	uni2.Print();
	uni2[1] = L'd';
	uni2.Print();

	MyMulStr str("I don't know");

	auto it(str.ToIter());
	auto str2(str);
	str2.Print();

	while (it.IsAble()) {
		_putch(*it);
		++it;
	}
};
