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
	EasyXͼ�ο�
*/
//int sp; // tss spend time
//
//int isfind = 0;//ab�Ƿ�����
//
//
//unsigned long long hash_table[GRID_SIZE][GRID_SIZE][3];
//int nowdepth;//�������������
//int tss_flag = 0;
//int abtss_flag = 0;
//int turnID;
//
//
//std::map<unsigned long long, transtable> mp;
std::chrono::duration<double> elapsed;//���ڼ�ʱ
int cost = 0;
int last_time = 0;

#define offsetx 20 //��ͼʱx��ƫ��ֵ
#define offsety 20 //��ͼʱx��ƫ��ֵ
#define map_width 19//���̿�
#define map_height 19//���̸�
#define piece_size 30	//����ֱ��
const int image_size = (map_height - 1) * piece_size + offsety * 2;

IMAGE backImg;	 //����ͼƬ����������
MOUSEMSG msg = { 0 };
int board_map[map_height][map_width];//���̣�0��ʾ���ӣ�1��ʾ���ӣ�2��ʾ����
int flag[2][4];//��һά���- | / \ �ĸ����������������ӣ��ڶ�ά����Ƿ���ӣ�2��ʾ��2�ӣ�1��ʾ��1�ӣ�0��ʾ����
int currentPiece; //��ǰ������ɫ
int changePiece;//�����ı�����
int AIx, AIy;//������¼�˻�����λ��

void initView(); //��ʼ������
void initGame(); //��Ϸ��ʼ��
void playerVSAI(); //�˻���ս
void playerVSplayer(); //���˶�ս
bool pieceSet(int y, int x, int color); //�������ӣ�����true��ʾ���óɹ���false ��ʾ����ʧ��
void judge(int y, int x, int color); //�жϵ�ǰλ��4���������ӵ���������������Ϊ������ɫ 1��2��
void drawPiece(int x, int y, int color); //��������
void AI(); //AI�ж����ӵ�λ��
void InitPage(); //��ʼ��ҳ��
void ModelShow(); //ģʽ��ʾ
int cur_player = -1;//Ĭ��player�Ǻ���


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
deque<pair<int, pair<int, int>>> sq;//1�ǰ�ɫ 2�Ǻ�ɫ
void in_que(int color, int x, int y)
{
	sq.push_back({ color,{x,y} });
}
void ou_que()//���ڻ���
{
	sq.pop_back();
}
void print_qipu(int win)//2Ϊ����ʤ 1Ϊ����ʤ
{
	string s = "{[C6][";
	s += team_fir;
	s += " B][";
	s += team_sec;
	s += " W][";
	if (win == 2) s += "����ʤ][";
	else s += "����ʤ][";
	s += nowt;
	s += city;
	s += "][2024 CCGC];";
	int ff = 0;//�Ƿ�Ϊ��һ��
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

	std::string fileName = "C6-"; // ���ļ����洢��һ��������
	fileName += team_fir;
	fileName += " B vs ";
	fileName += team_sec;
	fileName += " W";
	if (win == 2)
	{
		fileName += "-����ʤ-";
		fileName += nowt.substr(0, 10);
		fileName += city;
		fileName += "-2024 CCGC";
	}
	else
	{
		fileName += "-����ʤ-";
		fileName += nowt.substr(0, 10);
		fileName += city;
		fileName += "-2024 CCGC";
	}
	fileName += ".txt";
	// ����һ������ļ������󣬲�ʹ�ñ����е��ļ������ļ�
	std::ofstream outFile(fileName);
	outFile.close();
	outFile.open(fileName);


	// д���ı����ݵ��ļ�
	outFile << s << std::endl;

	// �ر��ļ���
	outFile.close();

	std::cout << "�ļ� " << fileName << " д����ɡ�" << std::endl;
	printf("%s", s.c_str());
	//printf("{[C6][���ֲ����� %s][���ֲ����� %s]");
}
std::string getCurrentDateTime() {
	// ��ȡ��ǰʱ���
	auto now = std::chrono::system_clock::now();

	// ��ʱ���ת��Ϊʱ��_t ����
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);

	// ��ʱ��_t ����ת��Ϊ����ʱ��
	std::tm* now_tm = std::localtime(&now_c);

	// ʹ�� stringstream ��ʽ�����ں�ʱ��
	std::ostringstream oss;
	oss << std::put_time(now_tm, "%Y.%m.%d %H:%M "); // ��ʽ��Ϊ yyyy-mm-dd hh:mm:ss

	return oss.str();
}
void main(void)
{
	inithash();
	char control[3];
	printf("------------�������������Ϣ-------------\n");
	nowt = getCurrentDateTime();
	printf("���������ֶ�����\n");
	cin >> team_fir;

	printf("��������ֶ�����\n");
	cin >> team_sec;
	printf("�����������(������)\n");
	cin >> city;

	InitPage(); //��ʼ��ҳ��
	while (1)
	{
		ModelShow(); //ģʽ��ʾ
		switch (msg.uMsg)
		{
		case WM_LBUTTONDOWN: //����������
			if (msg.x >= 255 && msg.x < 520 && msg.y >= 180 && msg.y <= 230) //���˻���ս������
			{
				playerVSAI(); //�˻���ս ���ִ��
				InitPage(); //������ʾҳ��
				break;
			}
			if (msg.x >= 255 && msg.x < 520 && msg.y >= 250 && msg.y <= 300) //�����˶�ս������
			{
				cur_player = 1;
				playerVSAI(); //�˻���ս ���ִ�� 
				InitPage(); //������ʾҳ��
				break;
			}
			if (msg.x >= 255 && msg.x < 520 && msg.y >= 320 && msg.y <= 370) //�����˶�ս������
			{
				cur_player = 1;
				playerVSAI(); //�˻���ս ���ִ�� 
				InitPage(); //������ʾҳ��
				break;
			}
		}
	}
	////���ٻ����������
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

void ModelShow() //ģʽ��ʾ
{
	msg = GetMouseMsg();  //��ȡ������Ϣ

	//����λ�� ��Begin !��λ��
	if (msg.x >= 255 && msg.x < 520 && msg.y >= 180 && msg.y <= 230)
	{
		settextcolor(RGB(255, 0, 0)); //�����������ɫ
		outtextxy(250, 180, L"player��!"); //255, 180 ������ָ��λ������ַ�����
	}
	else if (msg.x >= 255 && msg.x < 520 && msg.y >= 250 && msg.y <= 300)
	{
		settextcolor(RGB(255, 0, 0));	//�����������ɫ
		outtextxy(250, 250, L"player��!");
	}
	else if (msg.x >= 255 && msg.x < 520 && msg.y >= 320 && msg.y <= 370)
	{
		settextcolor(RGB(255, 0, 0));	//�����������ɫ
		outtextxy(250, 320, L"ai vs ai!");
	}
	else
	{
		settextcolor(RGB(137, 57, 196)); //�����������ɫ
		outtextxy(250, 180, L"player��!");   //�������
		outtextxy(250, 250, L"player��!");
		outtextxy(250, 320, L"ai vs ai!");
	}
}

void InitPage() //��ʼ��ҳ��
{
	initgraph(540, 540, EW_SHOWCONSOLE); //520����Ŀ� 520����ĸ� 520*520�����ص�
	loadimage(&backImg, L"3.jpg", 540, 540);
	//backImg ���ͼƬ  //ͼ�ο�  win32 MFC duilib opencv openGL qt
	putimage(0, 0, &backImg);

	//msg.  //mkCtrl �ڷ���ָ���ʱ�� �Ƿ���Ctrl
	setbkmode(TRANSPARENT);  //���ñ����ķ�� tranparent��͸���ģ�
	settextstyle(55, 0, L"����");    //��������ķ��
}

void AI()
{

}
void drawPiece(int x, int y, int color) //��������
{

	if (color == 1)
	{
		setfillcolor(WHITE); //���õ�ǰ�������ɫ�� WHITE  0xFFFFFF ��ɫ
	}
	else if (color == 2)
	{
		setfillcolor(BLACK); //��ɫ
	}
	solidcircle(x * piece_size + offsetx, image_size - y * piece_size - offsety, piece_size / 2); //���ڻ����Բ���ޱ߿򣩡�
}
void undoDraw(int x, int y, int color) // ��������
{
	// �������ӣ�������豳��ɫΪ LIGHTGRAY��
	setfillcolor(LIGHTGRAY);
	solidcircle(x * piece_size + offsetx, y * piece_size + offsety, piece_size / 2);

	// ���»���������
	setlinecolor(BLACK);
	int centerX = x * piece_size + offsetx;
	int centerY = y * piece_size + offsety;

	// ���������ߵı߽�
	int left = centerX - piece_size / 2;
	int right = centerX + piece_size / 2;
	int top = centerY - piece_size / 2;
	int bottom = centerY + piece_size / 2;

	// ���»���������
	line(left, centerY, right, centerY); // ˮƽ��
	line(centerX, top, centerX, bottom); // ��ֱ��
}


void initGame() //��Ϸ��ʼ��
{
	currentPiece = 2;//��ʼ����Ϊ��
	changePiece = -1;
	//�������ÿ�
	for (int i = 0; i < map_height; i++)
	{
		for (int j = 0; j < map_width; j++)
		{
			board_map[i][j] = 0;
		}
	}
}

void initView() //��ʼ������
{
	initgraph((map_width - 1) * piece_size + offsetx * 2, (map_height - 1) * piece_size + offsety * 2, EW_SHOWCONSOLE);
	//���ر���
	loadimage(NULL, _T("2.jpg"), map_width * piece_size + offsetx * 2, map_height * piece_size + offsety * 2); // ��ȡ����ͼƬ����ͼ���� 

	//����
	setlinecolor(BLACK);

	//����������ɫ�ͱ���ɫ
	settextcolor(WHITE);  // ����������ɫΪ��ɫ
	setbkcolor(BLACK);    // �������屳��ɫΪ��ɫ
	//setbkmode(OPAQUE);    // �������屳��ģʽΪ��͸��
	setbkmode(TRANSPARENT);
	TCHAR buffer[10]; // ���ڴ�������ַ�

	// �����ߺ��б��
	for (int i = 0; i < map_width; i++) // ����
	{
		int x = i * piece_size + offsetx;
		line(x, offsety, x, offsety + (map_height - 1) * piece_size);

		// �����б��
		_stprintf(buffer, _T("%c"), 'A' + i);
		//outtextxy(x, offsety - piece_size / 2, buffer);
		outtextxy(x, image_size - piece_size / 2, buffer);
	}

	// �����ߺ��б��
	for (int i = 0; i < map_height; i++) // ����
	{
		int y = offsety + i * piece_size;
		line(offsetx, y, (map_width - 1) * piece_size + offsetx, y);

		// �����б��
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
	std::cout << "H�� " << h << " M�� " << m << " S�� " << time << std::endl;
}
int lwx1, lwy1, lbx1, lby1, lwx2, lwy2, lbx2, lby2;//�ֱ�Ϊ��һ���ĺ���Ͱ�������
void unfallen(int y, int x)
{
	board_map[x][y] = 0;
}
void playerVSAI() //�˻���ս
{
	initGame(); //��Ϸ��ʼ��
	initView(); //ҳ���ʼ��
	GetMouseMsg();
	MOUSEMSG m;
	int x, y;
	int fflag = 0;//�ж��Ƿ��ǵ�һ������
	int flag = 0;//0Ϊ���巽�µĵ�һ������ 
	while (1)
	{
		m = GetMouseMsg(); //��ȡһ�������Ϣ
		if (m.mkRButton)
		{

			cout << "����" << endl;
			closegraph();
			initView();
			cost -= last_time;//ʱ�䷵��
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
			if (!fflag) flag = 0;//��һ���ͻ���



		}


		int tag = 0;//�ж�player�Ƿ�����
		if (cur_player == 1 && !flag)//˵��playerִ��ɫ��aiִ��ɫ
		{
			std::cout << "J 10" << std::endl;
			board_map[9][9] = 2;
			flag = 1;
			drawPiece(9, 9, currentPiece);
			in_que(currentPiece, 9, 9);
			//�ı�������ɫ
			currentPiece += changePiece;
			changePiece *= -1;
		}
		//��������������
		if (m.mkLButton) //�������
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
					if (m.mkLButton) //�������
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


			//�ж���Ӯ
			if (jud() == 1)
			{
				if (MessageBox(GetHWnd(), L"����ʤ����", L"��Ϸ������", MB_OKCANCEL) == IDOK)
				{
					print_qipu(1);
					break;
				}

			}
			else if (jud() == 2)
			{
				if (MessageBox(GetHWnd(), L"����ʤ����", L"��Ϸ������", MB_OKCANCEL) == IDOK)
				{
					print_qipu(2);
					break;
				}
			}
			if (!tag) continue;
			//std::cout << 1 << std::endl;
			//�ı�������ɫ
			currentPiece += changePiece;
			changePiece *= -1;
			//AI�ж�

			//////���ٻ����������
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
			int remain_time = 900 - cost;//ʣ��ʱ��
			auto start = std::chrono::high_resolution_clock::now();

			if (temp.first != -1 && temp.second != -1) {
				//�ɹ����
			}
			else {
				//û�д��
				if (cur_player == -1)
				{
					temp = main_init(1, board_map, remain_time);
				}
				else temp = main_init(-1, board_map, remain_time);
			}



			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = end - start;
			std::cout << "������ʱ��"; output(elapsed.count());
			last_time = elapsed.count();
			cost += elapsed.count();
			std::cout << "����ʱ��"; output(cost);

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


			//�ж���Ӯ
			if (jud() == 1)
			{
				if (MessageBox(GetHWnd(), L"����ʤ����", L"��Ϸ������", MB_OKCANCEL) == IDOK)
				{
					print_qipu(1);
					break;
				}
			}
			else if (jud() == 2)
			{
				if (MessageBox(GetHWnd(), L"����ʤ����", L"��Ϸ������", MB_OKCANCEL) == IDOK)
				{
					print_qipu(2);
					break;
				}
			}
			//std::cout << 2 << std::endl;
			//�ı�������ɫ
			currentPiece += changePiece;
			changePiece *= -1;

		}
	}
	closegraph();
}



void judge(int y, int x, int color) //�жϵ�ǰλ��4���������ӵ���������������Ϊ������ɫ 1��2��
{
	bool flag1 = false, flag2 = false; //��Ƿ�����ͷ�ǲ������෴��ɫ������
	int count = 0;//������������
	int addx = 0, addy = -1;//����
	int posx = x, posy = y;//���Ӻ��x,y
	// - ��
	while (1)
	{
		posx += addx;
		posy += addy;
		if (posy < 0)//���Խ��
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
		if (posy >= map_width)//���Խ��
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
	// | ��
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
		if (posx < 0) //���Խ��
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
		if (posx >= map_height) //���Խ��
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
	// / ��
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
		if (posx < 0 || posy >= map_width) //���Խ��
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
		if (posx >= map_height || posy < 0) //���Խ��
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
	// \ ��
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
		if (posy < 0 || posx < 0) //���Խ��
		{
			flag1 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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
		if (posx >= map_height || posy >= map_width) //���Խ��
		{
			flag2 = true;
			break;
		}
		if (board_map[posx][posy] == 0) //���û������
		{
			break;
		}
		if (board_map[posx][posy] != color) //���Ϊ�Է�����
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

bool pieceSet(int y, int x, int color) //�������ӣ�����true��ʾ���óɹ���false ��ʾ����ʧ��
{
	if (board_map[x][y] != 0)//��ǰλ��������
	{
		return false;
	}
	board_map[x][y] = color;
	return true;
}

void playerVSplayer() //���˶�ս
{
	initGame();
	initView();
	MOUSEMSG m;
	int x, y;
	while (1) {//game looping
		m = GetMouseMsg();
		//��������������
		if (m.mkLButton) {
			x = (m.x - offsetx + piece_size / 2) / piece_size;
			y = (m.y - offsety + piece_size / 2) / piece_size;
			if (pieceSet(x, y, currentPiece)) {
				drawPiece(x, y, currentPiece);
				judge(x, y, currentPiece);
				if (flag[0][0] >= 5 || flag[0][1] >= 5 || flag[0][2] >= 5 || flag[0][3] >= 5) {//�ֳ�ʤ�����˳�
					if (currentPiece == 2)
						MessageBox(GetHWnd(), L"����ʤ����", L"��Ϸ������", MB_OKCANCEL);
					else
						MessageBox(GetHWnd(), L"����ʤ����", L"��Ϸ������", MB_OKCANCEL);
					break;
				}
				//�ı�������ɫ
				currentPiece += changePiece;
				changePiece *= -1;
			}
		}
	}
	closegraph();
}

void inithash() {
	//��ʼ����Ӧ���
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
		return { -1, -1 }; // ����һ����Ч������
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
