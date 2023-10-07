#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <Windows.h>
using namespace std;

UINT readfile(FILE* file, char*& s) {
	if (file == 0)return 0;
	fseek(file, 0, SEEK_END);
	size_t fsize = ftell(file) + 1;
	rewind(file);
	s = (char*)calloc(fsize, sizeof(char));
	fread(s, 1, fsize - 1, file);
	return fsize;
}

/*
a i u e o
ka ki ku ke ko
kya kyu kye kyo
ga gi gu ge go
gya gyu gye gyo
sa shi-si su se so
sha shu she sho
za ji-zi zu ze zo
ja ju je jo
ta chi-ti-ci tsu-tu te to
cha-ca chu-cu che-ce cho-co
da di du de do
na ni nu ne no
nya nyu nye nyo
ha hi hu he ho
hya hyu hye hyo
ba bi bu be bo
bya byu bye byo
pa pi pu pe po
pya pyu pye pyo
ma mi mu me mo
mya myu mye myo
ra ri ru re ro
rya ryu rye ryo
wa wi we wo 
*/
enum{
	rule_Cy,
	rule_V=1,
	rule_C1=2,
	rule_Cend=3,
};
CHAR man[4] = { 0,0,0,0 };
CHAR rule[][50] = { "kgnhbpmr","aiueo","kgszjtdcnhbpmryw","nt"};

void syllable() {//음절
}
bool match_rule(CHAR c,UCHAR type) {
	for (CHAR* rc = rule[type]; *rc != 0; rc++) {
		if (*rc == c)return true;
	}
	return false;
}
void main() {
	CHAR* str;
	FILE* file;
	size_t fsize;
	fopen_s(&file, "a.txt", "r");
	if (file == 0) {
		printf("응 파일 없어");
		return;
	}
	fsize = readfile(file, str);
	fclose(file);
	
	puts(str);
	for (CHAR* c = str; *c != '\0';) {
		while (*c == ' ' || *c == '\'')c++;
		[&]() {
			if (*(c + 1) == 'h') {
				if (*c == 's')man[0] = '$';
				else if (*c == 'c')man[0] = 'c';
				else return;
				c+=2;
			}
			else if (*c == 't' && *(c + 1) == 's') {
				man[0] = 'x';
				c+=2;
			}
		}();//sh ch
		if (man[0] != 0)goto VOWEL;
		if (*(c + 1) == 'y') {
			if (match_rule(*c, rule_Cy)) {
				man[0] = *c; man[1] = 'y';
				c += 2;
			}
		}
		else if (match_rule(*c, rule_C1)) {
			man[0] = *c;
			c++;
		}
	VOWEL:
		if (match_rule(*c, rule_V)) {
			man[2] = *c;
			c++;
		}
		else{
			cout << "\n==모음 획득 실패==";
			break;
		}

		if (match_rule(*c, rule_Cend)) {
			if (*c == 't' && *(c + 1) == 's');
			else if (!match_rule(*(c+1), rule_V)) {
				man[3] = *c;
				c++;
			}
		}
		else if (*c == *(c + 1)&&(*c=='p'|| *c == 's'|| *c == 'k')) {
			man[3] = 't';
			c++;
		}
		for (int i = 0; i < 4; i++) {
			if (man[i]!=0)cout << man[i];
		}
		cout << ' ';
		man[0] = man[1] = man[2] = man[3] = 0;
		
	}
	free(str);
	//cnst
	//vowel
}
