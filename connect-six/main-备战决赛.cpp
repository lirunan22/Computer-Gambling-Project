#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <stdio.h>
#include <graphics.h>
#include <conio.h>
#include <direct.h>
#include <string.h>
#include<iostream>
#include<chrono>
#include<cstring>
#include<queue>
#include <iomanip>
#include <sstream>
#include <fstream>
#include"ai.h"
#include<vector>
#include <unordered_map>
#include <string>
using namespace std;
/*
	GeekPan 2022.1.1
	EasyX图形库
*/
//int sp; // tss spend time
//
//int isfind = 0;//ab是否出结果
//
//
//unsigned long long hash_table[GRID_SIZE][GRID_SIZE][3];
//int nowdepth;//迭代搜索的深度
//int tss_flag = 0;
//int abtss_flag = 0;
//int turnID;
//
//
//std::map<unsigned long long, transtable> mp;
std::chrono::duration<double> elapsed;//用于计时
int cost = 0;
int last_time = 0;

#define offsetx 20 //绘图时x的偏移值
#define offsety 20 //绘图时x的偏移值
#define map_width 19//棋盘宽
#define map_height 19//棋盘高
#define piece_size 30	//棋子直径
const int image_size = (map_height - 1) * piece_size + offsety * 2;

IMAGE backImg;	 //保存图片的数据类型
MOUSEMSG msg = { 0 };
int board_map[map_height][map_width];//棋盘，0表示无子，1表示白子，2表示黑子
int flag[2][4];//第一维标记- | / \ 四个方向能连几个棋子，第二维标记是否活子，2表示活2子，1表示活1子，0表示死子
int currentPiece; //当前棋子颜色
int changePiece;//用来改变棋子
int AIx, AIy;//用来记录人机落子位置

void initView(); //初始化界面
void initGame(); //游戏初始化
void playerVSAI(); //人机对战
void playerVSplayer(); //人人对战
bool pieceSet(int y, int x, int color); //放置棋子，返回true表示放置成功，false 表示放置失败
void judge(int y, int x, int color); //判断当前位置4个方向连接的棋子数量，参数为棋子颜色 1白2黑
void drawPiece(int x, int y, int color); //绘制棋子
void AI(); //AI判断落子的位置
void InitPage(); //初始化页面
void ModelShow(); //模式显示
int cur_player = -1;//默认player是黑子


void storeMove(const int board_map[map_height][map_width], const std::pair<int, int>& move);
std::pair<int, int> getMove(const int board_map[map_height][map_width]);
std::string encodeBoard(const int board_map[map_height][map_width]);
std::unordered_map<std::string, std::pair<int, int>> memo;
void inithash();
int example[100][map_height][map_width] = {
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
},
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,2,0,2,0,1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1,2,0,2,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1,1,2,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,2,2,1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,2,2,2,2,1,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,2,1,1,1,0,2,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		},
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,1,0,2,0,2,1,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
		}


};



bool legal(int x, int y)
{
	if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return false;
	return true;
}

int scan(int x, int y)
{
	int dir[4][2] = {
	{0, 1}, {1, 0}, {1, 1}, {1, -1} };
	for (int k = 0; k < 4; k++)
	{
		int posx = x + dir[k][0], posy = y + dir[k][1];
		for (int i = 0; i < 5; i++)
		{
			if (!legal(posx, posy)) break;
			if (board_map[posx][posy] == 0) break;
			if (board_map[posx][posy] != board_map[posx - dir[k][0]][posy - dir[k][1]]) break;
			if (i == 4)
			{
				//std::cout << board_map[posx][posy] << std::endl;
				return board_map[posx][posy];
			}
			posx += dir[k][0], posy += dir[k][1];
		}
	}
	return 0;
}
int jud()
{
	for (int i = 0; i < GRID_SIZE; i++)
	{
		for (int j = 0; j < GRID_SIZE; j++)
		{
			if (int temp = scan(i, j)) return temp;
		}
	}
	return 0;
}
string team_fir, team_sec, city, nowt;
deque<pair<int, pair<int, int>>> sq;//1是白色 2是黑色
void in_que(int color, int x, int y)
{
	sq.push_back({ color,{x,y} });
}
void ou_que()//用于悔棋
{
	sq.pop_back();
}
void print_qipu(int win)//2为先手胜 1为后手胜
{
	string s = "{[C6][";
	s += team_fir;
	s += " B][";
	s += team_sec;
	s += " W][";
	if (win == 2) s += "先手胜][";
	else s += "后手胜][";
	s += nowt;
	s += city;
	s += "][2024 CCGC];";
	int ff = 0;//是否为第一手
	while (!sq.empty())
	{
		auto temp = sq.front();
		sq.pop_front();
		if (temp.first == 2)
		{
			s += "B(";
			s += (char)(temp.second.first + 'A');
			s += ",";
			if (temp.second.second <= 8)
				s += (char)(temp.second.second + '1');
			else
			{
				s += '1';
				temp.second.second -= 9;
				s += (char)(temp.second.second + '0');
			}
			/*if (!ff)
			{
				s += ")MARK[1];";
				ff = 1;
			}*/
			//else 
			s += ");";
		}
		else if (temp.first == 1)
		{
			s += "W(";
			s += (char)(temp.second.first + 'A');
			s += ",";
			if (temp.second.second <= 8)
				s += (char)(temp.second.second + '1');
			else
			{
				s += '1';
				temp.second.second -= 9;
				s += (char)(temp.second.second + '0');
			}
			s += ");";
		}
	}
	s.pop_back();
	s += "}";

	std::string fileName = "C6-"; // 将文件名存储在一个变量中
	fileName += team_fir;
	fileName += " B vs ";
	fileName += team_sec;
	fileName += " W";
	if (win == 2)
	{
		fileName += "-先手胜-";
		fileName += nowt.substr(0, 10);
		fileName += city;
		fileName += "-2024 CCGC";
	}
	else
	{
		fileName += "-后手胜-";
		fileName += nowt.substr(0, 10);
		fileName += city;
		fileName += "-2024 CCGC";
	}
	fileName += ".txt";
	// 创建一个输出文件流对象，并使用变量中的文件名打开文件
	std::ofstream outFile(fileName);
	outFile.close();
	outFile.open(fileName);


	// 写入文本内容到文件
	outFile << s << std::endl;

	// 关闭文件流
	outFile.close();

	std::cout << "文件 " << fileName << " 写入完成。" << std::endl;
	printf("%s", s.c_str());
	//printf("{[C6][先手参赛队 %s][后手参赛队 %s]");
}
std::string getCurrentDateTime() {
	// 获取当前时间点
	auto now = std::chrono::system_clock::now();

	// 将时间点转换为时间_t 类型
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);

	// 将时间_t 类型转换为本地时间
	std::tm* now_tm = std::localtime(&now_c);

	// 使用 stringstream 格式化日期和时间
	std::ostringstream oss;
	oss << std::put_time(now_tm, "%Y.%m.%d %H:%M "); // 格式化为 yyyy-mm-dd hh:mm:ss

	return oss.str();
}
void main(void)
{
	inithash();
	char control[3];
	printf("------------请先输入比赛信息-------------\n");
	nowt = getCurrentDateTime();
	printf("请输入先手队伍名\n");
	cin >> team_fir;

	printf("请输入后手队伍名\n");
	cin >> team_sec;
	printf("请输入城市名(或线上)\n");
	cin >> city;

	InitPage(); //初始化页面
	while (1)
	{
		ModelShow(); //模式显示
		switch (msg.uMsg)
		{
		case WM_LBUTTONDOWN: //鼠标左键按下
			if (msg.x >= 255 && msg.x < 520 && msg.y >= 180 && msg.y <= 230) //在人机对战区域按下
			{
				playerVSAI(); //人机对战 玩家执黑
				InitPage(); //重新显示页面
				break;
			}
			if (msg.x >= 255 && msg.x < 520 && msg.y >= 250 && msg.y <= 300) //在人人对战区域按下
			{
				cur_player = 1;
				playerVSAI(); //人机对战 玩家执白 
				InitPage(); //重新显示页面
				break;
			}
			if (msg.x >= 255 && msg.x < 520 && msg.y >= 320 && msg.y <= 370) //在人人对战区域按下
			{
				cur_player = 1;
				playerVSAI(); //人机对战 玩家执白 
				InitPage(); //重新显示页面
				break;
			}
		}
	}
	////快速获得棋盘数组
	//for (int i = 0; i < map_height; i++) {
	//	std::cout << "{";
	//	for (int j = 0; j < map_width - 1; j++) {
	//		std::cout << board_map[i][j] << ",";

	//	}
	//	if (i < map_height - 1)
	//		std::cout << board_map[i][map_width - 1] << "}," << std::endl;
	//	else std::cout << board_map[i][map_width - 1] << "}" << std::endl;
	//}
	//std::cout << std::endl;


}

void ModelShow() //模式显示
{
	msg = GetMouseMsg();  //获取鼠标的消息

	//鼠标的位置 在Begin !的位置
	if (msg.x >= 255 && msg.x < 520 && msg.y >= 180 && msg.y <= 230)
	{
		settextcolor(RGB(255, 0, 0)); //设置字体的颜色
		outtextxy(250, 180, L"player黑!"); //255, 180 用于在指定位置输出字符串。
	}
	else if (msg.x >= 255 && msg.x < 520 && msg.y >= 250 && msg.y <= 300)
	{
		settextcolor(RGB(255, 0, 0));	//设置字体的颜色
		outtextxy(250, 250, L"player白!");
	}
	else if (msg.x >= 255 && msg.x < 520 && msg.y >= 320 && msg.y <= 370)
	{
		settextcolor(RGB(255, 0, 0));	//设置字体的颜色
		outtextxy(250, 320, L"ai vs ai!");
	}
	else
	{
		settextcolor(RGB(137, 57, 196)); //设置字体的颜色
		outtextxy(250, 180, L"player黑!");   //输出文字
		outtextxy(250, 250, L"player白!");
		outtextxy(250, 320, L"ai vs ai!");
	}
}

void InitPage() //初始化页面
{
	initgraph(540, 540, EW_SHOWCONSOLE); //520界面的宽 520界面的高 520*520个像素点
	loadimage(&backImg, L"3.jpg", 540, 540);
	//backImg 输出图片  //图形库  win32 MFC duilib opencv openGL qt
	putimage(0, 0, &backImg);

	//msg.  //mkCtrl 在发出指令的时候 是否按下Ctrl
	setbkmode(TRANSPARENT);  //设置背景的风格 tranparent（透明的）
	settextstyle(55, 0, L"楷体");    //设置字体的风格
}

void AI()
{

}
void drawPiece(int x, int y, int color) //绘制棋子
{

	if (color == 1)
	{
		setfillcolor(WHITE); //设置当前的填充颜色。 WHITE  0xFFFFFF 白色
	}
	else if (color == 2)
	{
		setfillcolor(BLACK); //黑色
	}
	solidcircle(x * piece_size + offsetx, image_size - y * piece_size - offsety, piece_size / 2); //用于画填充圆（无边框）。
}
void undoDraw(int x, int y, int color) // 撤销绘制
{
	// 擦除棋子（这里假设背景色为 LIGHTGRAY）
	setfillcolor(LIGHTGRAY);
	solidcircle(x * piece_size + offsetx, y * piece_size + offsety, piece_size / 2);

	// 重新绘制网格线
	setlinecolor(BLACK);
	int centerX = x * piece_size + offsetx;
	int centerY = y * piece_size + offsety;

	// 计算网格线的边界
	int left = centerX - piece_size / 2;
	int right = centerX + piece_size / 2;
	int top = centerY - piece_size / 2;
	int bottom = centerY + piece_size / 2;

	// 重新绘制网格线
	line(left, centerY, right, centerY); // 水平线
	line(centerX, top, centerX, bottom); // 垂直线
}


void initGame() //游戏初始化
{
	currentPiece = 2;//初始棋子为黑
	changePiece = -1;
	//把棋盘置空
	for (int i = 0; i < map_height; i++)
	{
		for (int j = 0; j < map_width; j++)
		{
			board_map[i][j] = 0;
		}
	}
}

void initView() //初始化界面
{
	initgraph((map_width - 1) * piece_size + offsetx * 2, (map_height - 1) * piece_size + offsety * 2, EW_SHOWCONSOLE);
	//加载背景
	loadimage(NULL, _T("2.jpg"), map_width * piece_size + offsetx * 2, map_height * piece_size + offsety * 2); // 读取棋盘图片至绘图窗口 

	//画线
	setlinecolor(BLACK);

	//设置字体颜色和背景色
	settextcolor(WHITE);  // 设置字体颜色为白色
	setbkcolor(BLACK);    // 设置字体背景色为黑色
	//setbkmode(OPAQUE);    // 设置字体背景模式为不透明
	setbkmode(TRANSPARENT);
	TCHAR buffer[10]; // 用于存放数字字符

	// 画竖线和列标号
	for (int i = 0; i < map_width; i++) // 竖线
	{
		int x = i * piece_size + offsetx;
		line(x, offsety, x, offsety + (map_height - 1) * piece_size);

		// 绘制列标号
		_stprintf(buffer, _T("%c"), 'A' + i);
		//outtextxy(x, offsety - piece_size / 2, buffer);
		outtextxy(x, image_size - piece_size / 2, buffer);
	}

	// 画横线和行标号
	for (int i = 0; i < map_height; i++) // 横线
	{
		int y = offsety + i * piece_size;
		line(offsetx, y, (map_width - 1) * piece_size + offsetx, y);

		// 绘制行标号
		_stprintf(buffer, _T("%d"), 19 - i);
		outtextxy(offsetx - piece_size / 2, y, buffer);
	}
}
void output(int time)
{
	int h = time / 3600;
	time %= 3600;
	int m = time / 60;
	time %= 60;
	std::cout << "H： " << h << " M： " << m << " S： " << time << std::endl;
}
int lwx1, lwy1, lbx1, lby1, lwx2, lwy2, lbx2, lby2;//分别为上一步的黑棋和白棋落子
void unfallen(int y, int x)
{
	board_map[x][y] = 0;
}
void playerVSAI() //人机对战
{
	initGame(); //游戏初始化
	initView(); //页面初始化
	GetMouseMsg();
	MOUSEMSG m;
	int x, y;
	int fflag = 0;//判断是否是第一步悔棋
	int flag = 0;//0为黑棋方下的第一个棋子 
	while (1)
	{
		m = GetMouseMsg(); //获取一个鼠标消息
		if (m.mkRButton)
		{

			cout << "悔棋" << endl;
			closegraph();
			initView();
			cost -= last_time;//时间返回
			unfallen(lwx1, lwy1), ou_que(); unfallen(lwx2, lwy2), ou_que();
			unfallen(lbx1, lby1), ou_que();
			if (fflag)
			{
				unfallen(lbx2, lby2), ou_que();
			}

			for (int i = 0; i < GRID_SIZE; i++)
			{
				for (int j = 0; j < GRID_SIZE; j++)
				{
					if (board_map[i][j])
						drawPiece(j, i, board_map[i][j]);
				}
			}
			if (!fflag) flag = 0;//第一步就悔棋



		}


		int tag = 0;//判断player是否落子
		if (cur_player == 1 && !flag)//说明player执白色，ai执黑色
		{
			std::cout << "J 10" << std::endl;
			board_map[9][9] = 2;
			flag = 1;
			drawPiece(9, 9, currentPiece);
			in_que(currentPiece, 9, 9);
			//改变棋子颜色
			currentPiece += changePiece;
			changePiece *= -1;
		}
		//左键点击放置棋子
		if (m.mkLButton) //左键按下
		{
			x = (m.x - offsetx + piece_size / 2) / piece_size;
			y = ((image_size - m.y) - offsety + piece_size / 2) / piece_size;
			if (pieceSet(x, y, currentPiece))
			{
				//x y 
				if (flag) fflag = 1;

				//std::cout << image_size-m.x<<" "<<image_size-m.y<<" "<<x<<" "<<y;
				if (currentPiece == 1)
					lwx1 = x, lwy1 = y;
				else if (currentPiece == 2)
					lbx1 = x, lby1 = y;
				in_que(currentPiece, x, y);
				drawPiece(x, y, currentPiece);
				while (1)
				{
					if (!flag)
					{
						tag = 1; break;
					}

					m = GetMouseMsg();
					if (m.mkLButton) //左键按下
					{
						x = (m.x - offsetx + piece_size / 2) / piece_size;
						y = ((image_size - m.y) - offsety + piece_size / 2) / piece_size;
						if (pieceSet(x, y, currentPiece))
						{
							if (currentPiece == 1)
								lwx2 = x, lwy2 = y;
							else if (currentPiece == 2)
								lbx2 = x, lby2 = y;
							tag = 1;
							in_que(currentPiece, x, y);
							drawPiece(x, y, currentPiece);
							break;
						}

					}
				}
				flag = 1;
			}


			//判断输赢
			if (jud() == 1)
			{
				if (MessageBox(GetHWnd(), L"白子胜出！", L"游戏结束：", MB_OKCANCEL) == IDOK)
				{
					print_qipu(1);
					break;
				}

			}
			else if (jud() == 2)
			{
				if (MessageBox(GetHWnd(), L"黑子胜出！", L"游戏结束：", MB_OKCANCEL) == IDOK)
				{
					print_qipu(2);
					break;
				}
			}
			if (!tag) continue;
			//std::cout << 1 << std::endl;
			//改变棋子颜色
			currentPiece += changePiece;
			changePiece *= -1;
			//AI行动

			//////快速获得棋盘数组
			for (int i = 0; i < map_height; i++) {
				cout << "{";
				for (int j = 0; j < map_width; j++) {
					if (j != map_width - 1)
						std::cout << board_map[i][j] << ",";
					else std::cout << board_map[i][j];

				}
				cout << "}," << endl;

			}
			cout << endl;

			std::pair<int, int> temp = getMove(board_map);
			int remain_time = 900 - cost;//剩余时间
			auto start = std::chrono::high_resolution_clock::now();

			if (temp.first != -1 && temp.second != -1) {
				//成功打表
			}
			else {
				//没有打表
				if (cur_player == -1)
				{
					temp = main_init(1, board_map, remain_time);
				}
				else temp = main_init(-1, board_map, remain_time);
			}



			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = end - start;
			std::cout << "当步用时："; output(elapsed.count());
			last_time = elapsed.count();
			cost += elapsed.count();
			std::cout << "总用时："; output(cost);

			AIx = temp.first / GRID_SIZE, AIy = temp.first % GRID_SIZE;

			//std::cout << AIx << " " << AIy ;
			if (pieceSet(AIx, AIy, currentPiece))
			{
				if (currentPiece == 1)
					lwx1 = AIx, lwy1 = AIy;
				else if (currentPiece == 2)
					lbx1 = AIx, lby1 = AIy;
				in_que(currentPiece, AIx, AIy);
				drawPiece(AIx, AIy, currentPiece);

				AIx = temp.second / GRID_SIZE, AIy = temp.second % GRID_SIZE;

				if (pieceSet(AIx, AIy, currentPiece))
				{
					if (currentPiece == 1)
						lwx2 = AIx, lwy2 = AIy;
					else if (currentPiece == 2)
						lbx2 = AIx, lby2 = AIy;
					in_que(currentPiece, AIx, AIy);
					drawPiece(AIx, AIy, currentPiece);
				}

			}


			//判断输赢
			if (jud() == 1)
			{
				if (MessageBox(GetHWnd(), L"白子胜出！", L"游戏结束：", MB_OKCANCEL) == IDOK)
				{
					print_qipu(1);
					break;
				}
			}
			else if (jud() == 2)
			{
				if (MessageBox(GetHWnd(), L"黑子胜出！", L"游戏结束：", MB_OKCANCEL) == IDOK)
				{
					print_qipu(2);
					break;
				}
			}
			//std::cout << 2 << std::endl;
			//改变棋子颜色
			currentPiece += changePiece;
			changePiece *= -1;

		}
	}
	closegraph();
}



void judge(int y, int x, int color) //判断当前位置4个方向连接的棋子数量，参数为棋子颜色 1白2黑
{
	bool flag1 = false, flag2 = false; //标记方向两头是不是有相反颜色的棋子
	int count = 0;//连接棋子数量
	int addx = 0, addy = -1;//增量
	int posx = x, posy = y;//增加后的x,y
	// - 向
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posy < 0)//如果越界
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag1 = true;
			break;
		}
		count++;
	}
	addy = 1;
	posx = x;
	posy = y;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posy >= map_width)//如果越界
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag2 = true;
			break;
		}
		count++;
	}
	flag[1][0] = 2;
	if (flag1)
	{
		flag[1][0]--;
	}
	if (flag2)
	{
		flag[1][0]--;
	}

	flag[0][0] = count + 1;
	count = 0;
	// | 向
	flag1 = false;
	flag2 = false;
	posx = x;
	posy = y;
	addx = -1;
	addy = 0;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posx < 0) //如果越界
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag1 = true;
			break;
		}
		count++;
	}
	addx = 1;
	posx = x;
	posy = y;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posx >= map_height) //如果越界
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag2 = true;
			break;
		}
		count++;
	}
	flag[1][1] = 2;
	if (flag1)
	{
		flag[1][1]--;
	}
	if (flag2)
	{
		flag[1][1]--;
	}
	flag[0][1] = count + 1;
	count = 0;
	// / 向
	flag1 = false;
	flag2 = false;
	posx = x;
	posy = y;
	addx = -1;
	addy = 1;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posx < 0 || posy >= map_width) //如果越界
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag1 = true;
			break;
		}
		count++;
	}
	addx = 1;
	addy = -1;
	posx = x;
	posy = y;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posx >= map_height || posy < 0) //如果越界
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag2 = true;
			break;
		}
		count++;
	}
	flag[1][2] = 2;
	if (flag1)
	{
		flag[1][2]--;
	}
	if (flag2)
	{
		flag[1][2]--;
	}
	flag[0][2] = count + 1;
	count = 0;
	// \ 向
	flag1 = false;
	flag2 = false;
	posx = x;
	posy = y;
	addx = -1;
	addy = -1;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posy < 0 || posx < 0) //如果越界
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag1 = true;
			break;
		}
		count++;
	}
	addx = 1;
	addy = 1;
	posx = x;
	posy = y;
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posx >= map_height || posy >= map_width) //如果越界
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //如果没有棋子
		{
			break;
		}
		if (board_map[posx][posy] != color) //如果为对方棋子
		{
			flag2 = true;
			break;
		}
		count++;
	}
	flag[1][3] = 2;
	if (flag1)
	{
		flag[1][3]--;
	}
	if (flag2)
	{
		flag[1][3]--;
	}
	flag[0][3] = count + 1;
	count = 0;
}

bool pieceSet(int y, int x, int color) //放置棋子，返回true表示放置成功，false 表示放置失败
{
	if (board_map[x][y] != 0)//当前位置有棋子
	{
		return false;
	}
	board_map[x][y] = color;
	return true;
}

void playerVSplayer() //人人对战
{
	initGame();
	initView();
	MOUSEMSG m;
	int x, y;
	while (1) {//game looping
		m = GetMouseMsg();
		//左键点击放置棋子
		if (m.mkLButton) {
			x = (m.x - offsetx + piece_size / 2) / piece_size;
			y = (m.y - offsety + piece_size / 2) / piece_size;
			if (pieceSet(x, y, currentPiece)) {
				drawPiece(x, y, currentPiece);
				judge(x, y, currentPiece);
				if (flag[0][0] >= 5 || flag[0][1] >= 5 || flag[0][2] >= 5 || flag[0][3] >= 5) {//分出胜负则退出
					if (currentPiece == 2)
						MessageBox(GetHWnd(), L"黑子胜出！", L"游戏结束：", MB_OKCANCEL);
					else
						MessageBox(GetHWnd(), L"白子胜出！", L"游戏结束：", MB_OKCANCEL);
					break;
				}
				//改变棋子颜色
				currentPiece += changePiece;
				changePiece *= -1;
			}
		}
	}
	closegraph();
}

void inithash() {
	//初始化对应打表
	storeMove(example[0], { (GRID_SIZE - 13) * GRID_SIZE + 10, (GRID_SIZE - 11) * GRID_SIZE + 12 });
	storeMove(example[1], { (GRID_SIZE - 10) * GRID_SIZE + 8, (GRID_SIZE - 9) * GRID_SIZE + 8 });
	storeMove(example[2], { (GRID_SIZE - 10) * GRID_SIZE + 6, (GRID_SIZE - 10) * GRID_SIZE + 10 });
	storeMove(example[3], { (GRID_SIZE - 11) * GRID_SIZE + 11, (GRID_SIZE - 10) * GRID_SIZE + 12 });
	storeMove(example[4], { (GRID_SIZE - 7) * GRID_SIZE + 10, (GRID_SIZE - 9) * GRID_SIZE + 12 });
	storeMove(example[5], { (GRID_SIZE - 9) * GRID_SIZE + 6, (GRID_SIZE - 7) * GRID_SIZE + 8 });
	storeMove(example[6], { (GRID_SIZE - 11) * GRID_SIZE + 6, (GRID_SIZE - 13) * GRID_SIZE + 8 });
	storeMove(example[7], { (GRID_SIZE - 13) * GRID_SIZE + 9, (GRID_SIZE - 12) * GRID_SIZE + 10 });
	storeMove(example[8], { (GRID_SIZE - 9) * GRID_SIZE + 10, (GRID_SIZE - 13) * GRID_SIZE + 9 });
	storeMove(example[9], { (GRID_SIZE - 6) * GRID_SIZE + 9, (GRID_SIZE - 9) * GRID_SIZE + 5 });
	storeMove(example[10], { (GRID_SIZE - 6) * GRID_SIZE + 8, (GRID_SIZE - 7) * GRID_SIZE + 12 });
}
void storeMove(const int board_map[map_height][map_width], const std::pair<int, int>& move) {
	std::string encodedBoard = encodeBoard(board_map);
	memo[encodedBoard] = move;
}

std::pair<int, int> getMove(const int board_map[map_height][map_width]) {
	std::string encodedBoard = encodeBoard(board_map);
	auto it = memo.find(encodedBoard);
	if (it != memo.end()) {
		return it->second;
	}
	else {
		return { -1, -1 }; // 返回一个无效的坐标
	}
}

std::string encodeBoard(const int board_map[map_height][map_width]) {
	std::string encodedBoard;
	for (int i = 0; i < map_height; ++i) {
		for (int j = 0; j < map_width; ++j) {
			encodedBoard += std::to_string(board_map[i][j]);
		}
	}
	return encodedBoard;
}
