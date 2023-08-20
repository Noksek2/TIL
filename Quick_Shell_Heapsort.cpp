#define PROGRAM
#ifdef PROGRAM
#include <iostream>
using namespace std;
int arr[100000] = {1,2,3,4,9,5,6,7,8};
void swaping(int a, int b) {
	int d = arr[a];
	arr[a] = arr[b];
	arr[b] = d;
}
void QuickSort(int l,int r) { 
	int p = (l+r)/2;
	int low = l, high = r;
	swaping(p, l);
	low++;
	//0 1 0 1 2 1 10 21
	while (low<high) {
		while (low<high && arr[low]<=arr[l]) {
			low++;
		}
		while (low < high && arr[high]>arr[l]) {
			high--;
		}
		swaping(low, high);
	}
	if (arr[low] > arr[l])low--; 
	swaping(low, l);
	if (l < low - 1)QuickSort(l, low - 1);
	if (low + 1 < r)QuickSort(low + 1, r);
	//return high - 1;
}
void PrintArr(int size) {
	for (int i = 0; i < size; i++)cout << arr[i] << ' ';
	cout << '\n';
}
void ShellSort(int size) {
	int inc = size / 2;
	int temp;
	int j;
	while (inc > 0) {
		for (int t = 0; t < inc; t++) {
			for (int i = t+inc; i < size; i += inc) {
				temp = arr[i];
				for (j = i - inc; j >= 0; j -= inc) {
					if (arr[j] > temp)arr[j + inc] = arr[j];
					else break;
				}
				arr[j + inc] = temp;
			}
		}
		inc /= 2;
	}
}
#include <time.h>
void HeapInsert(int i) {
	int pi;
	while (i >0) {
		pi = (i - 1) / 2;
		if (arr[pi] < arr[i]) {
			swaping(pi, i);
		}
		i = pi;
	}
}
void HeapDelete(int size){
	int i = 0;
	int lc, rc,p=i;
	while (i<size) {
		lc = i * 2+1;
		rc = lc + 1;
		if (rc < size && arr[p] < arr[rc])
			p = rc;
		if (lc < size && arr[p] < arr[lc])
			p = lc;
		if (p == i)break;
		swaping(p, i);
		i = p;
	}
}
void HeapSort(int size) {
	for (int i = 0; i < size; i++) {
		HeapInsert(i);
	}
	//PrintArr(size);
	for (int i = size - 1; i > 0; i--) {
		swaping(0, i);
		HeapDelete(i - 1);
	}
/*
parent = (n-1)/2
p*2+1 = left
p*2+2 = right
0 /1 2/3456/78 9
*/
}
void main() {
	time_t s, e;
	s = clock();
	int size = 100000;
	for (int i = 0; i < size; i++)arr[i] = rand() % size;
	//QuickSort(0,size-1);
	//ShellSort(size);
	HeapSort(size);
	for (int i = 0; i < size - 1; i++)
		if (arr[i] > arr[i + 1]) {
			goto FAIL;
		}
	cout << "SUCCESS\n";
	goto END;
FAIL:
	cout << "FAILED\n";
END:
	e = clock();
	cout << '\n' << e - s;
}
#endif
