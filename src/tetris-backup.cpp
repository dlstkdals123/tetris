#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <Windows.h>
#include <time.h>

#include <iostream>
#include "position.h"
#include "rotation.h"
#include "Stage.h"
#include "Color.h"
#include "gameState.h"

using namespace std;

//*********************************
//상수 선언
//*********************************

#define EXT_KEY			0xffffffe0	//확장키 인식값 
#define KEY_LEFT		0x4b
#define KEY_RIGHT		0x4d
#define KEY_UP			0x48
#define KEY_DOWN		0x50

//*********************************
//전역변수선언
//*********************************

gameState gamestate;	// 게임 상태 관리하는 객체
Position board_offset;	//화면중 블럭이 나타나는 좌표의 절대위치 (렌더링 원점)
int block_shape;
Rotation block_rotation;	//현재 블럭의 회전 상태
Position block_pos;	//현재 블럭의 위치
int next_block_shape;
char total_block[21][14];		//화면에 표시되는 블럭들
char block[7][4][4][4]={
	//막대모양
	1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,	1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,	1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,	1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,

	//네모모양
	1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,	1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,	1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,	1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,

	//'ㅓ' 모양
	0,1,0,0,1,1,0,0,0,1,0,0,0,0,0,0,	1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,	1,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,	0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,

	//'ㄱ'모양
	1,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,	1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,	1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,	0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,

	//'ㄴ' 모양
	1,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,	1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,	0,1,0,0,0,1,0,0,1,1,0,0,0,0,0,0,	1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,

	//'Z' 모양
	1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,	0,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,	1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,	0,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,

	//'S' 모양
	0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,	1,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,	0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,	1,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0

};

// 스테이지별 설정 초기화
// 순서대로 speed, stick_rate, clear_line
const STAGE stage_data[10] = {
	STAGE(40, 20, 20),		// Level 1
	STAGE(38, 18, 20),		// Level 2
	STAGE(35, 18, 20),		// Level 3
	STAGE(30, 17, 20),		// Level 4
	STAGE(25, 16, 20),		// Level 5
	STAGE(20, 14, 20),		// Level 6
	STAGE(15, 14, 20),		// Level 7
	STAGE(10, 13, 20),		// Level 8
	STAGE(6, 12, 20),		// Level 9
	STAGE(4, 11, 99999)		// Level 10
};

//*********************************
//함수 선언
//*********************************
int gotoxy(int x,int y);	//커서옮기기
void SetColor(COLOR color);	//색표현
int init();					//각종변수 초기화
int show_cur_block(int shape,const Rotation& rotation,const Position& pos);	//진행중인 블럭을 화면에 표시한다
int erase_cur_block(int shape,const Rotation& rotation,const Position& pos);	//블럭 진행의 잔상을 지우기 위한 함수
int show_total_block();	//쌓여져있는 블럭을 화면에 표시한다.
int show_next_block(int shape);
int make_new_block();	//return값으로 block의 모양번호를 알려줌
int strike_check(int shape,const Rotation& rotation,const Position& pos);	//블럭이 화면 맨 아래에 부닥쳤는지 검사 부닥치면 1을리턴 아니면 0리턴
int merge_block(int shape,const Rotation& rotation,const Position& pos);	//블럭이 바닥에 닿았을때 진행중인 블럭과 쌓아진 블럭을 합침
int block_start(int shape,Rotation& rotation,Position& pos);	//블럭이 처음 나올때 위치와 모양을 알려줌
int move_block(int* shape,Rotation& rotation,Position& pos,int* next_shape);	//게임오버는 1을리턴 바닥에 블럭이 닿으면 2를 리턴
int rotate_block(int shape,Rotation& rotation,Position& pos);
int show_gameover();
int show_gamestat(bool printed_text = false);
int show_logo();
int input_data();
int check_full_line();	


int main()
{
	int i;
	int is_gameover=0;
	char keytemp;
	init();
	show_logo();
	while(1)
	{
		
		input_data();
		show_total_block();
		block_shape = make_new_block();
		next_block_shape = make_new_block();
		show_next_block(next_block_shape);
		block_start(block_shape,block_rotation,block_pos);
		show_gamestat(true);
		for(i=1;1;i++)
		{
			if(_kbhit())
			{
				keytemp = _getche();
				if(keytemp == EXT_KEY)
				{
					keytemp = _getche();
					switch(keytemp)
					{
					case KEY_UP:		//회전하기
						{
							Rotation next_rotation(block_rotation.getNextAngle());
							if(strike_check(block_shape,next_rotation,block_pos) == 0)
							{
								erase_cur_block(block_shape,block_rotation,block_pos);
								block_rotation.rotateClockwise();
								show_cur_block(block_shape,block_rotation,block_pos);
							}
						}
						break;
					case KEY_LEFT:		//왼쪽으로 이동
						if(block_pos.getX()>1)
						{
							erase_cur_block(block_shape,block_rotation,block_pos);
							block_pos.moveLeft();
							if(strike_check(block_shape,block_rotation,block_pos) == 1)
								block_pos.moveRight();
							
							show_cur_block(block_shape,block_rotation,block_pos);
						}
						break;
					case KEY_RIGHT:		//오른쪽으로 이동
						
						if(block_pos.getX()<13)
						{
							erase_cur_block(block_shape,block_rotation,block_pos);
							block_pos.moveRight();
							if(strike_check(block_shape,block_rotation,block_pos) == 1)
								block_pos.moveLeft();
							show_cur_block(block_shape,block_rotation,block_pos);
						}
						break;
					case KEY_DOWN:		//아래로 이동
						is_gameover = move_block(&block_shape,block_rotation,block_pos,&next_block_shape);
						show_cur_block(block_shape,block_rotation,block_pos);
						break;
					}
				}
				if(keytemp == 32 )	//스페이스바를 눌렀을때
				{
					while(is_gameover == 0)
					{
						is_gameover = move_block(&block_shape,block_rotation,block_pos,&next_block_shape);
					}
					show_cur_block(block_shape,block_rotation,block_pos);
				}
			}
			if(i%stage_data[gamestate.getLevel()].getSpeed() == 0)
			{
				is_gameover = move_block(&block_shape,block_rotation,block_pos,&next_block_shape);
				
				show_cur_block(block_shape,block_rotation,block_pos);
			}
			
			if(stage_data[gamestate.getLevel()].getClearLine() <= gamestate.getLines())	//클리어 스테이지
			{
				gamestate.levelUp();
				show_total_block();
				show_gamestat();
				show_next_block(next_block_shape);
			}
			if(is_gameover == 1)
			{
				show_gameover();
				SetColor(COLOR::GRAY);
				is_gameover = 0;
				break;
			}
			
			gotoxy(77,23);
			Sleep(15);
			gotoxy(77,23);
		}
		init();
	}
	return 0;
}

int gotoxy(int x,int y)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	COORD pos; 
	pos.Y=y;
	pos.X=x;
	SetConsoleCursorPosition(hConsole, pos); 
	return 0;
}

void SetColor(COLOR color) 
{ 
	static HANDLE std_output_handle=GetStdHandle(STD_OUTPUT_HANDLE);
	// enum 클래스를 int로 형변환
	SetConsoleTextAttribute(std_output_handle, static_cast<int>(color));
} 

int init()
{
	int i,j;

	srand((unsigned)time(NULL));
	
	
	for(i=0;i<20;i++)
	{
		for(j=0;j<14;j++)
		{
			if((j==0) || (j==13))
			{
				total_block[i][j]=1;
			}else{
				total_block[i][j]=0;
			}
		}
	}

	for(j=0;j<14;j++)			//화면의 제일 밑의 줄은 1로 채운다.
		total_block[20][j]=1;
	
	//전역변수 초기화
	gamestate.resetState();
	board_offset.set(5, 1);

	return 0;
}

int show_cur_block(int shape,const Rotation& rotation,const Position& pos)
{
	int i,j;
	int x = pos.getX();
	int y = pos.getY();
	int angle = rotation.getAngle();
	
	switch(shape)
	{
	case 0:
		SetColor(COLOR::RED);
		break;
	case 1:
		SetColor(COLOR::BLUE);
		break;
	case 2:
		SetColor(COLOR::SKY_BLUE);
		break;
	case 3:
		SetColor(COLOR::WHITE);
		break;
	case 4:
		SetColor(COLOR::YELLOW);
		break;
	case 5:
		SetColor(COLOR::VOILET);
		break;
	case 6:
		SetColor(COLOR::GREEN);
		break;
	}

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if( (j+y) <0)
				continue;

			if(block[shape][angle][j][i] == 1)
			{
				gotoxy((i+x)*2+board_offset.getX(),j+y+board_offset.getY());
				printf("■");

			}
		}
	}
	SetColor(COLOR::BLACK);
	gotoxy(77,23);
	return 0;
}

int erase_cur_block(int shape,const Rotation& rotation,const Position& pos)
{
	int i,j;
	int x = pos.getX();
	int y = pos.getY();
	int angle = rotation.getAngle();
	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(block[shape][angle][j][i] == 1)
			{
				gotoxy((i+x)*2+board_offset.getX(),j+y+board_offset.getY());
				printf("  ");
				//break;
				
			}
		}
	}
	return 0;	
}



int show_total_block()
{
	int i,j;
	SetColor(COLOR::DARK_GRAY);
	for(i=0;i<21;i++)
	{
		for(j=0;j<14;j++)
		{
			if(j==0 || j==13 || i==20)		//레벨에 따라 외벽 색이 변함
			{
				SetColor(static_cast<COLOR>((gamestate.getLevel() % 6) + 1));
				
			}else{
				SetColor(COLOR::DARK_GRAY);
			}
			gotoxy( (j*2)+board_offset.getX(), i+board_offset.getY() );
			if(total_block[i][j] == 1)
			{	
				printf("■");
			}else{
				printf("  ");
			}
			
		}
	}
	SetColor(COLOR::BLACK);
	gotoxy(77,23);
	return 0;
}

int make_new_block()
{
	int shape;
	int i;
	i=rand()%100;
	if(i<=stage_data[gamestate.getLevel()].getStickRate())		//막대기 나올확률 계산
		return 0;							//막대기 모양으로 리턴

	shape = (rand()%6)+1;		//shape에는 1~6의 값이 들어감
	return shape;
}


int strike_check(int shape,const Rotation& rotation,const Position& pos)
{
	int i,j;
	int block_dat = 0;
	int x = pos.getX();
	int y = pos.getY();
	int angle = rotation.getAngle();

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(  ((x+j) == 0)  || ((x+j) == 13) )
				block_dat = 1;
			else if (y + i >= 0 && x + j >= 0)
				block_dat = total_block[y+i][x+j];
			
			
			if((block_dat == 1) && (block[shape][angle][i][j] == 1))																							//좌측벽의 좌표를 빼기위함
			{
				return 1;
			}
		}
	}
	return 0;
}

int merge_block(int shape,const Rotation& rotation,const Position& pos)
{
	int i,j;
	int x = pos.getX();
	int y = pos.getY();
	int angle = rotation.getAngle();
	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			total_block[y+i][x+j] |=  block[shape][angle][i][j];
		}
	}
	check_full_line();
	show_total_block();
	
	return 0;
}

int block_start(int shape,Rotation& rotation,Position& pos)
{
	
	pos.set(5, -4);
	rotation.reset();
	return 0;	
}

int show_gameover()
{
	SetColor(COLOR::RED);
	gotoxy(15,8);
	printf("┏━━━━━━━━━━━━━┓");
	gotoxy(15,9);
	printf("┃**************************┃");
	gotoxy(15,10);
	printf("┃*        GAME OVER       *┃");
	gotoxy(15,11);
	printf("┃**************************┃");
	gotoxy(15,12);
	printf("┗━━━━━━━━━━━━━┛");
	fflush(stdin);
	Sleep(1000);
	
	_getche();
	system("cls");

	return 0;
}

int move_block(int* shape,Rotation& rotation,Position& pos,int* next_shape)
{
	erase_cur_block(*shape,rotation,pos);
	
	pos.moveDown();	//블럭을 한칸 아래로 내림
	if(strike_check(*shape,rotation,pos) == 1)
	{
		if(pos.getY()<=0)	//게임오버
		{
			pos.moveUp();
			return 1;
		}
		pos.moveUp();
		merge_block(*shape,rotation,pos);
		*shape = *next_shape;
		*next_shape = make_new_block();
		
		block_start(*shape,rotation,pos);	//rotation,pos는 참조임
		show_next_block(*next_shape);
		return 2;
	}
	return 0;
}

int rotate_block(int shape,Rotation& rotation,Position& pos)
{
	return 0;
}

int check_full_line()
{
	int i,j,k;
	for(i=0;i<20;i++)
	{
		for(j=1;j<13;j++)
		{
			if(total_block[i][j] == 0)
				break;
		}
		if(j == 13)	//한줄이 다 채워졌음
		{
			gamestate.addLines(1);
			show_total_block(); 
			SetColor(COLOR::BLUE);
			gotoxy(1*2+board_offset.getX(),i+board_offset.getY());
			for(j=1;j<13;j++)
			{
				printf("□");
				Sleep(10);
			}
			gotoxy(1*2+board_offset.getX(),i+board_offset.getY());
			for(j=1;j<13;j++)
			{
				printf("  ");
				Sleep(10);
			}

			for(k=i;k>0;k--)
			{
				for(j=1;j<13;j++)
					total_block[k][j] = total_block[k-1][j];
			}
			for(j=1;j<13;j++)
				total_block[0][j] = 0;
			int plusScore = 100 + (gamestate.getLevel() * 10) + (rand() % 10);
			gamestate.addScore(plusScore);
			show_gamestat();
		}
	}
	return 0;
}

int show_next_block(int shape)
{
	int i,j;
	SetColor(static_cast<COLOR>((gamestate.getLevel() + 1) % 6 + 1));
	for(i=1;i<7;i++)
	{
		gotoxy(33,i);
		for(j=0;j<6;j++)
		{
			if(i==1 || i==6 || j==0 || j==5)
			{
				printf("■");				
			}else{
				printf("  ");
			}

		}
	}
	Position next_pos(15, 1);
	Rotation next_rotation(0);
	show_cur_block(shape,next_rotation,next_pos);
	return 0;
}

int show_gamestat(bool printed_text)
{
	SetColor(COLOR::GRAY);
	if(printed_text)
	{
		gotoxy(35,7);
		printf("STAGE");

		gotoxy(35,9);
		printf("SCORE");

		gotoxy(35,12);
		printf("LINES");
	}
	gotoxy(41,7);
	printf("%d",gamestate.getLevel() + 1);
	gotoxy(35,10);
	printf("%10d", gamestate.getScore());
	gotoxy(35,13);
	printf("%10d",stage_data[gamestate.getLevel()].getClearLine() - gamestate.getLines());
	return 0;
}

int input_data()
{
	int i=0;
	SetColor(COLOR::GRAY);
	gotoxy(10,7);
	printf("┏━━━━<GAME KEY>━━━━━┓");
	Sleep(10);
	gotoxy(10,8);
	printf("┃ UP   : Rotate Block        ┃");
	Sleep(10);
	gotoxy(10,9);
	printf("┃ DOWN : Move One-Step Down  ┃");
	Sleep(10);
	gotoxy(10,10);
	printf("┃ SPACE: Move Bottom Down    ┃");
	Sleep(10);
	gotoxy(10,11);
	printf("┃ LEFT : Move Left           ┃");
	Sleep(10);
	gotoxy(10,12);
	printf("┃ RIGHT: Move Right          ┃");
	Sleep(10);
	gotoxy(10,13);
	printf("┗━━━━━━━━━━━━━━┛");

	
	while(i<1 || i>8)
	{
		gotoxy(10,3);
		printf("Select Start level[1-8]:       \b\b\b\b\b\b\b");
		// scanf_s("%d",&i);
		cin >> i; // 수정
		if (cin.fail()) {
			cin.clear();
			cin.ignore(1024, '\n');
			continue;
		}
	}
	
	
	gamestate.setLevel(i - 1);
	system("cls");
	return 0;
}

int show_logo()
{
	int i,j;
	gotoxy(13,3);
	printf("┏━━━━━━━━━━━━━━━━━━━━━━━┓");
	Sleep(100);
	gotoxy(13,4);
	printf("┃◆◆◆  ◆◆◆  ◆◆◆   ◆◆     ◆   ◆◆◆ ┃");
	Sleep(100);
	gotoxy(13,5);
	printf("┃  ◆    ◆        ◆     ◆ ◆    ◆   ◆     ┃");
	Sleep(100);
	gotoxy(13,6);
	printf("┃  ◆    ◆◆◆    ◆     ◆◆     ◆     ◆   ┃");
	Sleep(100);
	gotoxy(13,7);
	printf("┃  ◆    ◆        ◆     ◆ ◆    ◆       ◆ ┃");
	Sleep(100);
	gotoxy(13,8);
	printf("┃  ◆    ◆◆◆    ◆     ◆  ◆   ◆   ◆◆◆ ┃");
	Sleep(100);
	gotoxy(13,9);
	printf("┗━━━━━━━━━━━━━━━━━━━━━━━┛");

	gotoxy(28,20);
	printf("Please Press Any Key~!");

	for(i=0;i>=0;i++){
		if(i%40==0)
		{
			
			
			for(j=0;j<5;j++)
			{
				gotoxy(17,14+j); // 수정
				printf("                                                          ");
				
				
			}
			Position logo_pos1(6, 14);
			Position logo_pos2(12, 14);
			Position logo_pos3(19, 14);
			Position logo_pos4(24, 14);
			Rotation logo_rot1(rand()%4);
			Rotation logo_rot2(rand()%4);
			Rotation logo_rot3(rand()%4);
			Rotation logo_rot4(rand()%4);
			show_cur_block(rand()%7,logo_rot1,logo_pos1);
			show_cur_block(rand()%7,logo_rot2,logo_pos2);
			show_cur_block(rand()%7,logo_rot3,logo_pos3);
			show_cur_block(rand()%7,logo_rot4,logo_pos4);
		}
		if(_kbhit())
			break;
		Sleep(30);
	}
	
	_getche();
	system("cls");

	return 0;
}