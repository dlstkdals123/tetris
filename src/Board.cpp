#include "Board.h"
#include <iostream>
#include <Windows.h>

using namespace std;

Board::Board() {
  int i,j;
  for(i=0;i<20;i++)
	{
		for(j=0;j<14;j++)
		{
			if((j==0) || (j==13))
			{
				total_block[i][j]=1; // Ï¢åÏö∞ Î≤?
			}else{
				total_block[i][j]=0; // Îπ? Í≥µÍ∞Ñ
			}
		}
	}

  for(j=0;j<14;j++) { //?ôîÎ©¥Ïùò ?†ú?ùº Î∞ëÏùò Ï§ÑÏ?? 1Î°? Ï±ÑÏö¥?ã§.
    total_block[20][j]=1; // Î∞îÎã•
  }
}
Board::~Board() {}

int Board::init() {
  int i,j;
  for(i=0;i<20;i++) {
		for(j=0;j<14;j++) {
			if((j==0) || (j==13)) {
				total_block[i][j]=1; // Ï¢åÏö∞ Î≤?
			} else {
				total_block[i][j]=0; // Îπ? Í≥µÍ∞Ñ
			}
		}
	}

  for(j=0;j<14;j++) { //?ôîÎ©¥Ïùò ?†ú?ùº Î∞ëÏùò Ï§ÑÏ?? 1Î°? Ï±ÑÏö¥?ã§.
    total_block[20][j]=1; // Î∞îÎã•
  }

  return 0;
}

void Board::draw(const int &level) const {
  int i,j;
  Utils::setColor(COLOR::DARK_GRAY);

  for(i=0;i<21;i++) {
		for(j=0;j<14;j++) {
			if(j==0 || j==13 || i==20) {		//?†àÎ≤®Ïóê ?î∞?ùº ?ô∏Î≤? ?Éâ?ù¥ Î≥??ï® 
				Utils::setColor((level %6) +1);
			} else {
				Utils::setColor(COLOR::DARK_GRAY);
			}
			Utils::gotoxy( (j * 2) + Utils::ab_x, i + Utils::ab_y );
			if(total_block[i][j] == 1) {
        cout << "?ñ†";
      } else {
        cout << "  ";
      }
    }
    
	}

  cout.flush();

  Utils::setColor(COLOR::BLACK);
	Utils::gotoxy(77,23);
} 

int Board::isStrike(const Block &block) {
  int i,j;
  int x = block.getx();
  int y = block.gety();

  for(i=0;i<4;i++) {
		for(j=0;j<4;j++) {
      if (block.getShape(i, j) == 0) { // Î∏îÎü≠?ùò ?ï¥?ãπ ?úÑÏπòÍ?? Ï±ÑÏõå?†∏?ûà?äîÏß? Í≤??Ç¨.
        continue;
      }

      if (((x+j) == 0) || ((x+j) == 13)) { // Î≤? Ï∂©Îèå?ãú
        return 1;
      }
      
      if (y + i >= 0) {
        if (total_block[y + i][x + j] == 1) {  // Î∞îÎã• or ?ã§Î•? Î∏îÎ°ù?óê ?ãø?ïò?äîÏß? Í≤??Ç¨
          return 1;
        }
      }
		}
	}
	return 0;
}

void Board::mergeBlock(const Block &block) {
  int i,j;
  int x = block.getx();
  int y = block.gety();
  for (i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      if (block.getShape(i, j) == 1) {
        if (y + i >= 0) {
          total_block[y + i][x + j] = 1;
        }
      }
		}
  }
  // check_full_line();
	// show_total_block();
	
}
// check_full_line
int Board::deleteFullLine() {
  int i,j,k;
  int deletedLines = 0;

  for(i = 0; i < 20; i++) {
		for(j = 1; j < 13; j++) {
			if(total_block[i][j] == 0) {
        break;
      }
		}
		if(j == 13) {	// ?ïúÏ§ÑÏù¥ ?ã§ Ï±ÑÏõåÏ°åÏùÑ ?ñÑ
			deletedLines++;
			// show_total_block(); 

			Utils::setColor(COLOR::BLUE);
			Utils::gotoxy(1 * 2 + Utils::ab_x, i + Utils::ab_y);

			for(j = 1; j < 13; j++) {
        cout << "?ñ°";
        cout.flush();
        Sleep(10);
      }
			Utils::gotoxy(1 * 2 + Utils::ab_x, i + Utils::ab_y);
			for(j = 1; j < 13; j++) {
				cout << "  ";
        cout.flush();
				Sleep(10);
			}

			for(k = i; k > 0; k--) {
				for(j = 1; j < 13; j++) {
          total_block[k][j] = total_block[k-1][j];
        }
			}
			for (j = 1; j < 13; j++) {
        total_block[0][j] = 0;
      }
		}
	}
  return deletedLines;
}

void Borad::show_next_block() {
  int i,j;
	Utils::setColor(COLOR::DARK_GRAY);
	for(i=0;i<21;i++)
	{
		for(j=0;j<14;j++)
		{
			if(j==0 || j==13 || i==20)		//∑π∫ßø° µ˚∂Û ø‹∫Æ ªˆ¿Ã ∫Ø«‘
			{
				Utils::setColor(static_cast<COLOR>((gs.getLevel() % 6) + 1));
				
			}else{
				Utils::setColor(COLOR::DARK_GRAY);
			}
			Utils::gotoxy( (j*2)+boardOffset.getX(), i+boardOffset.getY() );
            int total_block = board.data
			if(total_block[i][j] == 1)
			{	
				printf("°·");
			}else{
				printf("  ");
			}
			
		}
	}
	Utils::setColor(COLOR::BLACK);
	Utils::gotoxy(77,23);
}