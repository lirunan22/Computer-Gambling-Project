#pragma once
#include<map>

#define GRID_SIZE 19
struct transtable {
	int flag;//�ж�����״̬�Ƿ�����
	int length;//�ж��ǵڼ����֮ǰ״̬
	int score;
};
std::pair<int, int> main_init(int, int [GRID_SIZE][GRID_SIZE],int );
