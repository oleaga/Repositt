// Xonix.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <conio.h>
#include <dos.h>
#include <iostream>
#include <ctime>

#include <mmsystem.h>				// необходимы для подключения музыки
#pragma comment (lib, "winmm.lib")  // необходимы для подключения музыки

#include <string.h>
#include <stdlib.h>
#include <direct.h>

#define sizeX 77// количество символов консоли по горизонтали, в циклах для буфера Scrn - вторая координата (соответствует j)
#define sizeY 32// количество символов консоли по вертикали, в циклах для буфера Scrn - первая координата (соответствует i)

const int LenStr=30;
const int StrCount=10;

struct CharAttr
{
	unsigned char Symbol;
	unsigned char Color;
	unsigned short int FieldID;// номер области, используется для подсчета кол-ва областей для штриховки
};

struct Ghost
{
	COORD CurPos;
	COORD PrevPos;
	short int ID;

};

enum SymbolCode
{
	ChBorder = 0xb0,
	ChLine   = 0xf9,
	ChGhost  = 0xfd,
	ChSpace  = 0x20, // ' ' - пробел
	ChHead   = '@'
};

enum KeyCode
	{
		KDown      = 0x50,
		KUp        = 0x48,
		KLeft      = 0x4b,
		KRight     = 0x4d,
		KEnter     = 0x0d,
		KEsc       = 0x1b,
		KDefault   = 0x25,
		KBackspace = 0x08
	};

struct TextAttribute
	{
		unsigned char SelTextAttr;
		unsigned char BorderTextAttr;
		unsigned char LightSelTextAttr;
		unsigned char LightTextAttr;
		unsigned char XTextAttr;
		unsigned char GhostTextAttr;
		unsigned char DarkGhostTextAttr;
		unsigned char HeadTextAttr;
		unsigned char LineTextAttr;
		unsigned char TextAttr;
	} TA,DTA,LTA;

HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
UINT CodePage;// код страницы символов, используемой системой по умолчанию

int shell();
void PreSet(COORD &SizeWin);
int Menu(COORD Center, int &ItemNumber);
void GotoXY(COORD &Pos);
int Play(COORD SizeWin, unsigned int &Score);
bool EscMsBox(COORD SizeWin,CharAttr *Scrn);
bool TryMsBox(COORD SizeWin, CharAttr Scrn[sizeY][sizeX], int TryCnt);
bool MoveGhosts(Ghost Ghosts[], CharAttr Scrn[sizeY][sizeX]);
bool MoveHead(Ghost *Head, CharAttr Scrn[sizeY][sizeX], int Key, COORD &StartPos, COORD &TopLeft, COORD &BottRight, unsigned int &Score, unsigned int &FieldCount);
void PrintChr(COORD Pos, unsigned char Clr, unsigned char Smbl);
void PrintStr(COORD Pos, unsigned char Clr, char *Str);
void Filling(CharAttr Scrn[sizeY][sizeX], Ghost *Ghs,  unsigned int &Score, unsigned int &FieldCount);
void ReadRecords(char NameList[StrCount][LenStr], unsigned int ScoreList[StrCount]);
void SaveRecords(char NameList[StrCount][LenStr], unsigned int ScoreList[StrCount]);
void Records(COORD &SizeWin, unsigned int &Score,char NameList[StrCount][LenStr], unsigned int ScoreList[StrCount]);
void ShowRecords(COORD SizeWin, char NameList[StrCount][LenStr], unsigned int ScoreList[StrCount], unsigned int &Score, short int Index);
void SetName(COORD SizeWin, COORD Pos, char Str[21]);
void Exit(COORD SizeWin);
void Settings(COORD SizeWin);
void ClearRect(COORD SizeWin, COORD Pos, COORD Size);

int main()
{
	return shell();
}

int shell()
{
	COORD SizeWin;

	unsigned int Score=0;
	unsigned int ScoreList[StrCount];
	int ItN = 0;
	char NameList[StrCount][LenStr];
	
	DTA.BorderTextAttr = 0x07;
	DTA.SelTextAttr = 0x70;
	DTA.LightTextAttr = 0x0f;
	DTA.LightSelTextAttr = 0xf0;
	DTA.XTextAttr = 0x0d;
	DTA.GhostTextAttr = 0x0e;
	DTA.DarkGhostTextAttr = 0x06;
	DTA.HeadTextAttr = 0x0f;
	DTA.LineTextAttr = 0x08;
	DTA.TextAttr = 0x07;

	LTA.BorderTextAttr = 0x8f;
	LTA.SelTextAttr = 0xf8;
	LTA.LightTextAttr = 0x80;
	LTA.LightSelTextAttr = 0xf0;
	LTA.XTextAttr = 0x85;
	LTA.GhostTextAttr = 0x8e;
	LTA.DarkGhostTextAttr = 0x86;
	LTA.HeadTextAttr = 0x80;
	LTA.LineTextAttr = 0x81;
	LTA.TextAttr = 0x80;

	TA = DTA;

	PreSet(SizeWin);
	ReadRecords(NameList, ScoreList);
	while(1)
	{
		Menu(SizeWin, ItN);
		switch(ItN)
		{
			case 0:// PLAY()
				{
					Play( SizeWin, Score);	
				}
			case 1:// RECORDS()
				{
					Records( SizeWin, Score, NameList, ScoreList);
					break;
				}
			case 2:// SETTINGS()
				{
					//ЗДЕСЬ ФУНКЦИЯ SETTINGS()
					Settings( SizeWin);
					break;
				}
			case 3:// EXIT
				{
					Exit( SizeWin);
					return 0;
				}// EXIT
		}
	};
	return -1;
}

void Exit(COORD SizeWin)
{
	char GoodBye[] = "Всего хорошего!";
	int i, j, n=strlen(GoodBye);
	COORD Pos;

	system("cls");
	SizeWin.X = (SizeWin.X-strlen(GoodBye))/2;
	SizeWin.Y = SizeWin.Y/2 - 1;
	Pos.X = SizeWin.X-1;
	Pos.Y = SizeWin.Y;
	mciSendString(L"play sound\\завершение.wav",0,0,0);
	PrintChr(Pos, TA.DarkGhostTextAttr, GoodBye[0]);
	Sleep(50);
	for(i=1; i<n; i++)
	{
		PrintChr(Pos, TA.GhostTextAttr, GoodBye[i-1]);
		Pos.X++;
		PrintChr(Pos, TA.DarkGhostTextAttr, GoodBye[i]);
		Sleep(50);
	}
	PrintChr(Pos, TA.GhostTextAttr, GoodBye[i-1]);
	Sleep(300);
	Pos.X = SizeWin.X-1;
	Pos.Y = SizeWin.Y;

	PrintChr(Pos, TA.DarkGhostTextAttr, GoodBye[0]);
	Sleep(50);
	for(i=1; i<n; i++)
	{
		PrintChr(Pos, TA.GhostTextAttr, ' ');
		Pos.X++;
		PrintChr(Pos, TA.DarkGhostTextAttr, GoodBye[i]);
		Sleep(50);
	}
	PrintChr(Pos, TA.GhostTextAttr, ' ');
	Sleep(50);
}

void GotoXY(COORD &Pos)
{
  SetConsoleCursorPosition(handle,Pos);
}

int Menu(COORD Center, int &CurrItem)
{
	int Key = KDefault;

	const int IQ = 4;// количество строк меню
	const int IL = 14;// длина строки меню

	char Items[IQ][IL] = {"    Игра     ","   Рекорды   ","  Настройки  ","    Выход    "};// можно добавлять строки, но 'выход' должен быть в последней строке!!!

	COORD Pos;

	Center.X /= 2;
	Center.Y /= 2;

	mciSendString(L"play sound\\меню.mp3 repeat",0,0,0);

	Pos.X = Center.X-IL/2;
	SetConsoleTextAttribute(handle, TA.BorderTextAttr);
	for(int i = 0; i<IQ; i++)
	{
		if(CurrItem==i) SetConsoleTextAttribute(handle, TA.SelTextAttr);
		Pos.Y = Center.Y-IQ+i*2;
		GotoXY(Pos);
		printf("%s",Items[i]);
		SetConsoleTextAttribute(handle, TA.BorderTextAttr);
	}

	while(!((Key==KEnter) && (CurrItem==IQ-1)))
	{
		Key = getch();
		Pos.X = Center.X-IL/2;
		switch(Key)
		{
			case KUp   : 
			{
				Pos.Y = Center.Y-IQ+2*CurrItem;
				GotoXY(Pos);
				printf("%s",Items[CurrItem]);
				if(CurrItem>0) CurrItem--;
				else CurrItem = IQ-1;
				SetConsoleTextAttribute(handle, TA.SelTextAttr);
				Pos.Y = Center.Y-IQ+2*CurrItem;
				GotoXY(Pos);
				printf("%s",Items[CurrItem]);
				SetConsoleTextAttribute(handle, TA.BorderTextAttr);
				break;
			}
			case KDown : 
			{
				Pos.Y = Center.Y-IQ+2*CurrItem;
				GotoXY(Pos);
				printf("%s",Items[CurrItem]);
				if(CurrItem<IQ-1) CurrItem++;
				else CurrItem = 0;
				SetConsoleTextAttribute(handle, TA.SelTextAttr);
				Pos.Y = Center.Y-IQ+2*CurrItem;
				GotoXY(Pos);
				printf("%s",Items[CurrItem]);
				SetConsoleTextAttribute(handle, TA.BorderTextAttr);
				break;
			}
			case KEnter:
			{
				mciSendString(L"stop sound\\меню.mp3",0,0,0);
				return 0;
			}
		}
	}

	mciSendString(L"stop sound\\меню.mp3",0,0,0);
	return -1;
}

void PreSet(COORD &SizeWin)
{
	COORD FontSize;
	unsigned int dx;
	unsigned int dy;
	unsigned int sx;
	unsigned int sy; 
	// устанавливаем страницу символов ASCII на ввод и вывод кириллицы
	CodePage = GetConsoleOutputCP();
	SetConsoleTextAttribute(handle,TA.BorderTextAttr);
	system("cls");

	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);
	
	CONSOLE_FONT_INFO FontInfo;
	GetCurrentConsoleFont(handle,false,&FontInfo);
	FontSize = GetConsoleFontSize(handle, FontInfo.nFont);
	sx = FontSize.X+1;
	sy = FontSize.Y+1;
	
	SizeWin.X = 10;
	SizeWin.Y = 10;
	SetConsoleScreenBufferSize(handle,SizeWin);
	SizeWin.X = sizeX;
	SizeWin.Y = sizeY;
	dx = (1400 - sx*SizeWin.X)/2;
	dy = ( 750 - sy*SizeWin.Y)/2;

	// устанавливаем рабочую папку
	chdir("..\\Debug\\");
	// устанавливаем заголовок окна
	SetConsoleTitle(_T("XONIX"));

	// прячем курсор. текст с форума. http://www.cyberforum.ru/cpp-beginners/thread102665.html
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);
	CursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo( handle, &CursorInfo );

	// перемещаем и растягиваем окно
	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, dx*sx, dy*sy,1,1,true);
    SetConsoleScreenBufferSize(handle,SizeWin);// подтянули размер консоли
	MoveWindow(hwnd, dx, dy, SizeWin.X*sx, SizeWin.Y*sy, true);// расположили консоль по центру

}

int Play(COORD SizeWin, unsigned int &Score)
{
	const int GN = 100; // Ghosts number
	const int LOFN = 100;// Lenght Of FileName;

	char LvlDir[] = "levels\\";
	char LvlFileExt[] = ".lvl";
	char LvlFileName[LOFN] = "levels\\0.lvl";
	char LvlFileName2[LOFN]="";
	char str[LOFN];
	char *str2;
	char tryStr[]="Осталось попыток: ";

	int Key = KDefault;
	int i,j,GCnt,Lvl,TryCnt = 3;
	Score = 0;
	unsigned int FieldCount = 0;
	unsigned int FieldMax = (sizeX-2)*(sizeY-4);
	unsigned int FieldLvl;
	unsigned int minTime;
	unsigned short int GPrison;

	bool isExit = 0;
	bool isFail = 0;

	COORD Center;
	COORD Pos;
	COORD HeadPos;
	COORD StartPos, TopLeft, BottRight;

	FILE* LvlFile;

	Ghost Ghs[GN];

	CharAttr Scrn[sizeY][sizeX];
	//////////////////////////////////////////24 клетки под каждую муху
	
	Lvl=1;
	do{
		minTime=60-(Lvl-1)*6;
		itoa(Lvl,LvlFileName,10);
		strcat(LvlFileName,LvlFileExt);
		strcpy(str,LvlDir);
		strcat(str,LvlFileName);
		strcpy(LvlFileName,str);
		LvlFile = fopen (LvlFileName,"r");
		if(LvlFile !=0)
		{
			system("cls");
			mciSendString(L"play sound\\игра.mp3 repeat",0,0,0);
			SetConsoleOutputCP(1251);
			Pos.X = 0;
			Pos.Y = 0;
			GotoXY(Pos);
			SetConsoleTextAttribute(handle, TA.XTextAttr);
			printf(" Уровень:");// 9 символов, 10-й - номер уровня!
			Pos.X = 10;
			GotoXY(Pos);
			printf("%d",Lvl);

			Pos.X = sizeX - strlen(tryStr)-3;
			GotoXY(Pos);
			printf("%s%d",tryStr,TryCnt);

			char strScore[20]="Очки: ";
			Pos.X=35;
			Pos.Y=0;
			PrintStr(Pos, TA.XTextAttr, strScore);
			Pos.X=35 + strlen(strScore);
			itoa(Score,strScore,10);
			PrintStr(Pos, TA.XTextAttr, strScore);

			SetConsoleOutputCP(CodePage);
			for(i = 0; i<GN; i++) Ghs[i].ID = -1;
			GCnt = 1;
			for(i = 1; i<sizeY-1; i++)
			{
				Pos.Y = i;
				str2 = fgets(str, sizeX+5, LvlFile);
				for(j = 0; j<sizeX; j++)
				{
					Pos.X = j;
					Scrn[i][j].Symbol = str[j];
					Scrn[i][j].Color = TA.BorderTextAttr;
					Scrn[i][j].FieldID = 0;// номер области, используется для подсчета кол-ва областей для штриховки
					if(Scrn[i][j].Symbol==ChGhost)
					{
						Scrn[i][j].Color = TA.GhostTextAttr; //
						Ghs[GCnt].ID = GCnt;
						Ghs[GCnt].CurPos.X = j;
						Ghs[GCnt].CurPos.Y = i;
						////////////////////////////////////
						COORD PrvPos[4];
						PrvPos[0].X = Ghs[GCnt].CurPos.X-1;
						PrvPos[0].Y = Ghs[GCnt].CurPos.Y-1;

						PrvPos[1].X = Ghs[GCnt].CurPos.X-1;
						PrvPos[1].Y = Ghs[GCnt].CurPos.Y+1;

						PrvPos[2].X = Ghs[GCnt].CurPos.X+1;
						PrvPos[2].Y = Ghs[GCnt].CurPos.Y+1;

						PrvPos[3].X = Ghs[GCnt].CurPos.X+1;
						PrvPos[3].Y = Ghs[GCnt].CurPos.Y-1;
						////////////////////////////////////
						srand( time(0) );
						unsigned int numb = (rand()%100 + GCnt) * (rand()%10 + GCnt)%4;
						Ghs[GCnt].PrevPos.X = PrvPos[numb].X;
						Ghs[GCnt].PrevPos.Y = PrvPos[numb].Y;
						GCnt++;
					
					}
					else
						if(Scrn[i][j].Symbol == ChHead)
						{
							Scrn[i][j].Symbol = ChBorder;
							//Scrn[i][j].Color = TA.HeadTextAttr;
							Ghs[0].ID = ChBorder;// устанавливаем символ бордюра 'под' голову
							Ghs[0].CurPos.X = j;
							Ghs[0].CurPos.Y = i;
							Ghs[0].PrevPos.X = j;
							Ghs[0].PrevPos.Y = i;
							HeadPos.X = Ghs[0].CurPos.X;
							HeadPos.Y = Ghs[0].CurPos.Y;
						}
					PrintChr(Pos, Scrn[i][j].Color, Scrn[i][j].Symbol);
				}
				PrintChr(Ghs[0].CurPos, TA.HeadTextAttr, ChHead);
			}
			fclose(LvlFile);
			Center.X = SizeWin.X/2;
			Center.Y = SizeWin.Y/2-1;
			GPrison = 24;
			FieldLvl = (GCnt-1)*GPrison;
			do{
				while(Key!=KEsc && !isFail)
				{
					for(int t = 0; t<2; t++)
					{
						if(kbhit()!=0)
						{
							Key = getch();
							if((Key==KLeft||Key==KRight||Key==KUp||Key==KDown)) isFail = MoveHead(Ghs, Scrn, Key, StartPos, TopLeft, BottRight, Score, FieldCount);
							Sleep(minTime);
						}
						else
						{
							if(kbhit()!=0)
							{
								Key = getch();
							}
							Sleep(minTime);
							if(Key != KEsc) Key = KDefault;
							isFail = MoveHead(Ghs, Scrn, Key, StartPos, TopLeft, BottRight, Score, FieldCount);
						}
						if(FieldMax-FieldCount<FieldLvl) 
						{
							int tmp=TryCnt;
							TryCnt=-1;
							TryMsBox(SizeWin, Scrn, TryCnt);
							TryCnt=tmp;
							Key = KDefault;
							break;
						}
					}
					if(FieldMax - FieldCount < FieldLvl)
					{
						mciSendString(L"stop sound\\игра.mp3",0,0,0);
						break;
					}
					isFail = MoveGhosts(Ghs, Scrn);
				}
		
				if(isFail)
				{
					HeadPos.X=StartPos.X;
					HeadPos.Y=StartPos.Y;
					TryCnt--;
					SetConsoleTextAttribute(handle, TA.XTextAttr);
					Pos.Y = 0;
					Pos.X = sizeX -3;//осталось попыток: 
					GotoXY(Pos);
					printf("%d",TryCnt);

					SetConsoleOutputCP(CodePage);
			
					Ghs[0].CurPos=HeadPos;
					Ghs[0].PrevPos=HeadPos;
					Ghs[0].ID=ChBorder;
					//Scrn[Ghs[0].CurPos.Y][Ghs[0].CurPos.X].Symbol=ChBorder;

					mciSendString(L"pause sound\\игра.mp3",0,0,0);
					TryMsBox(SizeWin, Scrn, TryCnt);
					int I,J;
					I=BottRight.X;
					J=BottRight.Y;
					//SetConsoleOutputCP(CodePage);
					for(j=TopLeft.Y;j<=J;j++)
					{
						for(i=TopLeft.X;i<=I;i++)
						{
							if(Scrn[j][i].Symbol==ChLine)
							{
								Pos.X=i;
								Pos.Y=j;
								Scrn[j][i].Color=TA.BorderTextAttr;
								Scrn[j][i].Symbol=ChSpace;
								PrintChr(Pos, Scrn[j][i].Color, Scrn[j][i].Symbol);
							}
						}
					}
					PrintChr(Ghs[0].CurPos, Scrn[Ghs[0].CurPos.Y][Ghs[0].CurPos.X].Color, Scrn[Ghs[0].CurPos.Y][Ghs[0].CurPos.X].Symbol);
					PrintChr(HeadPos, TA.HeadTextAttr, ChHead);
					//SetConsoleOutputCP(1251);
					mciSendString(L"resume sound\\игра.mp3",0,0,0);
					isExit=!(TryCnt);
				}
				isFail=0;
		
				if(Key==KEsc)
				{
					mciSendString(L"pause sound\\игра.mp3",0,0,0);
					isExit = EscMsBox(SizeWin, &(Scrn[0][0]));
					mciSendString(L"resume sound\\игра.mp3",0,0,0);
				}
				if(FieldMax - FieldCount < FieldLvl)
				{
					FieldCount=0;
					break;
				}
				Key = KDefault;
			}while(!isExit);
			if(isExit)
			{
				system("cls");
				mciSendString(L"stop sound\\игра.mp3",0,0,0);	
				SetConsoleOutputCP(1251);
				return 0;
			}
		}
		Lvl++;
	}while(LvlFile != 0);
	SetConsoleOutputCP(1251);
	return 0;
}

bool MoveGhosts(Ghost Ghs[], CharAttr Scrn[sizeY][sizeX])
{
	int i;
	COORD Next;
	COORD Templ;
	unsigned char NextSymbol;
	bool GameOver = 0;
	

	for(i = 1; Ghs[i].ID!=-1; i++)// перемещение призраков начиная с первого в массиве (нулевой - голова)
	{
		Next.X = 2 * Ghs[i].CurPos.X - Ghs[i].PrevPos.X;
		Next.Y = 2 * Ghs[i].CurPos.Y - Ghs[i].PrevPos.Y;
		NextSymbol = Scrn[Next.Y][Next.X].Symbol;

		if(NextSymbol == ChSpace || NextSymbol == ChLine)
		{
			Ghs[i].PrevPos.X = Ghs[i].CurPos.X;
			Ghs[i].PrevPos.Y = Ghs[i].CurPos.Y;
			Ghs[i].CurPos.X = Next.X;
			Ghs[i].CurPos.Y = Next.Y;
			if(NextSymbol == ChLine)
				{
					GameOver = 1;
				}
			
		}
		else
		{
			if(NextSymbol == ChBorder)
			{
				if(Scrn[Next.Y] [Ghs[i].CurPos.X].Symbol == ChBorder && Scrn[Ghs[i].CurPos.Y] [Next.X].Symbol != ChBorder)
				{
					Templ.X = Ghs[i].CurPos.X;
					Templ.Y = Ghs[i].CurPos.Y;
					Ghs[i].CurPos.X = Next.X;
					Ghs[i].CurPos.Y = Ghs[i].PrevPos.Y;
					Ghs[i].PrevPos.X = Templ.X;
					Ghs[i].PrevPos.Y = Templ.Y;
					if(Scrn[Ghs[i].CurPos.Y] [Ghs[i].CurPos.X].Symbol == ChLine)
					{
						GameOver = 1;
					}
				}

				if(Scrn[Next.Y] [Ghs[i].CurPos.X].Symbol != ChBorder && Scrn[Ghs[i].CurPos.Y] [Next.X].Symbol == ChBorder)
				{
					Templ.Y = Ghs[i].CurPos.Y;
					Templ.X = Ghs[i].CurPos.X;
					Ghs[i].CurPos.Y = Next.Y;
					Ghs[i].CurPos.X = Ghs[i].PrevPos.X;
					Ghs[i].PrevPos.Y = Templ.Y;
					Ghs[i].PrevPos.X = Templ.X;
					if(Scrn[Ghs[i].CurPos.Y] [Ghs[i].CurPos.X].Symbol == ChLine)
					{
						GameOver = 1;
					}
				}

				if(Scrn[Next.Y] [Ghs[i].CurPos.X].Symbol == ChBorder && Scrn[Ghs[i].CurPos.Y] [Next.X].Symbol == ChBorder)
				{
					Templ.X = Ghs[i].CurPos.X;
					Templ.Y = Ghs[i].CurPos.Y;
					Ghs[i].CurPos.X = Ghs[i].PrevPos.X;
					Ghs[i].CurPos.Y = Ghs[i].PrevPos.Y;
					Ghs[i].PrevPos.X = Templ.X;
					Ghs[i].PrevPos.Y = Templ.Y;
					if(Scrn[Ghs[i].CurPos.Y] [Ghs[i].CurPos.X].Symbol == ChLine)
					{
						GameOver = 1;
					}
				}
				if(Scrn[Next.Y] [Ghs[i].CurPos.X].Symbol != ChBorder && Scrn[Ghs[i].CurPos.Y] [Next.X].Symbol != ChBorder)
				{
					COORD NextPos[2];

					NextPos[0].X = Next.X;
					NextPos[0].Y = Ghs[i].PrevPos.Y;

					NextPos[1].X = Ghs[i].PrevPos.X;
					NextPos[1].Y = Next.Y;

					srand( time(0) );
					unsigned int numb = (rand()%100+i)*(rand()%10+i)%2;
					Ghs[i].PrevPos.X = Ghs[i].CurPos.X;
					Ghs[i].PrevPos.Y = Ghs[i].CurPos.Y;
					Ghs[i].CurPos.X = NextPos[numb].X;
					Ghs[i].CurPos.Y = NextPos[numb].Y;
					if(Scrn[Ghs[i].CurPos.Y] [Ghs[i].CurPos.X].Symbol == ChLine)
					{
						GameOver = 1;
					}
				}
			}
			else
			{
				if(NextSymbol == ChGhost)
				{
					COORD NextPos[2];
					NextPos[0].X = Next.X;
					NextPos[0].Y = Ghs[i].PrevPos.Y;

					NextPos[1].X = Ghs[i].PrevPos.X;
					NextPos[1].Y = Next.Y;

					srand( time(0) );
					unsigned int numb = (rand()%100+i)*(rand()%10+i)%2;
					Ghs[i].PrevPos.X = Ghs[i].CurPos.X;
					Ghs[i].PrevPos.Y = Ghs[i].CurPos.Y;
					Ghs[i].CurPos.X = NextPos[numb].X;
					Ghs[i].CurPos.Y = NextPos[numb].Y;
					if(Scrn[Ghs[i].CurPos.Y] [Ghs[i].CurPos.X].Symbol == ChLine)
					{
						GameOver = 1;
					}
			
				}
				//else
				//{
					// остльные варианты столкновения
				//}
			}
		}
		Scrn[Ghs[i].CurPos.Y]  [Ghs[i].CurPos.X].Symbol = ChGhost;
		Scrn[Ghs[i].CurPos.Y]  [Ghs[i].CurPos.X].Color = TA.GhostTextAttr;
		if(Scrn[Ghs[i].PrevPos.Y] [Ghs[i].PrevPos.X].Symbol != ChLine) Scrn[Ghs[i].PrevPos.Y] [Ghs[i].PrevPos.X].Symbol = ChSpace;
		if(Scrn[Ghs[i].PrevPos.Y] [Ghs[i].PrevPos.X].Symbol == ChSpace) Scrn[Ghs[i].PrevPos.Y] [Ghs[i].PrevPos.X].Color = TA.BorderTextAttr;

		PrintChr(Ghs[i].PrevPos, Scrn[ Ghs[i].PrevPos.Y ][ Ghs[i].PrevPos.X ].Color, Scrn[ Ghs[i].PrevPos.Y ][ Ghs[i].PrevPos.X ].Symbol);

		PrintChr(Ghs[i].CurPos, Scrn[ Ghs[i].CurPos.Y ][ Ghs[i].CurPos.X ].Color, Scrn[ Ghs[i].CurPos.Y ][ Ghs[i].CurPos.X ].Symbol);

		if(GameOver) return GameOver;
	}
	return GameOver;
}

bool MoveHead(Ghost *Head, CharAttr Scrn[sizeY][sizeX], int Key, COORD &StartPos, COORD &TopLeft, COORD &BottRight, unsigned int &Score, unsigned int &FieldCount)
{
	//int i;
	COORD Next;
	bool GameOver = 0;
	
	if(Head->ID == ChBorder)
	{
		Scrn[Head->CurPos.Y][Head->CurPos.X].Symbol = Head->ID;
		PrintChr(Head->CurPos, TA.BorderTextAttr, Head->ID);

		Next.X = Head->CurPos.X;
		Next.Y = Head->CurPos.Y;
		switch(Key)
		{
		case KLeft:
			{
				if(Next.X>0)
				{
					Next.X = Next.X-1;
				}
				break;
			}
		case KRight:
			{
				if(Next.X<sizeX-1)
				{
					Next.X = Next.X+1;
				}
				break;
			}
		case KUp:
			{
				if(Next.Y>1)
				{
					Next.Y = Next.Y-1;
				}
				break;
			}
		case KDown:
			{
				if(Next.Y<sizeY-2)
				{
					Next.Y = Next.Y+1;
				}
				break;
			}
			
		}
		if((Scrn[Next.Y][Next.X].Symbol==ChSpace) || (Scrn[Next.Y][Next.X].Symbol==ChGhost))
		{
			Head->ID = ChLine;
			StartPos.X = Head->CurPos.X;
			StartPos.Y = Head->CurPos.Y;
			TopLeft.X=Next.X;
			TopLeft.Y=Next.Y;
			BottRight.X=Next.X;
			BottRight.Y=Next.Y;
			if(Scrn[Next.Y][Next.X].Symbol==ChGhost) GameOver=1;
		}
		else// Scrn[Next.Y][Next.X].Symbol==ChBorder
		{
			Head->PrevPos.X = Next.X;
			Head->PrevPos.Y = Next.Y;
		}
		Head->CurPos.X = Next.X;
		Head->CurPos.Y = Next.Y;
	}
	else // Head->ID = ChLine
	{
		Next.X = Head->CurPos.X-Head->PrevPos.X;// это пока только разница позиций X, а не следующая позиция
		Next.Y = Head->CurPos.Y-Head->PrevPos.Y;// это пока только разница позиций Y, а не следующая позиция
		switch(Key)
		{
		case KLeft:
			{
				if(Next.Y!=0)
				{
					Head->PrevPos.X = Head->CurPos.X + 1;
					Head->PrevPos.Y = Head->CurPos.Y;
				}
				break;
			}
		case KRight:
			{
				if(Next.Y!=0)
				{
					Head->PrevPos.X = Head->CurPos.X - 1;
					Head->PrevPos.Y = Head->CurPos.Y;
				}
				break;
			}
		case KUp:
			{
				if(Next.X!=0)
				{
					Head->PrevPos.X = Head->CurPos.X;
					Head->PrevPos.Y = Head->CurPos.Y + 1;
				}
				break;
			}
		case KDown:
			{
				if(Next.X!=0)
				{
					Head->PrevPos.X = Head->CurPos.X;
					Head->PrevPos.Y = Head->CurPos.Y - 1;
				}
				break;
			}			
		}
		Next.X = 2 * Head->CurPos.X - Head->PrevPos.X;
		Next.Y = 2 * Head->CurPos.Y - Head->PrevPos.Y;
		Head->PrevPos.X = Head->CurPos.X;
		Head->PrevPos.Y = Head->CurPos.Y;
		Head->CurPos.X = Next.X;
		Head->CurPos.Y = Next.Y;

		if(TopLeft.X>Head->CurPos.X) TopLeft.X=Head->CurPos.X;
		if(TopLeft.Y>Head->CurPos.Y) TopLeft.Y=Head->CurPos.Y;

		if(BottRight.X<Head->CurPos.X) BottRight.X=Head->CurPos.X;
		if(BottRight.Y<Head->CurPos.Y) BottRight.Y=Head->CurPos.Y;


		Scrn[Head->PrevPos.Y][Head->PrevPos.X].Color=TA.LineTextAttr;
		Scrn[Head->PrevPos.Y][Head->PrevPos.X].Symbol = ChLine;
		PrintChr(Head->PrevPos, Scrn[Head->PrevPos.Y][Head->PrevPos.X].Color, ChLine);
		if(Scrn[Head->CurPos.Y][Head->CurPos.X].Symbol==ChBorder)
		{
			Head->ID = ChBorder;
			Scrn[Head->CurPos.Y][Head->CurPos.X].Color=TA.HeadTextAttr;
			Filling(Scrn, &(*Head),  Score, FieldCount);
		}
		else
		{
			if(Scrn[Head->CurPos.Y][Head->CurPos.X].Symbol==ChGhost) GameOver = 1;
			Scrn[Head->CurPos.Y][Head->CurPos.X].Symbol=ChLine;
			Scrn[Head->CurPos.Y][Head->CurPos.X].Color=TA.LineTextAttr;
		}
	}

	Scrn[Head->CurPos.Y][Head->CurPos.X].Symbol = Head->ID;
	PrintChr(Head->CurPos, TA.HeadTextAttr, ChHead);

	return GameOver;
}

bool TryMsBox(COORD SizeWin, CharAttr Scrn[sizeY][sizeX], int TryCnt)
{
	const int MB_N = 5;
	const int MB_M = 35;

	int i,j;

	char MB[MB_N][MB_M] =  {"                                  ",///
							"  Неудача. Нажмите любую кнопку,  ",///
							"      чтобы продолжить игру.      ",///
							"      Осталось попыток - Х        ",///
							"                                  "};

	COORD Pos;

	SetConsoleOutputCP(1251);
	if(TryCnt < 0)
	{
		SizeWin.X = (SizeWin.X-MB_M+10)/2;
		SizeWin.Y = SizeWin.Y/2-2;
		PrintStr(SizeWin, TA.LightSelTextAttr, "                          ");
		SizeWin.Y++;
		PrintStr(SizeWin, NULL, "    Вы прошли уровень!    ");
		SizeWin.Y++;
		PrintStr(SizeWin, NULL, "                          ");
		SetConsoleTextAttribute(handle, TA.BorderTextAttr); 
		getch();
	}
	if(TryCnt > 0)
	{
		SizeWin.X = (SizeWin.X-MB_M+1)/2;
		SizeWin.Y = SizeWin.Y/2-3;
	
		Pos.X = SizeWin.X;
		SetConsoleTextAttribute(handle, TA.LightSelTextAttr);
		for(i = 0;i<MB_N;i++)
		{
			Pos.Y = SizeWin.Y+i;
			PrintStr(Pos, NULL, MB[i]);
		}
		SetConsoleOutputCP(CodePage);
		Pos.X = SizeWin.X+25;
		Pos.Y = SizeWin.Y+3;
		GotoXY(Pos);
		SetConsoleTextAttribute(handle, TA.XTextAttr);
		printf("%d", TryCnt);
		SetConsoleTextAttribute(handle, TA.LightSelTextAttr);
		getch();
		
		for(i = 0;i<MB_N;i++)
		{
			for(j = 0;j<MB_M;j++)
			{
				Pos.X = j+SizeWin.X;
				Pos.Y = i+SizeWin.Y;
				PrintChr(Pos, Scrn[Pos.Y][Pos.X].Color, Scrn[Pos.Y][Pos.X].Symbol);

			}
		}
		SetConsoleTextAttribute(handle, TA.BorderTextAttr); 
		//SetConsoleOutputCP(CodePage);
	}
	if(TryCnt == 0)
	{
		SizeWin.X = (SizeWin.X-MB_M+10)/2;
		SizeWin.Y = SizeWin.Y/2-2;
		PrintStr(SizeWin, TA.LightSelTextAttr, "                          ");
		SizeWin.Y++;
		PrintStr(SizeWin, NULL, "       Вы проиграли       ");
		SizeWin.Y++;
		PrintStr(SizeWin, NULL, "                          ");
		SetConsoleTextAttribute(handle, TA.BorderTextAttr); 
		getch();
	}
	return 0;
}

bool EscMsBox(COORD TopLeft, CharAttr *Scrn)
{
	bool Exit = 0;
	int i, j, Key = KDefault;
	const int MB_N = 5;
	const int MB_M = 35;
	char MB[MB_N][MB_M] =  {"                                  ",///
							"  Завершить игру и выйти в меню?  ",///
							"                                  ",///
							"             Да  Нет              ",///
							"                                  "};
	char WYes[] = " Да ";
	char WNo[] = " Нет ";
	COORD Pos,Yes,No;

	SetConsoleOutputCP(1251);

	TopLeft.X = (TopLeft.X-MB_M+1)/2;
	TopLeft.Y = TopLeft.Y/2-3;
	
	Pos.X = TopLeft.X;
	SetConsoleTextAttribute(handle, TA.LightSelTextAttr);
	for(i = 0;i<MB_N;i++)
	{
		Pos.Y = TopLeft.Y+i;
		PrintStr(Pos, NULL, MB[i]);
	}
	Yes.X = TopLeft.X+12;
	Yes.Y = TopLeft.Y+3;
	No.X = TopLeft.X+16;
	No.Y = TopLeft.Y+3;
	PrintStr(No, TA.LightTextAttr, WNo);
	SetConsoleTextAttribute(handle, TA.LightSelTextAttr);

	while(Key!=KEnter)
	{
		Key = getch();
		switch(Key)
		{
		case KLeft:
			{
				PrintStr(Yes, TA.LightTextAttr, WYes);
				PrintStr(No, TA.LightSelTextAttr, WNo);
				Exit = 1;
				break;
			}
		case KRight:
			{
				PrintStr(No, TA.LightTextAttr, WNo);
				PrintStr(Yes, TA.LightSelTextAttr, WYes);
				Exit = 0;
				break;
			}
		case KEnter:
			{
				SetConsoleOutputCP(CodePage);
				if(Exit)
				{
					SetConsoleTextAttribute(handle, TA.BorderTextAttr);
					
					return Exit;
				}

				for(i = 0;i<MB_N;i++)
				{
					for(j = 0;j<MB_M;j++)
					{
						Pos.X = j+TopLeft.X;
						Pos.Y = i+TopLeft.Y;
						PrintChr(Pos, (Scrn+Pos.X+Pos.Y*sizeX)->Color, (Scrn+Pos.X+Pos.Y*sizeX)->Symbol);
					}
				}
				SetConsoleTextAttribute(handle, TA.BorderTextAttr); 
				SetConsoleOutputCP(CodePage);
				return Exit;
			}
		case KEsc:
			{
				SetConsoleOutputCP(CodePage);
				for(i = 0;i<MB_N;i++)
				{
					for(j = 0;j<MB_M;j++)
					{
						Pos.X = j+TopLeft.X;
						Pos.Y = i+TopLeft.Y;
						PrintChr(Pos, (Scrn+Pos.X+Pos.Y*sizeX)->Color, (Scrn+Pos.X+Pos.Y*sizeX)->Symbol);
					}
				}
				SetConsoleTextAttribute(handle, TA.BorderTextAttr);
				
				return Exit;
			}
		
		}
	};
	SetConsoleOutputCP(CodePage);
	return Exit;

}

void PrintChr(COORD Pos, unsigned char Clr, unsigned char Smbl)
{
	GotoXY(Pos);
	SetConsoleTextAttribute( handle, Clr);
	putchar(Smbl);
}

void PrintStr(COORD Pos, unsigned char Clr, char *Str)
{
	GotoXY(Pos);
	if(Clr!=NULL) SetConsoleTextAttribute( handle, Clr);
	printf(Str);
}

void Filling(CharAttr Scrn[sizeY][sizeX], Ghost *Ghs, unsigned int &Score, unsigned int &FieldCounter)
{
	COORD Pos;
	
	unsigned short int i,j,k;
	unsigned char isFieldCountUse=0;
	unsigned int FieldCount=1;
	for(j=2;j<sizeY-2;j++)
	{
		
		for(i=1;i<sizeX-1;i++)
		{
			// текущий символ: пробел или "муха"...
			if(Scrn[j][i].Symbol == ChSpace || Scrn[j][i].Symbol == ChGhost) 
			{
				if(Scrn[j][i].Symbol == ChSpace)
				{
					
				}
				// верхний символ: пробел или "муха"...
				if(Scrn[j-1][i].Symbol == ChSpace || Scrn[j-1][i].Symbol == ChGhost)
				{
					// устанавливаем ID поля как в верхнем
					Scrn[j][i].FieldID = Scrn[j-1][i].FieldID;
					// предыдущий: пробел или "муха"...
					if(Scrn[j][i-1].Symbol == ChSpace || Scrn[j][i-1].Symbol == ChGhost)
					{
						// ID области(поля) предыдущего символа меньше чем верхнего символа
						if(Scrn[j][i-1].FieldID < Scrn[j][i].FieldID)
						{
							// устанавливаем наименьший ID
							Scrn[j][i].FieldID = Scrn[j][i-1].FieldID;
							Scrn[j-1][i].FieldID = Scrn[j][i-1].FieldID;
						}
					}
					
				}
				else// верхний символ: не пробел и не "муха"
				{
					// предыдущий: пробел или "муха"...
					if(Scrn[j][i-1].Symbol == ChSpace || Scrn[j][i-1].Symbol == ChGhost)
					{
						// устанавливаем ID поля как в предыдущем
						Scrn[j][i].FieldID = Scrn[j][i-1].FieldID;
					}
					else// предыдущий: не пробел и не "муха"
					{
						// устанавливаем ID поля из FieldCount
						Scrn[j][i].FieldID=FieldCount;
						// делаем отметку что этот ID исполоьзовался
						isFieldCountUse=1;
					}
					
				}
			}// текущий символ: пробел или "муха"...
			// следующий символ: линия или бордюр
			if(Scrn[j][i+1].Symbol == ChLine || Scrn[j][i+1].Symbol == ChBorder)
			{
				// такой ID использовался...
				if(isFieldCountUse && i<sizeX-2)
				{
					// ... увеличиваем счётчик ID
					FieldCount++;
					// делаем отметку что этот ID не исполоьзовался
					isFieldCountUse=0;
				}
			}
		}// for(i=1;i<sizeX-1;i++)
	}// for(j=2;j<sizeY-2;j++)

	isFieldCountUse=0;// теперь это индикатор соседства двух полей с разным ID (соседства не через линию)
	do
	{

		for(i=sizeX-2;i>0;i--)
		{
		
			for(j=2;j<sizeY-2;j++)
			{
				// текущий символ: пробел или "муха"...
				if(Scrn[j][i].Symbol == ChSpace || Scrn[j][i].Symbol == ChGhost)
				{
					// правый символ: пробел или "муха"...
					if(Scrn[j][i+1].Symbol == ChSpace || Scrn[j][i+1].Symbol == ChGhost)
					{
						// ID текущего больше чем справа...
						if(Scrn[j][i].FieldID > Scrn[j][i+1].FieldID)
						{
							isFieldCountUse=1;
							// заменяем ID в текущем на меньший
							Scrn[j][i].FieldID = Scrn[j][i+1].FieldID;
							// верхний символ: пробел или "муха"...
							if(Scrn[j-1][i].Symbol == ChSpace || Scrn[j-1][i].Symbol == ChGhost)
							{
								// ID текущего больше чем сверху...
								if(Scrn[j][i].FieldID > Scrn[j-1][i].FieldID)
								{
									//isFieldCountUse=1;
									Scrn[j][i].FieldID = Scrn[j-1][i].FieldID;
									Scrn[j][i+1].FieldID = Scrn[j-1][i].FieldID;
								}
								else
								{
									// ID текущего меньше чем сверху...
									if(Scrn[j][i].FieldID < Scrn[j-1][i].FieldID)
									{
										//isFieldCountUse=1;
										Scrn[j-1][i].FieldID = Scrn[j][i].FieldID;
									}
								}
							}
							
						}
						else
						{
							// ID текущего меньше чем справа...
							if(Scrn[j][i].FieldID < Scrn[j][i+1].FieldID)
							{
								isFieldCountUse=1;
								// заменяем ID в правом на меньший
								Scrn[j][i+1].FieldID = Scrn[j][i].FieldID;
							}
							
						}
					}
					else// правый символ: не пробел и не "муха"...
					{
						// верхний символ: пробел или "муха"...
						if(Scrn[j-1][i].Symbol == ChSpace || Scrn[j-1][i].Symbol == ChGhost)
						{
							// ID текущего больше чем сверху...
							if(Scrn[j][i].FieldID > Scrn[j-1][i].FieldID)
							{
								isFieldCountUse=1;
								Scrn[j][i].FieldID = Scrn[j-1][i].FieldID;
								
							}
							else
							{
								// ID текущего меньше чем сверху...
								if(Scrn[j][i].FieldID < Scrn[j-1][i].FieldID)
								{
									isFieldCountUse=1;
									Scrn[j-1][i].FieldID = Scrn[j][i].FieldID;
									
								}
							}
						}
					}
				}// текущий символ: пробел или "муха"...
			}// for(j=...)
		}//for(i=...)

		if(isFieldCountUse == 3) break;

		for(j=sizeY-3;j>1;j--)
		{
		
			for(i=sizeX-2;i>0;i--)
			{
				// текущий символ: пробел или "муха"...
				if(Scrn[j][i].Symbol == ChSpace || Scrn[j][i].Symbol == ChGhost)
				{
					// нижний символ: пробел или "муха"...
					if(Scrn[j+1][i].Symbol == ChSpace || Scrn[j+1][i].Symbol == ChGhost)
					{
						// ID текущего больше чем снизу...
						if(Scrn[j][i].FieldID > Scrn[j+1][i].FieldID)
						{
							isFieldCountUse=2;
							// заменяем ID в текущем на меньший
							Scrn[j][i].FieldID = Scrn[j+1][i].FieldID;
							// правый символ: пробел или "муха"...
							if(Scrn[j][i+1].Symbol == ChSpace || Scrn[j][i+1].Symbol == ChGhost)
							{
								// ID текущего больше чем справа...
								if(Scrn[j][i].FieldID > Scrn[j][i+1].FieldID)
								{
									isFieldCountUse=2;
									Scrn[j][i].FieldID = Scrn[j][i+1].FieldID;
									Scrn[j+1][i].FieldID = Scrn[j][i+1].FieldID;
								}
								else
								{
									// ID текущего меньше чем справа...
									if(Scrn[j][i].FieldID < Scrn[j][i+1].FieldID)
									{
										isFieldCountUse=2;
										Scrn[j][i+1].FieldID = Scrn[j][i].FieldID;
									}
								}
							}

						}
						else
						{
							// ID текущего меньше чем снизу...
							if(Scrn[j][i].FieldID < Scrn[j+1][i].FieldID)
							{
								isFieldCountUse=2;
								// заменяем ID в нижнем на меньший
								Scrn[j+1][i].FieldID = Scrn[j][i].FieldID;
							}
						}
					}
					else// нижний символ: не пробел и не "муха"...
					{
						// правый символ: пробел или "муха"...
						if(Scrn[j][i+1].Symbol == ChSpace || Scrn[j][i+1].Symbol == ChGhost)
						{
							// ID текущего больше чем справа...
							if(Scrn[j][i].FieldID > Scrn[j][i+1].FieldID)
							{
								isFieldCountUse=2;
								Scrn[j][i].FieldID = Scrn[j][i+1].FieldID;
							}
							else
							{
								// ID текущего меньше чем справа...
								if(Scrn[j][i].FieldID < Scrn[j][i+1].FieldID)
								{
									isFieldCountUse=2;
									Scrn[j][i+1].FieldID = Scrn[j][i].FieldID;
								}
							}
						}
					}
				}// текущий символ: пробел или "муха"...
			}// for(i=...)
		}//for(j=...)
		
		if(isFieldCountUse == 4) break;

		for(i=1;i<sizeX-1;i++)
		{
		
			for(j=sizeY-3;j>1;j--)
			{
				// текущий символ: пробел или "муха"...
				if(Scrn[j][i].Symbol == ChSpace || Scrn[j][i].Symbol == ChGhost)
				{
					// левый символ: пробел или "муха"...
					if(Scrn[j][i-1].Symbol == ChSpace || Scrn[j][i-1].Symbol == ChGhost)
					{
						// ID текущего больше чем слева...
						if(Scrn[j][i].FieldID > Scrn[j][i-1].FieldID)
						{
							isFieldCountUse=3;
							// заменяем ID в текущем на меньший
							Scrn[j][i].FieldID = Scrn[j][i-1].FieldID;
							// нижний символ: пробел или "муха"...
							if(Scrn[j+1][i].Symbol == ChSpace || Scrn[j+1][i].Symbol == ChGhost)
							{
								// ID текущего больше чем снизу...
								if(Scrn[j][i].FieldID > Scrn[j+1][i].FieldID)
								{
									isFieldCountUse=3;
									Scrn[j][i].FieldID = Scrn[j+1][i].FieldID;
									Scrn[j][i-1].FieldID = Scrn[j+1][i].FieldID;
								}
								else
								{
									// ID текущего меньше чем снизу...
									if(Scrn[j][i].FieldID < Scrn[j+1][i].FieldID)
									{
										isFieldCountUse=3;
										Scrn[j+1][i].FieldID = Scrn[j][i].FieldID;
									}
								}
							}

						}
						else
						{
							// ID текущего меньше чем слева...
							if(Scrn[j][i].FieldID < Scrn[j][i-1].FieldID)
							{
								isFieldCountUse=3;
								// заменяем ID в левом на меньший
								Scrn[j][i-1].FieldID = Scrn[j][i].FieldID;
							}
						}
					}
					else// левый символ: не пробел и не "муха"...
					{
						// нижний символ: пробел или "муха"...
						if(Scrn[j+1][i].Symbol == ChSpace || Scrn[j+1][i].Symbol == ChGhost)
						{
							// ID текущего больше чем снизу...
							if(Scrn[j][i].FieldID > Scrn[j+1][i].FieldID)
							{
								isFieldCountUse=3;
								Scrn[j][i].FieldID = Scrn[j+1][i].FieldID;
							}
							else
							{
								// ID текущего меньше чем снизу...
								if(Scrn[j][i].FieldID < Scrn[j+1][i].FieldID)
								{
									isFieldCountUse=3;
									Scrn[j+1][i].FieldID = Scrn[j][i].FieldID;
								}
							}
						}
					}
				}// текущий символ: пробел или "муха"...
			}// for(j=...)
		}//for(i=...)

		if(isFieldCountUse == 1) break;

		for(j=2;j<sizeY-2;j++)
		{
		
			for(i=1;i<sizeX-1;i++)
			{
				// текущий символ: пробел или "муха"...
				if(Scrn[j][i].Symbol == ChSpace || Scrn[j][i].Symbol == ChGhost)
				{
					// верхний символ: пробел или "муха"...
					if(Scrn[j-1][i].Symbol == ChSpace || Scrn[j-1][i].Symbol == ChGhost)
					{
						// ID текущего больше чем сверху...
						if(Scrn[j][i].FieldID > Scrn[j-1][i].FieldID)
						{
							isFieldCountUse=4;
							// заменяем ID в текущем на меньший
							Scrn[j][i].FieldID = Scrn[j-1][i].FieldID;
							// левый символ: пробел или "муха"...
							if(Scrn[j][i-1].Symbol == ChSpace || Scrn[j][i-1].Symbol == ChGhost)
							{
								// ID текущего больше чем слева...
								if(Scrn[j][i].FieldID > Scrn[j][i-1].FieldID)
								{
									isFieldCountUse=4;
									Scrn[j][i].FieldID = Scrn[j][i-1].FieldID;
									Scrn[j-1][i].FieldID = Scrn[j][i-1].FieldID;
								}
								else
								{
									// ID текущего меньше чем слева...
									if(Scrn[j][i].FieldID < Scrn[j][i-1].FieldID)
									{
										isFieldCountUse=4;
										Scrn[j][i-1].FieldID = Scrn[j][i].FieldID;
									}
								}
							}

						}
						else
						{
							// ID текущего меньше чем сверху...
							if(Scrn[j][i].FieldID < Scrn[j-1][i].FieldID)
							{
								isFieldCountUse=4;
								// заменяем ID в верхнем на меньший
								Scrn[j-1][i].FieldID = Scrn[j][i].FieldID;
							}
						}
					}
					else// верхний символ: не пробел и не "муха"...
					{
						// левый символ: пробел или "муха"...
						if(Scrn[j][i-1].Symbol == ChSpace || Scrn[j][i-1].Symbol == ChGhost)
						{
							// ID текущего больше чем слева...
							if(Scrn[j][i].FieldID > Scrn[j][i-1].FieldID)
							{
								isFieldCountUse=4;
								Scrn[j][i].FieldID = Scrn[j][i-1].FieldID;
							}
							else
							{
								// ID текущего меньше чем слева...
								if(Scrn[j][i].FieldID < Scrn[j][i-1].FieldID)
								{
									isFieldCountUse=4;
									Scrn[j][i-1].FieldID = Scrn[j][i].FieldID;
								}
							}
						}
					}
				}// текущий символ: пробел или "муха"...
			}// for(i=...)
		}//for(j=...)
		
		if(isFieldCountUse == 2) break;
	}
	while(isFieldCountUse);

	unsigned short int curField=0;
	for(j=2;j<sizeY-2;j++)
	{
		for(i=1;i<sizeX-1;i++)
		{
			if(Scrn[j][i].Symbol == ChSpace)
			{

				if(Scrn[j][i].FieldID == curField) continue;
				else
				{
					for(k=1; (Ghs+k)->ID>0; k++)
					{
					
						if(Scrn[j][i].FieldID == Scrn[ (Ghs+k)->CurPos.Y ][ (Ghs+k)->CurPos.X ].FieldID) 
						{
							curField=Scrn[j][i].FieldID;
							break;
						}
					}
					if((Ghs+k)->ID<0)
					{
						Scrn[j][i].FieldID = 0;
						Pos.X=i;
						Pos.Y=j;
						Scrn[j][i].Symbol = ChBorder;
						Scrn[j][i].Color = TA.BorderTextAttr;
						PrintChr(Pos, TA.BorderTextAttr, ChBorder);
						Score++;
						FieldCounter++;
						
					}
				}
			}
			if(Scrn[j][i].Symbol == ChLine)
			{
				FieldCounter++;
				Pos.X=i;
				Pos.Y=j;
				Scrn[j][i].Symbol = ChBorder;
				Scrn[j][i].Color = TA.BorderTextAttr;
				PrintChr(Pos, TA.BorderTextAttr, ChBorder);
				
			}
			

		}
	}

	char strScore[20]="Очки: ";
	SetConsoleOutputCP(1251);
	Pos.X=35;
	Pos.Y=0;
	PrintStr(Pos, TA.XTextAttr, strScore);
	SetConsoleOutputCP(CodePage);
	Pos.X=35 + strlen(strScore);
	itoa(Score,strScore,10);
	PrintStr(Pos, TA.XTextAttr, strScore);
				
}

void ReadRecords(char NameList[StrCount][LenStr], unsigned int ScoreList[StrCount])
{
	int i,j,k;

	FILE* RecFile;

	char RecFileName[]="levels\\records.txt";
	char str[LenStr], str2[LenStr];

	RecFile = fopen (RecFileName,"r");
	if(RecFile != 0)
	{
		for(i=0; i<StrCount; i++)
		{ 
			fgets(str, LenStr, RecFile);
			for(j=0; str[j] != '\t'; j++)
			{
				NameList[i][j]=str[j];
			}
			NameList[i][j++]='\0';
			
			for(k=0; str[j]>47 && str[j]<58;k++, j++)
			{
				str2[k]=str[j];
			}
			str2[k]='\0';
			ScoreList[i] = atoi(str2);
		}
		
		fclose(RecFile);
	}

}

void Records(COORD &SizeWin, unsigned int &Score,char NameList[StrCount][LenStr], unsigned int ScoreList[StrCount])
{
	short int i;
	unsigned int tmp;
	char Name[21]="Вы",str[21]="";

	mciSendString(L"play sound\\рекорды.mp3 repeat",0,0,0);
	system("cls");
	
	if(Score==0)
	{
		ShowRecords(SizeWin, NameList, ScoreList, Score, -1);
	}
	else
	{
		if(ScoreList[StrCount-1]<Score)
		{
			ScoreList[StrCount-1] = Score;
			strcpy(NameList[StrCount-1], Name);
			for(i=StrCount-2; (ScoreList[i]<Score) && (i>-1); i--)
			{
				tmp=ScoreList[i];
				ScoreList[i]=ScoreList[i+1];
				ScoreList[i+1]=tmp;

				strcpy(str,NameList[i]);
				strcpy(NameList[i],NameList[i+1]);
				strcpy(NameList[i+1],str);
			}
		}
		ShowRecords(SizeWin, NameList, ScoreList, Score, i+1);
	}
	int Key=KDefault;
	while(1)
	{
		Key=getch();
		if(Key == KEsc || Key == KEnter)
		{
			system("cls");
			mciSendString(L"stop sound\\рекорды.mp3",0,0,0);
			Score=0;
			return;
		}
	}
	
}

void ShowRecords(COORD SizeWin, char NameList[StrCount][LenStr],unsigned int  ScoreList[StrCount], unsigned int &Score, short int Index)
{
	int i, Y;
	COORD Pos;

	SizeWin.Y=(SizeWin.Y-20)/2;
	SizeWin.X=SizeWin.X/2-9;
	SetConsoleTextAttribute(handle,TA.BorderTextAttr);
	for(i=0; i<StrCount; i++)
	{
		Pos.X=SizeWin.X-14;
		Pos.Y=SizeWin.Y+2*i;
		GotoXY(Pos);
		if(Index == i)
		{
			Y=Pos.Y;
			SetConsoleTextAttribute(handle,TA.GhostTextAttr);
			
			printf(">>|                      %d  |<<", ScoreList[i]);

			SetConsoleTextAttribute(handle,TA.BorderTextAttr);
		}
		else printf("   %20s  %d",NameList[i],ScoreList[i]);
	}
	if(Index>-1)
	{
		Pos.X = SizeWin.X+8;
		Pos.Y = Y;
		SetName( SizeWin, Pos, NameList[Index]);
		SaveRecords(NameList, ScoreList);
		Pos.X += 9;
		PrintStr(Pos, TA.LineTextAttr,"   ");
		Pos.X -= 31;
		PrintStr(Pos, TA.LineTextAttr,"   ");
	}
	Pos.X=SizeWin.X-13;
	Pos.Y=SizeWin.Y+22;
	PrintStr(Pos, TA.LineTextAttr, "Нажмите 'Esc' или 'Enter', чтобы выйти в меню");
	SetConsoleTextAttribute(handle,TA.BorderTextAttr);
}

void SetName(COORD SizeWin, COORD Pos, char Str[21])
{
	unsigned int Symbol = 0;
	int i;
	COORD PosEnt;

	PosEnt.X = SizeWin.X-13;
	PosEnt.Y = SizeWin.Y+22;
	PrintStr(PosEnt, TA.HeadTextAttr, "Нажмите 'Enter' по завершении ввода имени");

	// показываем курсор.
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo( handle, &CursorInfo );
	CursorInfo.bVisible = TRUE;
	SetConsoleCursorInfo( handle, &CursorInfo );

	Str[0]='\0';
	PosEnt.X = Pos.X+1;
	PosEnt.Y = Pos.Y;
	i=0;
	do{
		GotoXY(Pos);
		Symbol = getch();

		if( (Symbol>31 && Symbol<127) || (Symbol>191 && Symbol<256) || Symbol==KBackspace)
		{
			if(Symbol==KBackspace && i>0)
			{
				//PosEnt.X = Pos.X-i+1;
				PrintChr(PosEnt, TA.HeadTextAttr, ' ');
				Str[--i]='\0';
				PosEnt.X++;
				PrintStr(PosEnt, TA.HeadTextAttr, Str);
				
				continue;
			}
			if(i<20)
			{
				Str[i++] = Symbol;
				Str[i] = '\0';
				PosEnt.X--;
				PrintStr(PosEnt, TA.HeadTextAttr, Str);
				
			}
		}
	}while(Symbol != KEnter);
	PrintStr(PosEnt, TA.GhostTextAttr, Str);
	// прячем курсор
	CursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo( handle, &CursorInfo );
}

void SaveRecords(char NameList[StrCount][LenStr], unsigned int  ScoreList[StrCount])
{
	FILE* RecFile;
	char RecFileName[]="levels\\records.txt";
	char str[LenStr];
	char strNum[LenStr];
	int i;
	
	RecFile = fopen (RecFileName,"r+");
	if(RecFile != 0)
	{
		for(i=0; i<StrCount; i++)
		{
			strcpy(str, NameList[i]);
			strcat(str, "\t");
			itoa( ScoreList[i], strNum, 10);
			strcat(str, strNum);
			strcat(str, "\n");
			fputs(str, RecFile);
		}
		
		fclose(RecFile);
	}
}

void Settings(COORD SizeWin)
{
	const int IQ = 2;// количество строк меню
	const int IL = 17;// длина строки меню
	TextAttribute TMP_TA;

	char Items[IQ][IL] = {"   Тёмный фон   ","   Светлый фон  "};
	char str[10][LenStr] = {"Обычный текст",
							"Выделенный текст",
							"Светлый текст",
							"Светлый выделенный текст",
							"Текст игровой информации",
							"х - муха",
							"х - тёмная муха",
							"х - голова",
							"ххх - линия",
							"ххх - бордюр"};
	str[5][0]=ChGhost;
	str[6][0]=ChGhost;
	str[7][0]=ChHead;
	str[8][0]=ChLine; str[8][1]=ChLine; str[8][2]=ChLine; 
	str[9][0]=ChBorder; str[8][1]=ChBorder; str[8][2]=ChBorder; 

	/*LTA.BorderTextAttr = 0x8f;
	LTA.SelTextAttr = 0xf8;
	LTA.LightTextAttr = 0x80;
	LTA.LightSelTextAttr = 0xf0;
	LTA.XTextAttr = 0x85;
	LTA.GhostTextAttr = 0x8e;
	LTA.DarkGhostTextAttr = 0x86;
	LTA.HeadTextAttr = 0x80;
	LTA.LineTextAttr = 0x81;
	LTA.TextAttr = 0x80;*/

	COORD Pos,PosMenu;
	int Key = KDefault, CurIt, i, j;
	PosMenu.X = (SizeWin.X/2-IL+1)/2;// = 11
	PosMenu.Y = SizeWin.Y/2-1;// = 15

	system("cls");
	if(TA.BorderTextAttr == DTA.BorderTextAttr) 
	{
		PrintStr(PosMenu, TA.SelTextAttr, Items[0]);
		PosMenu.Y +=2;
		PrintStr(PosMenu, TA.TextAttr, Items[1]);
		CurIt=0;
	}
	else
	{
		PrintStr(PosMenu, TA.TextAttr, Items[0]);
		PosMenu.Y +=2;
		PrintStr(PosMenu, TA.SelTextAttr, Items[1]);
		CurIt=1;
	}
	TMP_TA=TA;
	while(!(Key==KEnter))
	{
		Pos.X = SizeWin.X/2+5;
		Pos.Y = 6;
		PrintStr( Pos, TMP_TA.TextAttr, str[0]);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.SelTextAttr, str[1]);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.LightTextAttr, str[2]);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.LightSelTextAttr, str[3]);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.XTextAttr, str[4]);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.GhostTextAttr, str[5]);
		SetConsoleOutputCP(CodePage);
		PrintChr( Pos, TMP_TA.GhostTextAttr, str[5][0]);
		SetConsoleOutputCP(1251);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.DarkGhostTextAttr, str[6]);
		SetConsoleOutputCP(CodePage);
		PrintChr( Pos, TMP_TA.DarkGhostTextAttr, str[6][0]);
		SetConsoleOutputCP(1251);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.HeadTextAttr, str[7]);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.LineTextAttr, str[8]);
		SetConsoleOutputCP(CodePage);
		PrintChr( Pos, TMP_TA.LineTextAttr, str[8][0]); Pos.X++;
		PrintChr( Pos, TMP_TA.LineTextAttr, str[8][0]); Pos.X++;
		PrintChr( Pos, TMP_TA.LineTextAttr, str[8][0]); Pos.X-=2;
		SetConsoleOutputCP(1251);
		Pos.Y += 2;
		PrintStr( Pos, TMP_TA.BorderTextAttr, str[9]);
		SetConsoleOutputCP(CodePage);
		PrintChr( Pos, TMP_TA.BorderTextAttr, str[9][0]); Pos.X++;
		PrintChr( Pos, TMP_TA.BorderTextAttr, str[9][0]); Pos.X++;
		PrintChr( Pos, TMP_TA.BorderTextAttr, str[9][0]); Pos.X-=2;
		SetConsoleOutputCP(1251);

		Key = getch();

		switch(Key)
		{
			case KUp :
			{
				CurIt=(CurIt+1)%2;
				break;
			}
			case KDown :
			{
				CurIt=(CurIt+1)%2;
				break;
			}
			case KEnter:
			{
				
				break;
			}
		}
		if(CurIt == 0)
		{
			TMP_TA = DTA;
			PosMenu.Y -=2;
			PrintStr(PosMenu, TA.SelTextAttr, Items[0]);
			PosMenu.Y +=2;
			PrintStr(PosMenu, TA.TextAttr, Items[1]);
		}
		else
		{
			TMP_TA = LTA;
			PosMenu.Y -=2;
			PrintStr(PosMenu, TA.TextAttr, Items[0]);
			PosMenu.Y +=2;
			PrintStr(PosMenu, TA.SelTextAttr, Items[1]);
		}
		COORD Posss,Size;
		Posss.X = SizeWin.X/2;
		Posss.Y = 1;
		Size.X = SizeWin.X-1-Posss.X;
		Size.Y = SizeWin.Y-1-Posss.Y;
		SetConsoleTextAttribute(handle, TMP_TA.BorderTextAttr);
		//SetConsoleTextAttribute(handle, TMP_TA.LightSelTextAttr);  
		ClearRect(SizeWin, Posss, Size);
		/*for(j=1; j<SizeWin.Y-1; j++)
		{
			Pos.Y=j;
			for(i=SizeWin.X/2; i<SizeWin.X-1; i++)
			{
				Pos.X=i;
				PrintChr(Pos, TMP_TA.BorderTextAttr, ' ');
			}
		}*/
		
	}
	TA=TMP_TA;
	SetConsoleTextAttribute(handle, TA.BorderTextAttr);
	system("cls");
}

void ClearRect(COORD SizeWin, COORD Pos, COORD Size)
{
	
	int i, j;
	COORD Pos2=Pos;
	char *str=(char *)malloc(sizeof(char)*(SizeWin.X+1));
	str[0]='\0';

	GotoXY(Pos2);
	for(i=Pos2.Y; i<Size.Y+1; i++)
	{
		for(j=0; j<Size.X-Pos2.X; j++)
		{
			str[j]='*';
		}
		str[j]='\0';
		printf(str);
		Pos2.Y++;
		GotoXY(Pos2);
	}
	free(str);
	GotoXY(Pos);
}

void GetCurPos(COORD *Pos)
{
	CONSOLE_SCREEN_BUFFER_INFO CSBI;
	GetConsoleScreenBufferInfo(handle, &CSBI);
	*Pos=CSBI.dwCursorPosition;
}