
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
void main() {
	time_t s, e;
	s = clock();
	int size = 100000;
	for (int i = 0; i < size; i++)arr[i] = rand();
	//QuickSort(0,size-1);
	ShellSort(size);
	e = clock();
	cout << e - s;
	/*for (int i = 0; i < size; i++)
		cout << arr[i]<<' ';*/
}
