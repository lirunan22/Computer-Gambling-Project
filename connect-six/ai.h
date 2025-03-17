#pragma once
#include<map>

#define GRID_SIZE 19
struct transtable {
	int flag;//判断子树状态是否搜完
	int length;//判断是第几层的之前状态
	int score;
};
std::pair<int, int> main_init(int, int [GRID_SIZE][GRID_SIZE],int );
