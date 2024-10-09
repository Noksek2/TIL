#include <stdio.h>
#include <locale.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
int wmain(int argc,wchar_t** argv){
	setlocale(LC_ALL,"korean");
	//wprintf(L"나나나나ㅏ다ㅏ라마바사");
	unsigned int count=0,len=0,rdx=0;
	printf("input number(max 9 length) : ");
	char number[9]="98765431";
	//scanf("%s",number);
	puts(number);

	const char numname[][10]={"zAh", "yek","do","se", "cAr", "pan", "sheh", "ef", "hash", "noh"
	};
	const char rdxname[][10]={"","dah","sad","dah","emet"};
	char* p;
	p=number;

	while(*p>='0' && *p<='9'){
		*p-='0';
		p++;len++;
	}
	
	char *buf;
	for(int i=0;i<len;i++){
		printf(numname[number[i]]);
		rdx=(len-i-1);
		printf(rdxname[rdx%4]);
		if(rdx==4){printf(rdxname[rdx]);}

		putch(' ');
		//if(i==len-1)putch(' ');
		//else printf("'a ");
	}
	//printf(number);
	getch();
	return 0;
}
/*
safe : 
risky : 
*/
