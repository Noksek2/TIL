#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <sstream>
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
0 1 2
0-y-V (n:0x3093, t:0x3063)
if 1!='y'
0:kg

1==0 C+y: c,$,j : i단
1=='y' C:i단 2=='aieou'->ya yu e yo



a k g s z t
wchar_t
									   xe 3047
a 3042 i 3044 u 3046 e 3048 o304A (+2)
ka 304B ki 304D ku 304F ke 3051 ko 3053 (+2)
ga 304C gi 304E gu 3050 ge 3052 go 3054 (+2)
sa 3055 (+2)si,$i
za 3056 (+2) ji,zi
ta 305f ci,ti 3061 tsu,tu 3064 te 3066 to 3068
da 3060 (+2)
na 306A ni 306B (+1)
ha 306F (+3)
ba 3070
pa 3071
ma 307E +1
ra 3089 (+1)


wa 308F wo 3092
xya 3083 xyu 3085 xyo 3087
ya 3084 yu 3086 yo3088

kata +=0x60

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

Cy y V Ce?
C1 V Ce?
V Ce?


*/
enum {
	rule_Cy,
	rule_V = 1,
	rule_C1 = 2,
	rule_Cend = 3,
};
CHAR man[4] = { 0,0,0,0 };
CHAR rule[][50] = { "kgnhbpmr","aiueo","kgszjtdcnhbpmryw","nt" };
char sylls[4096][4];
class GetCode {
public:
	wchar_t unimap[128][5] = { {0} };
	wchar_t head, mid, tail;
	void initvowel(char c, wchar_t code, UCHAR type = 2) {
		if (c == 't' || c == 'd') {
			unimap[c][0] = code;
			unimap[c][1] = code+2;
			unimap[c][2] = code + 5;
			unimap[c][3] = code + 7;
			unimap[c][4] = code + 9;
		}
		else if (c == 'w') {
			unimap[c][0] = code;
			unimap[c][1] = code + 1;
			unimap[c][2] = 0;
			unimap[c][3] = code + 2;
			unimap[c][4] = code + 3;
		}
		else if (c == 'y') {
			unimap[c][0] = code;
			unimap[c][1] = 0;
			unimap[c][2] = code + 2;
			unimap[c][3] = 0;
			unimap[c][4] = code + 4;
		}
		else {
			for (int i = 0; i < 5; i++) {
				unimap[c][i] = code + i * type;
			}
		}
	}
	GetCode() {

		//unimap[0][0] = 0x3042;
		initvowel(0, 0x3042);
		unimap['a'][0] = 0;
		unimap['i'][0] = 1;
		unimap['u'][0] = 2;
		unimap['e'][0] = 3;
		unimap['o'][0] = 4;
		initvowel('k', 0x304B);
		initvowel('g', 0x304C);
		initvowel('s', 0x3055);
		initvowel('z', 0x3056);
		initvowel('t', 0x305f);
		initvowel('d', 0x3060);
		initvowel('n', 0x306A,1);
		initvowel('h', 0x306F, 3);
		initvowel('b', 0x3070, 3);
		initvowel('p', 0x3071, 3);
		initvowel('m', 0x307E,1);

		initvowel('r', 0x3089,1);

		initvowel('w', 0x308F);
		initvowel('y', 0x3084);

		//initvowel('c', 0x3061);
		//initvowel('x', 0x3064);
	}
	wchar_t get_Cend(char c) {
		if (c == 'n')return 0x3093;
		else if (c == 't')return 0x3063;
		return 0;
	}
	wchar_t get_Yvowel(char c) {
		switch (c) {
		case 'a':return 0x3083;
		case 'u':return 0x3085;
		case 'e':return 0x3047;
		case 'o':return 0x3087;
		}
		return 0;
	}
	void getcode_HeadMid(char* syll) {
		head = mid = tail = 0;
		if (syll[1] == 'y') {
			head = unimap[syll[0]][1];
			mid = get_Yvowel(syll[2]);
		}
		else if (syll[0] == 'x') {
			head = 0x3064;
		}
		else if (syll[0] == 'c') {
			head = unimap['t'][1];
			mid = get_Yvowel(syll[2]);
		}
		else if (syll[0] == 'j') {
			head = unimap['z'][1];
			mid = get_Yvowel(syll[2]);
		}
		else if (syll[0] == '$') {
			head = unimap['s'][1];
			mid = get_Yvowel(syll[2]);
		}
		else head = unimap[syll[0]][unimap[syll[2]][0]];
	}
	void getcode(char* syll) {
		getcode_HeadMid(syll);
		tail = get_Cend(syll[3]);
	}
};
bool match_rule(CHAR c, UCHAR type) {
	for (CHAR* rc = rule[type]; *rc != 0; rc++) {
		if (*rc == c)return true;
	}
	return false;
}
void get_syllable(UINT& syllsize, char* str) {//음절
	for (CHAR* c = str; *c != '\0';) {
		while (*c == ' ' ||* c == '\n' ||* c == '\t' || *c == '\'')c++;
		[&]() {
			if (*(c + 1) == 'h') {
				if (*c == 's')man[0] = '$';
				else if (*c == 'c')man[0] = 'c';
				else return;
				c += 2;
			}
			else if (*c == 't' && *(c + 1) == 's') {
				man[0] = 'x';
				c += 2;
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
			else {
				cout << "\n==모음 획득 실패==";
				break;
			}

			if (match_rule(*c, rule_Cend)) {
				if (*c == 't' && *(c + 1) == 's');
				else if (!match_rule(*(c + 1), rule_V)) {
					man[3] = *c;
					c++;
				}
			}
			else if (*c == *(c + 1) && (*c == 'p' || *c == 's' || *c == 'k')) {
				man[3] = 't';
				c++;
			}
			if (man[0] == '$' && man[1] == 'i')man[0] = 's';

			sylls[syllsize][0] = man[0];
			sylls[syllsize][1] = man[1];
			sylls[syllsize][2] = man[2];
			sylls[syllsize++][3] = man[3];
			man[0] = man[1] = man[2] = man[3] = 0;
	}
}
void main() {
	CHOOSEFONT font = { 0 };
	LOGFONT logfont;
	font.lStructSize = sizeof(CHOOSEFONT);
	font.hwndOwner = GetConsoleWindow();
	font.iPointSize = 100;
	font.Flags = CF_EFFECTS | CF_SCREENFONTS;
	font.lpLogFont = &logfont;
	//if (ChooseFont(&font)!=0) {}

	setlocale(LC_ALL, 0);
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

	/*음절 전부 획득하기*/
	UINT syllsize = 0;
	get_syllable(syllsize, str);

	for (UINT i = 0; i < syllsize; i++) {
		for (int j = 0;j < 4;j++)
			if (sylls[i][j] != 0)putchar(sylls[i][j]);
		putchar(' ');
	}
	{
		GetCode* getcode;
		getcode = new GetCode();

		std::wofstream wof;
		wof.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
		wof.open(L"a.out");
		for (UCHAR c = 0;c < 128;c++) {
			for (int j = 0;j < 5;j++)
				wof << getcode->unimap[c][j];
			wof << '\n';
		}
		for (UINT i = 0; i < syllsize; i++) {
			for (int j = 0;j < 4;j++)
				if (sylls[i][j] != 0)wof << sylls[i][j];
			
			getcode->getcode(sylls[i]);
			if(getcode->head)wof << getcode->head;
			if (getcode->mid)wof << getcode->mid;
			if (getcode->tail)wof << getcode->tail;
			wof << ' ';
		}
		wof << L"終";

		wof.close();

		delete getcode;
	}
	::free(str);
	//cnst
	//vowel
}
