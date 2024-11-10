#include "mystd.h"
#include <iostream>
#include <vector>
using namespace MyStd;
class MyParent {
public:
	virtual ~MyParent(){}
};
TMP class MyContain:public MyParent{
protected:
	T* _v;
	size_t _size;
	size_t _capa;
	void _Alloc(size_t sz) { _size = sz; _v = MyMalloc(T, sz+1); }
	void _Expand(size_t nowCapa,size_t expSize) {
		size_t tc;
		for (tc=nowCapa; tc < _size+expSize; tc <<= 1);
	}
	void _ExpandTo(size_t nowCapa, size_t toSize) {
		size_t tc;
		for (tc = nowCapa; tc < toSize; tc <<= 1);
	}
public:
	const T* End() { return this->_v + this->_size; }
	const T* Get() { return _v; }
	const size_t Size() { return _size; }
	const size_t Capa() { return _capa; }
	void Add(const T t) {
		if (this->_size + 1 >= this->_capa)_Expand();
		this->_v[this->_size++] = t;
	}
	virtual ~MyContain(){ puts("~Contain"); }
};

#define BASE MyContain<T>
class MyMultiStr: public MyContain<char> {
public:
	MyMultiStr(size_t sz) {
		_Alloc(sz);
	}
	MyMultiStr(char* str,size_t sz) {
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
	MyUniStr(const wchar_t* wstr,size_t sz) {
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
	bool _isDyn;
public:
	const T* End() { return this->_v + this->_size; }
	MyArray(size_t sz = 4U, bool isDynamic = true) {
		_size = sz;
		if (_isDyn = isDynamic)BASE::_Expand(4u,);
		
		this->_v = MyMalloc(T, sz);
	}
	void Add(const T t) {
		if (this->_size + 1 >= this->_capa)_Expand();
		this->_v[this->_size++] = t;
	}
	const T operator[](size_t index) const {
		//if (index < _size)
		return this->_v[index];

	}
	T& operator[](size_t index) {
		//if (index < _size)
		return this->_v[index];
	}
	virtual ~MyArray() {
		if (this->_v) {
			if (_isBasic) {
				free(this->_v);
				this->_v = 0;
			}
			else {
				for (auto p = this->_v; p != End(); p++)
					delete* p;
				delete[] this->_v;
				this->_v = 0;
				puts("~Array");
			}
		}
	}
};
TMP class MyObjArray: public MyContain<T>{

private:
	bool _isDyn;
	void _Expand() {
	}
public:
	MyObjArray(size_t sz = 4U, bool isDynamic = true) {
		this->_v = 0; this->_size = this->_capa = 0;
		_isDyn = isDynamic;
		if (!_isDyn) {
			if (_isBasic)this->_v = MyMalloc(T, sz);
			else this->_v = new T[sz];

			this->_size = this->_capa = sz;
		}
		else {
			this->_size = sz;
			for (this->_capa = 1; this->_capa < this->_size; this->_capa <<= 1);

			if (_isBasic)this->_v = MyMalloc(T, this->_capa);
			else this->_v = new T[this->_capa];
			this->_size = 0;
		}
	}
	
	const T operator[](size_t index) const {
		//if (index < _size)
		return this->_v[index];

	}
	T& operator[](size_t index) {
		//if (index < _size)
		return this->_v[index];
	}
	virtual ~MyArray() {
		if (this->_v) {
			if (_isBasic) {
				free(this->_v);
				this->_v = 0;
			}
			else {
				for (auto p = this->_v; p != End(); p++)
					delete *p;
				delete[] this->_v;
				this->_v = 0;
				puts("~Array");
			}
		}
	}
#undef BASE
};
class AutoRelease {
	MyObjArray<MyParent*>* obj;
public:
	AutoRelease() { obj = new MyArray<MyParent*>(); }
	void Add(MyParent* o) {
		obj->Add(o);
	}
	~AutoRelease() {
		delete obj;
	}
};
void main() {
	MyStd::SetInit();

	{
		AutoRelease ar;
		auto us = new MyUniStr(5);
		auto ms = new MyMultiStr(100);
		auto as = new MyArray<int>(4u,false,true);
		as[0] = 10;
		printf("%d\n", as[0]);
		ar.Add(us);
		ar.Add(ms);
		ar.Add(as);
	}
	_getch();
}
