#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <random>
#include <queue>
#include"ai.h"
#define INF 0x3f3f3f3f
#define GRID_SIZE 15     // 棋盘的规格
#define SELF_COLOR 1     // 己方的落子
#define OPPO_COLOR -1    // 对方的落子
#define NO_COLOR 0       // 空白，无落子
#define SURROUND_COLOR 2 // 合法的落子位置
#define TSSDEP 13        // TSS搜索层数
#define DISTANCE 2       // 搜寻合法落子位置的距离
#define DEP 6            // search搜索深度
#define N_TO_WIN 6       // 最大搜索长度（需要连续几子才能胜利)
#define SELF_PLAYER 1    // 代表己方玩家
#define OPPO_PLAYER -1   // 代表对方玩家
#define TIME_LIMIT 720000
#define MAXNODE 100
#define TSS_TIME 350000
#define ABTSS_TIME 200000
#define PRE_TIME_LIMIT 950000
#define PRE_ABTSS_TIME 5000

using namespace std;
int sp; // tss spend time
int isfind = 0;//ab是否出结果

//int yes;
//int no;

unsigned long long hash_table[GRID_SIZE][GRID_SIZE][3];
int nowdepth;//迭代搜索的深度
int tss_flag = 0;
int abtss_flag;
int turnID;
int limit_time;

std::map<unsigned long long, transtable> mp;
const int N = 1007;
const int Cover[24][2] = {
    {-3, 0}, {0, -3}, {3, -3}, {-3, 3}, {3, 3}, {-3, -3}, {3, 0}, {0, 3}, {2, -2}, {-2, 2}, {2, 2}, {-2, -2}, {2, 0}, {0, 2}, {-2, 0}, {0, -2}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, -1}, {-1, 1}, {1, 1}, {-1, -1} };

// 四个方向
const int dir[4][2] = {
    {0, 1}, {1, 0}, {1, 1}, {1, -1} };

const int dep_move[7] = { 0, 60, 60, 80, 80, 40, 40 };

// 按路估值分数评价，对方路和我方路的估值不同
const int score[2][7] = {
    {0, 9, 520, 2070, 7890, 10020, 1000000},
    {0, 3, 480, 2670, 3887, 4900, 1000000} };
/*
const int open_white[8][4] = {
    {1, -1, -2, 0},
    {1, 1, -2, 0},
    {1, 1, 0, -2},
    {-1, 1, 0, -2},
    {-1, 1, 2, 0},
    {-1, -1, 2, 0},
    {1, -1, 0, 2},
    {-1, -1, 0, 2} };*/

const int open_white[4][4] = {
    {-1, 0, 0, -1},
    {0, -1, 1, 0},
    {1, 0, 0, 1},
    {0, 1, -1, 0} };

class Road // 计算某条路的value
{
public:
    int road_x, road_y;
    int direction; // 这里的direction实际上有八个值，但分成两组对称
    int num[3];    // 黑子白子的数量
    bool valid;
    int idx;
    Road() {};

public:
    Road(int _road_x, int _road_y, int dir, int self, int oppo, bool _valid, int _idx)
    {
        road_x = _road_x;
        road_y = _road_y;
        direction = dir;
        num[0] = oppo;
        num[2] = self;
        idx = _idx;
        valid = _valid;
    }

    static bool is_chesslegal(int x, int y) // 判断当前落子是否在棋盘内
    {
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE)
            return true;
        else
            return false;
    }
};
class Action
{
public:
    int x1, y1; // 第一次落子位置
    int x2, y2; // 第二次落子位置
    int color;
    int step;
    int fallen;
    int w; // 当前state的评估价值
    Action() {};
    Action(int move) : x1(move / GRID_SIZE), y1(move% GRID_SIZE), fallen(move) {};
    Action(int move1, int move2) : x1(move1 / GRID_SIZE), y1(move1% GRID_SIZE), x2(move2 / GRID_SIZE), y2(move2% GRID_SIZE) {};

public:
    bool operator<(const Action& act) const
    {
        if (w != act.w)
            return w > act.w; // 按w值降序
        if (x1 != act.x1)
            return abs(x1 - GRID_SIZE / 2) < abs(act.x1 - GRID_SIZE / 2); // 给出set顺序，同时避免选点偏向某个方向
        return abs(y1 - GRID_SIZE / 2) < abs(act.y1 - GRID_SIZE / 2);
    }
};

class Table
{
public:
    // 路数组
    Road* roadVector[GRID_SIZE * GRID_SIZE * 4];
    int size;
    Table() { size = 0; };

    void Add(Road& road)
    {
        roadVector[size] = &road;
        road.idx = size++;
    }
    // 删除元素，把最后一个元素放在被删除的元素上，然后减小size
    void Remove(Road& road)
    {
        roadVector[road.idx] = roadVector[--size];
        roadVector[road.idx]->idx = road.idx;
    }
};

class Board // 记录棋盘状态
{
public:
    int cover[GRID_SIZE][GRID_SIZE]; // 覆盖范围
    Table table[7][7];
    int map[GRID_SIZE][GRID_SIZE]; // 棋盘
    Road road[GRID_SIZE][GRID_SIZE][4];
    vector<Action> all_action;
    set<int>* legal_actions;          // 合法落子集合    传指针，避免大量的申请，占用内存，可以共用一个集合
    double w;                         // 当前棋盘状态的value
    bool visit[GRID_SIZE][GRID_SIZE]; // 是否访问过

    unsigned long long state_hash;

public:
    Board()
    {
        Reset();
    }

    Board(Board* _Board) // 继承
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                map[i][j] = _Board->map[i][j];
            }
        }
        w = _Board->w;
    }

    void Reset()
    {
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 7; j++)
            {
                table[i][j].size = 0;
            }
        }
        w = 0; // 初始棋盘的价值为0
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    Road& temp = (road[i][j][k] = Road(i, j, k, 0, 0, false, 0));
                    if (temp.is_chesslegal(i + 5 * dir[k][0], j + 5 * dir[k][1]))
                    {
                        temp.valid = true;
                        table[0][0].Add(temp);
                    }
                }
                cover[i][j] = 0;
                map[i][j] = 0;
            }
        }
    }

    unsigned long long get_hash()
    {
        unsigned long long temp = 0;
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                //if (map[i][j] == 2)
                //    temp ^= hash_table[i][j][0];
                //else
                temp ^= hash_table[i][j][map[i][j] + 1];
            }
        }
        return temp;
    }
    // 获得一步落子
    Action get_single_fallen(int color)
    {
        Action act;
        act.w = -INF;
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                if (cover[i][j] && map[i][j] == 0)
                {
                    Action act1(i * GRID_SIZE + j);
                    virtualMakeMove(i * GRID_SIZE + j, color);
                    act1.w = update_w(color);
                    if (act1.w > act.w)
                    {
                        act = act1;
                    }
                    unvirtualMakeMove(i * GRID_SIZE + j, color);
                }
            }
        }
        return act;
    }
    // 获得落子
    Action get_fallen(int color)
    {
        Action act1 = get_single_fallen(color);
        virtualMakeMove(act1.x1 * GRID_SIZE + act1.y1, color);
        Action act2 = get_single_fallen(color);
        virtualMakeMove(act2.x1 * GRID_SIZE + act2.y1, color);
        Action act(act1.x1 * GRID_SIZE + act1.y1, act2.x1 * GRID_SIZE + act2.y1);
        act.w = update_w(color);
        unvirtualMakeMove(act1.x1 * GRID_SIZE + act1.y1, color);
        unvirtualMakeMove(act2.x1 * GRID_SIZE + act2.y1, color);
        return act;
    }
    int update_w(int color)
    {

        w = 0;
        for (int i = 1; i <= 6; i++)
        {
            if (color == SELF_COLOR)
                w += (double)table[i][0].size * score[0][i] - table[0][i].size * score[1][i];
            else
            {
                w += (double)table[i][0].size * score[1][i] - table[0][i].size * score[0][i];
            }
        }
        // 己方估值与对方估值不同
        return (color == SELF_COLOR ? w : -w);
    }

    void change_Mark(int x, int y)
    {
        for (int i = -DISTANCE; i <= DISTANCE; i++)
        {
            for (int j = -DISTANCE; j <= DISTANCE; j++)
            {
                int x1 = x + i;
                int y1 = y + j;
                if (x1 >= 0 && x1 < GRID_SIZE && y1 >= 0 && y1 < GRID_SIZE && map[x1][y1] == NO_COLOR)
                {
                    map[x1][y1] = SURROUND_COLOR;
                }
            }
        }
    }

    // 可能的落子
    void virtualMakeMove(int move, int color)
    {
        int x, y;
        x = move / GRID_SIZE, y = move % GRID_SIZE;
        state_hash = state_hash ^ hash_table[x][y][1] ^ hash_table[x][y][color + 1];//0423hash修改
        map[x][y] = color;
        for (int i = 0; i < 4; i++)
        {
            int xx = x, yy = y;
            if (road[xx][yy][i].valid)
            {
                Road& temp = road[xx][yy][i];
                table[temp.num[2]][temp.num[0]].Remove(temp);
                temp.num[color + 1]++;
                table[temp.num[2]][temp.num[0]].Add(temp);
            }
            for (int j = 0; j < 5; j++)
            {
                xx -= dir[i][0], yy -= dir[i][1];
                if (Road::is_chesslegal(xx, yy))
                {
                    if (road[xx][yy][i].valid)
                    {
                        Road& temp = road[xx][yy][i];
                        table[temp.num[2]][temp.num[0]].Remove(temp);
                        temp.num[color + 1]++;
                        table[temp.num[2]][temp.num[0]].Add(temp);
                    }
                }
                else
                    break;
            }
        }
    }

    void unvirtualMakeMove(int move, int color)
    {
        int x, y;
        x = move / GRID_SIZE, y = move % GRID_SIZE;
        state_hash = state_hash ^ hash_table[x][y][color + 1] ^ hash_table[x][y][1];//0423hash修改
        map[x][y] = 0;
        for (int i = 0; i < 4; i++)
        {
            int xx = x, yy = y;
            if (road[xx][yy][i].valid)
            {
                Road& temp = road[xx][yy][i];
                table[temp.num[2]][temp.num[0]].Remove(temp);
                temp.num[color + 1]--;
                table[temp.num[2]][temp.num[0]].Add(temp);
            }
            for (int j = 0; j < 5; j++)
            {
                xx -= dir[i][0], yy -= dir[i][1];
                if (Road::is_chesslegal(xx, yy))
                {
                    if (road[xx][yy][i].valid)
                    {
                        Road& temp = road[xx][yy][i];
                        table[temp.num[2]][temp.num[0]].Remove(temp);
                        temp.num[color + 1]--;
                        table[temp.num[2]][temp.num[0]].Add(temp);
                    }
                }
                else
                    break;
            }
        }
    }

    void fallen_Fruit(int move, int color) // 落子 传入的本方or对方即为棋子颜色
    {
        int x, y;
        x = move / GRID_SIZE, y = move % GRID_SIZE;
        state_hash = state_hash ^ hash_table[x][y][1] ^ hash_table[x][y][color + 1];//0423hash修改
        map[x][y] = color;
        cover[x][y]++;
        // 落子之后更新覆盖范围
        for (int i = 0; i < 24; i++)
        {
            int xx = x + Cover[i][0], yy = y + Cover[i][1];
            if (Road::is_chesslegal(xx, yy))
            {
                cover[xx][yy]++;
            }
        }
        // 更新路表
        for (int i = 0; i < 4; i++)
        {
            int xx = x, yy = y;
            if (Road::is_chesslegal(xx + 5 * dir[i][0], yy + 5 * dir[i][1]))
            {
                Road& temp = road[xx][yy][i];
                table[temp.num[2]][temp.num[0]].Remove(temp);
                temp.num[color + 1]++;
                table[temp.num[2]][temp.num[0]].Add(temp);
            }
            for (int j = 0; j < 5; j++)
            {
                xx -= dir[i][0], yy -= dir[i][1];
                if (Road::is_chesslegal(xx, yy))
                {
                    if (Road::is_chesslegal(xx + 5 * dir[i][0], yy + 5 * dir[i][1]))
                    {
                        Road& temp = road[xx][yy][i];
                        table[temp.num[2]][temp.num[0]].Remove(temp);
                        temp.num[color + 1]++;
                        table[temp.num[2]][temp.num[0]].Add(temp);
                    }
                }
                else
                    break;
            }
        }
    }
    void unfallen_Fruit(int move, int color)
    {

        int x, y;
        x = move / GRID_SIZE, y = move % GRID_SIZE;
        state_hash = state_hash ^ hash_table[x][y][color + 1] ^ hash_table[x][y][1];//0423hash修改
        map[x][y] = 0;
        cover[x][y]--;
        for (int i = 0; i < 24; i++)
        {
            int xx = x + Cover[i][0], yy = y + Cover[i][1];
            if (Road::is_chesslegal(xx, yy))
            {
                cover[xx][yy]--;
            }
        }
        for (int i = 0; i < 4; i++)
        {
            int xx = x, yy = y;
            if (Road::is_chesslegal(xx + 5 * dir[i][0], yy + 5 * dir[i][1]))
            {
                Road& temp = road[xx][yy][i];
                table[temp.num[2]][temp.num[0]].Remove(temp);
                temp.num[color + 1]--;
                table[temp.num[2]][temp.num[0]].Add(temp);
            }
            for (int j = 0; j < 5; j++)
            {
                xx -= dir[i][0], yy -= dir[i][1];
                if (Road::is_chesslegal(xx, yy))
                {
                    if (Road::is_chesslegal(xx + 5 * dir[i][0], yy + 5 * dir[i][1]))
                    {

                        Road& temp = road[xx][yy][i];
                        table[temp.num[2]][temp.num[0]].Remove(temp);
                        temp.num[color + 1]--;
                        table[temp.num[2]][temp.num[0]].Add(temp);
                    }
                }

                else
                    break;
            }
        }
    }
    // 更新有效路
    void get_valid()
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    if (road[i][j][k].num[0] > 0 && road[i][j][k].num[2] > 0)
                    {
                        road[i][j][k].valid = 0;
                    }
                    else if (Road::is_chesslegal(i + 5 * dir[k][0], j + 5 * dir[k][1]))
                    {
                        road[i][j][k].valid = 1;
                    }
                }
            }
        }
    }

    // 计算是否存在双迫着
    int get_DoubleThreats(int color)
    {
        Table& four = (color == SELF_COLOR ? table[0][4] : table[4][0]);
        Table& five = (color == SELF_COLOR ? table[0][5] : table[5][0]);
        // cout << "123:" << four.size << " " << five.size << endl;
        if (four.size + five.size == 0)
            return 0;
        // 如果四字就存在双迫着，五子一定存在双迫着
        Table& tab = (four.size == 0 ? five : four);

        for (int i = 0; i < 6; i++)
        {
            int x = tab.roadVector[0]->road_x + i * dir[tab.roadVector[0]->direction][0];
            int y = tab.roadVector[0]->road_y + i * dir[tab.roadVector[0]->direction][1];
            if (Road::is_chesslegal(x, y))
            {
                if (map[x][y] != 0)
                    continue;
                // 获取空格，虚拟落子，如果迫着消失，证明是单迫着
                virtualMakeMove(x * GRID_SIZE + y, color);
                int t = four.size + five.size;
                unvirtualMakeMove(x * GRID_SIZE + y, color);
                if (t == 0)
                    return 1;
            }
        }
        return 2;
    }

    // 获得所有的迫着数量
    int get_AllThreats(int color)
    {
        Table& four = (color == SELF_COLOR ? table[0][4] : table[4][0]);
        Table& five = (color == SELF_COLOR ? table[0][5] : table[5][0]);
        memset(visit, 0, sizeof visit);
        all_action.clear();
        if (five.size + four.size == 0)
            return 0;

        Table& temp = (four.size == 0 ? five : four);

        for (int i = 0; i < 6; i++)
        {
            int x = temp.roadVector[0]->road_x + i * dir[temp.roadVector[0]->direction][0];
            int y = temp.roadVector[0]->road_y + i * dir[temp.roadVector[0]->direction][1];
            if (Road::is_chesslegal(x, y))
            {
                if (map[x][y] != 0)
                    continue;
                virtualMakeMove(x * GRID_SIZE + y, color);
                int t = four.size + five.size;
                unvirtualMakeMove(x * GRID_SIZE + y, color);
                if (t == 0)
                    return 1;
            }
        }

        for (int i = 0; i < five.size; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                int x = five.roadVector[i]->road_x + j * dir[five.roadVector[i]->direction][0];
                int y = five.roadVector[i]->road_y + j * dir[five.roadVector[i]->direction][1];
                if (Road::is_chesslegal(x, y))
                {
                    if (map[x][y] != 0)
                        continue;
                    Action action(x * GRID_SIZE + y);
                    if (visit[x][y] == 0)
                    {
                        visit[x][y] = 1;
                        all_action.push_back(action);
                    }
                }
            }
        }

        for (int i = 0; i < four.size; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                int x = four.roadVector[i]->road_x + j * dir[four.roadVector[i]->direction][0];
                int y = four.roadVector[i]->road_y + j * dir[four.roadVector[i]->direction][1];
                if (Road::is_chesslegal(x, y))
                {
                    if (map[x][y] != 0)
                        continue;
                    Action action(x * GRID_SIZE + y);
                    if (visit[x][y] == 0)
                    {
                        visit[x][y] = 1;
                        all_action.push_back(action);
                    }
                }
            }
        }

        bool flag = 0;

        for (int i = 0; i < all_action.size() && !flag; i++)
        {
            for (int j = i + 1; j < all_action.size() && !flag; j++)
            {
                virtualMakeMove(all_action[i].x1 * GRID_SIZE + all_action[i].y1, color);
                virtualMakeMove(all_action[j].x1 * GRID_SIZE + all_action[j].y1, color);
                if (four.size + five.size == 0)
                {
                    flag = 1;
                }
                unvirtualMakeMove(all_action[i].x1 * GRID_SIZE + all_action[i].y1, color);
                unvirtualMakeMove(all_action[j].x1 * GRID_SIZE + all_action[j].y1, color);
            }
        }

        return (flag ? 2 : 3);
    }
};
class Move
{
public:
    vector<Action> single_action;
    vector<Action> action;
    bool visit[GRID_SIZE][GRID_SIZE];

    // 产生必胜着法
    void generateWinMove(Board& board, int color)
    {
        Table& four = (color == SELF_COLOR ? board.table[4][0] : board.table[0][4]);
        Table& five = (color == SELF_COLOR ? board.table[5][0] : board.table[0][5]);
        Action act;
        if (four.size != 0)
        {
            for (int i = 0, ind = 0; i < 6; i++)
            {
                int x = four.roadVector[0]->road_x + i * dir[four.roadVector[0]->direction][0];
                int y = four.roadVector[0]->road_y + i * dir[four.roadVector[0]->direction][1];
                if (board.map[x][y] != 0)
                    continue;
                if (ind == 0)
                {
                    act.x1 = x;
                    act.y1 = y;
                    ind++;
                }
                else
                {
                    act.x2 = x;
                    act.y2 = y;
                }
            }
            board.virtualMakeMove(act.x1 * GRID_SIZE + act.y1, color);
            board.virtualMakeMove(act.x2 * GRID_SIZE + act.y2, color);
            act.w = board.update_w(color);
            board.unvirtualMakeMove(act.x1 * GRID_SIZE + act.y1, color);
            board.unvirtualMakeMove(act.x2 * GRID_SIZE + act.y2, color);
            action.push_back(act);
        }

        if (five.size != 0 && action.size() == 0)
        {
            act = board.get_fallen(color);
            for (int i = 0; i < 6; i++)
            {
                int x = five.roadVector[0]->road_x + i * dir[five.roadVector[0]->direction][0];
                int y = five.roadVector[0]->road_y + i * dir[five.roadVector[0]->direction][1];
                if (board.map[x][y] != 0)
                    continue;
                act.x1 = x;
                act.y1 = y;
            }
            board.virtualMakeMove(act.x1 * GRID_SIZE + act.y1, color);
            board.virtualMakeMove(act.x2 * GRID_SIZE + act.y2, color);
            act.w = board.update_w(color);
            board.unvirtualMakeMove(act.x1 * GRID_SIZE + act.y1, color);
            board.unvirtualMakeMove(act.x2 * GRID_SIZE + act.y2, color);
            action.push_back(act);
        }
    }

    // 两个子全部用来堵
    void generateDoubleBlocks(Board& board, int color)
    {
        // 对方连四或者连五
        Table& four = (color == SELF_COLOR ? board.table[0][4] : board.table[4][0]);
        Table& five = (color == SELF_COLOR ? board.table[0][5] : board.table[5][0]);
        int lenfour = four.size, lenfive = five.size;
        single_action.clear();
        memset(visit, false, sizeof(visit));
        // 先把空白块存起来
        for (int k = 0; k < lenfour; k++)
        {
            findblanks(four.roadVector[k], board);
        }

        for (int k = 0; k < lenfive; k++)
        {
            findblanks(five.roadVector[k], board);
        }

        int rowlen = single_action.size();
        // 计算空白块，看能不能真正破除威胁
        for (int i = 0; i < rowlen; i++)
        {
            for (int j = i + 1; j < rowlen; j++)
            {
                board.virtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                board.virtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
                // 能破除，上升
                if (four.size + five.size == 0)
                {
                    Action act(single_action[i].x1 * GRID_SIZE + single_action[i].y1, single_action[j].x1 * GRID_SIZE + single_action[j].y1);
                    act.w = board.update_w(color);
                    action.push_back(act);
                }
                board.unvirtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                board.unvirtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
            }
        }
        if (action.size() == 0)
        {
            Action act = board.get_fallen(color);
            board.virtualMakeMove(act.x1 * GRID_SIZE + act.y1, color);
            board.virtualMakeMove(act.x2 * GRID_SIZE + act.y2, color);
            act.w = board.update_w(color);
            board.unvirtualMakeMove(act.x1 * GRID_SIZE + act.y1, color);
            board.unvirtualMakeMove(act.x2 * GRID_SIZE + act.y2, color);
            action.push_back(act);
        }
    }
    void generateSingleBlocks1(Board& board, int color)
    {
        // 对方连四或者连五
        Table& four = (color == SELF_COLOR ? board.table[0][4] : board.table[4][0]);
        Table& five = (color == SELF_COLOR ? board.table[0][5] : board.table[5][0]);
        int lenfour = four.size, lenfive = five.size;
        single_action.clear();
        memset(visit, false, sizeof(visit));
        // 先把空白块存起来
        for (int k = 0; k < lenfour; k++)
        {
            findblanks(four.roadVector[k], board);
        }

        for (int k = 0; k < lenfive; k++)
        {
            findblanks(five.roadVector[k], board);
        }

        int len1 = single_action.size();
        memset(visit, false, sizeof(visit));
        getPotentialPoint(board, color);
        int len2 = single_action.size();
        if (len1 == len2)
            return;
        // 计算空白块，看能不能真正破除威胁
        for (int i = 0; i < len1; i++) // 枚举防守点
        {
            board.virtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
            // 能破除，上升
            if (four.size + five.size == 0)
            {
                for (int j = len1; j < len2; j++)
                {
                    if (single_action[j].x1 != single_action[i].x1 || single_action[j].y1 != single_action[i].y1)
                    {
                        board.virtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                        if (board.get_DoubleThreats(-color) >= 2)
                        {
                            // cout << single_action[i].x1 << " " << single_action[i].y1 << " " << single_action[j].x1 << " " << single_action[j].y1 << endl;
                            Action act(single_action[i].x1 * GRID_SIZE + single_action[i].y1, single_action[j].x1 * GRID_SIZE + single_action[j].y1);
                            action.push_back(act);
                        }
                        board.unvirtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                    }
                }
            }
            board.unvirtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
        }
    }
    // 堵一个攻击另一个
    void generateSingleBlocks(Board& board, int color)
    {
        // 先找对方威胁的路并堵上
        Table& four = (color == SELF_COLOR ? board.table[0][4] : board.table[4][0]);
        Table& five = (color == SELF_COLOR ? board.table[0][5] : board.table[5][0]);
        int lenfour = four.size, lenfive = five.size;
        single_action.clear();
        memset(visit, false, sizeof(visit));
        // 找出可堵的点 放入singleaction中
        for (int k = 0; k < lenfour; k++)
        {
            findblanks(four.roadVector[k], board);
        }

        for (int k = 0; k < lenfive; k++)
        {
            findblanks(five.roadVector[k], board);
        }

        // 防御第一个点，攻击第二个点
        int temp1 = single_action.size();
        getGoodConnectPoint(board, color);
        int tag = 0;
        int temp2 = temp1 + (int)((single_action.size() - temp1));
        for (int i = 0; i < temp1; i++)
        {
            // 破除
            board.virtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
            if (four.size + five.size == 0)
            {
                for (int j = i + 1; j < temp2; j++)
                {
                    tag = 1;
                    board.virtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                    Action act(single_action[i].x1 * GRID_SIZE + single_action[i].y1, single_action[j].x1 * GRID_SIZE + single_action[j].y1);
                    act.w = board.update_w(color);
                    action.push_back(act);
                    board.unvirtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                }
                if (!tag)
                {
                    for (int i = 0; i < GRID_SIZE; i++)
                    {
                        for (int j = 0; j < GRID_SIZE; j++)
                        {
                            if (board.map[i][j] == 0)
                            {
                                Action act(single_action[0].x1 * GRID_SIZE + single_action[0].y1, i * GRID_SIZE + j);
                                act.w = board.update_w(color);
                                action.push_back(act);
                            }
                        }
                    }
                }
            }
            board.unvirtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
        }
    }

    // 产生连接上的好点，定义好点：我方连1，2，3，对方连2，3 对方连1也可以
    void getGoodConnectPoint(Board& board, int color)
    {
        //0424 修改
        //Table& one = (color == SELF_COLOR ? board.table[0][1] : board.table[1][0]);
        Table& two = (color == SELF_COLOR ? board.table[0][2] : board.table[2][0]);
        Table& three = (color == SELF_COLOR ? board.table[0][3] : board.table[3][0]);
        Table& ownOne = (color == SELF_COLOR ? board.table[1][0] : board.table[0][1]);
        Table& ownTwo = (color == SELF_COLOR ? board.table[2][0] : board.table[0][2]);
        Table& ownThree = (color == SELF_COLOR ? board.table[3][0] : board.table[0][3]);

        int lenOwnOne = ownOne.size, lenOwnTwo = ownTwo.size, lenthree = three.size, lentwo = two.size,
            lenOwnThree = ownThree.size;

        /*for (int k = 0; k < lenone; k++)
        {
            findblanks(one.roadVector[k], board);
        }*/
        for (int k = 0; k < lenthree; k++)
        {
            findblanks(three.roadVector[k], board);
        }

        for (int k = 0; k < lentwo; k++)
        {
            findblanks(two.roadVector[k], board);
        }
        for (int k = 0; k < lenOwnThree; k++)
        {
            findblanks(ownThree.roadVector[k], board);
        }
        for (int k = 0; k < lenOwnTwo; k++)
        {
            findblanks(ownTwo.roadVector[k], board);
        }

        for (int k = 0; k < lenOwnOne; k++)
        {
            findblanks(ownOne.roadVector[k], board);
        }
        int len = single_action.size();
        // 计算评分
        for (int i = 0; i < len; i++)
        {
            board.virtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
            single_action[i].w = board.update_w(color);
            board.unvirtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
        }
    }

    // 获取潜力点，即可能成为威胁的点
    void getPotentialPoint(Board& board, int color)
    {
        // 我方连二或者连三的点即为潜力点
        Table& ownTwo = (color == SELF_COLOR ? board.table[2][0] : board.table[0][2]);
        Table& ownThree = (color == SELF_COLOR ? board.table[3][0] : board.table[0][3]);
        int lenOwnTwo = ownTwo.size, lenOwnThree = ownThree.size;

        for (int k = 0; k < lenOwnThree; k++)
        {
            findblanks(ownThree.roadVector[k], board);
        }

        for (int k = 0; k < lenOwnTwo; k++)
        {
            findblanks(ownTwo.roadVector[k], board);
        }
    }
    void findblanks(Road* road, Board& board)
    {
        for (int i = 0; i < 6; i++)
        {
            int x = road->road_x + i * dir[road->direction][0];
            int y = road->road_y + i * dir[road->direction][1];
            if (Road::is_chesslegal(x, y))
            {
                if (board.map[x][y] != 0)
                    continue;
                if (visit[x][y] == 0)
                {
                    Action p(x * GRID_SIZE + y);
                    single_action.push_back(p);
                    visit[x][y] = true;
                }
            }
        }
    }
    // 产生着法，用于alpha_beta搜索
    void createMove(Board& board, int color)
    {
        action.clear();
        single_action.clear();
        // 先调用getWinMove查看有没有必胜着法
        generateWinMove(board, color);
        if (action.size() != 0)
            return;
        // 计算对方威胁
        int threats = board.get_AllThreats(color);
        // 堵一个攻击另一个
        if (threats == 1)
        {
            generateSingleBlocks(board, color);
        }
        // 两个全部用来堵
        else if (threats >= 2)
        {
            generateDoubleBlocks(board, color);
        }
        // 产生好点
        else
        {
            generateGoodValuePoint(board, color);
        }
        // 排序，启发式搜索
        sort(action.begin(), action.end());
    }

    // 双威胁着法，用于TSS搜索
    void createDoubleThreatsMove(Board& board, int color)
    {
        action.clear();
        single_action.clear();
        // 首先生成必胜着法
        generateWinMove(board, color);
        if (action.size() != 0)
            return;
        // 计算对方威胁数

        // for (int i = 0; i < 15; i++) {
        //	for (int j = 0; j < 15; j++) {
        //		if (board.map[i][j] == 1)cout << "o ";
        //		else if (board.map[i][j] == -1)cout << "x ";
        //		else cout << "  ";
        //	}
        //	cout << endl;
        // }

        int threats = board.get_DoubleThreats(color);
        // cout << "threats= " << threats << endl;
        // 对方没有威胁，展开双威胁搜索
        if (threats == 0)
        {
            // cout << "1" << endl;
            generateDoubleThreats(board, color);
        }
        else if (threats == 1)
        {
            generateSingleBlocks1(board, color);
        }
        else // 对方存在威胁，必须防守
        {
            // cout << "2" << endl;
            generateDoubleBlocks(board, color);
        }
    }

    // 综合评估，产生好的着法
    void generateGoodValuePoint(Board& board, int color)
    {
        single_action.clear();
        memset(visit, false, sizeof(visit));
        getGoodConnectPoint(board, color);
        sort(single_action.begin(), single_action.end());

        int len = single_action.size();
        int sum = 0;
        int temp = len;
        // 优化，去除小于平均值的
        for (int i = 0; i < len; i++)
        {
            sum += single_action[i].w;
        }
        if (len)
            sum /= len;

        for (int i = 0; i < len; i++)
        {
            if (single_action[i].w < sum)
            {
                temp = i;
                break;
            }
        }

        for (int i = 0; i < len; i++)
        {
            for (int j = i + 1; j < temp; j++)
            {
                board.virtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                board.virtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
                Action act(single_action[i].x1 * GRID_SIZE + single_action[i].y1, single_action[j].x1 * GRID_SIZE + single_action[j].y1);
                act.w = board.update_w(color);
                action.push_back(act);
                board.unvirtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                board.unvirtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
            }
            // 优化，中心夹挤
            temp--;
        }
    }

    // 产生双威胁着法
    void generateDoubleThreats(Board& board, int color)
    {

        single_action.clear();
        memset(visit, false, sizeof(visit));
        getPotentialPoint(board, color);
        int len = single_action.size();

        for (int i = 0; i < len; i++)
        {
            for (int j = i + 1; j < len; j++)
            {
                // 从潜力点中进行筛选，得到真正威胁点
                board.virtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                board.virtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
                if (board.get_DoubleThreats(-color) >= 2)
                {
                    // cout << single_action[i].x1 << " " << single_action[i].y1 << " " << single_action[j].x1 << " " << single_action[j].y1 << endl;
                    Action act(single_action[i].x1 * GRID_SIZE + single_action[i].y1, single_action[j].x1 * GRID_SIZE + single_action[j].y1);
                    action.push_back(act);
                }
                board.unvirtualMakeMove(single_action[j].x1 * GRID_SIZE + single_action[j].y1, color);
                board.unvirtualMakeMove(single_action[i].x1 * GRID_SIZE + single_action[i].y1, color);
            }
        }
    }
};

class Tree // 根据当前cur_board和legal_actions进行博弈树 返回最大叶子节点即当前的最大value
{
public:
    // Board* cur_state;
    // Tree(Board* _cur_state) : cur_state(_cur_state) {};
    vector<Action> fall_action;
    vector<Action> firstSteps;
    vector<Action> pp;
    Action act;
    int color;
    bool flag;
    int node;
    time_t start;
    time_t in_abstart;
    int tss_1;
    int grad;
    int isFind;

    int tss_action_1;
    int tss_action_2;
    int ab_action_1;
    int ab_action_2;

public:
    Tree()
    {
        flag = 0;
        node = 2;
        grad = 2;
        isFind = 0;
    }

    pair<int, int> fallen_action(Board& tss_board, Board& ab_board, int _color)

    {

        // return (search(&board, 5, -INF, INF, 0, NULL, NULL)).first;
        // for (int i = 0; i < GRID_SIZE; i++) {
        //	for (int j = 0; j < GRID_SIZE; j++) {
        //		cout << board.map[i][j] << " ";
        //	}
        //	cout << endl;
        // }
        // cout << endl;
        /* for (int i = 0; i < GRID_SIZE; i++)
         {
             for (int j = 0; j < GRID_SIZE; j++)
             {
                 cout << board.map[i][j];
             }
             cout << endl;
         }
         cout << endl;
         for (int i = 0; i < GRID_SIZE; i++)
         {
             for (int j = 0; j < GRID_SIZE; j++)
             {
                 cout << board.cover[i][j];
             }
             cout << endl;
         }
         cout << endl;*/

         /*tss_board.fallen_Fruit(4* GRID_SIZE + 6, color);
         tss_board.get_valid();
         tss_board.fallen_Fruit(3* GRID_SIZE + 5, color);
         tss_board.get_valid();
         time_t a = clock();
         int t = TSS(tss_board, -color, 13);
         time_t b = clock();
         cout << "using time " << b - a << endl;
         cout << t << endl;
         exit(0);*/
        tss_1 = 0; // 目前做tss_1
        color = _color;

        // time_t aa = time(0);
        start = clock(); // 起始时间;
        for (int i = 1; i <= TSSDEP; i += 2)
        {
            
            if (TSS(tss_board, color, i))
            {
                tss_flag = 1;
                // cout << "TSS success " << endl;

                // cout << "spend time on:" << b - a << endl;
                return { tss_action_1, tss_action_2 };
            }
            clock_t end = clock();
            if (end - start > TSS_TIME)
                break;
        }
        // cout << "TSS UNsuccess " << endl;
        // return { 66,66 };
        // board.get_legal_actions();
        // cout << board.legal_actions->size() << endl;
        /*for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                cout << ab_board.map[i][j] << " ";
            }
            cout << endl;
        }*/
        // return (search(&ab_board, 4, -INF, INF, 0, NULL, NULL)).first;

        // start = clock(); // 起始时间;
        tss_1 = 1;
        firstSteps.clear();
        isFind = 0;
        time_t aaa = clock();
        sp = aaa - start;

        color = -color;
        // if (turnID <= 3) score[0][1] = 0, score[0][2] = 30, score[0][3] = 300;
        int value = alphaBeta(DEP - grad, -INF, INF, -color, ab_board);
        time_t bbb = clock();
        // cout << "spend time on:" << bbb - aaa << endl;
        // cout << firstSteps.size() << endl;
        if (!isFind)
        {
            isfind = 1;//没找到
            if (pp.size() == 0)
            {
                if (firstSteps.size() != 0)
                {
                    ab_action_1 = firstSteps[0].x1 * GRID_SIZE + firstSteps[0].y1;
                    ab_action_2 = firstSteps[0].x2 * GRID_SIZE + firstSteps[0].y2;
                }
                else
                {
                    Action tt = tss_board.get_fallen(-color);
                    ab_action_1 = tt.x1 * GRID_SIZE + tt.y1;
                    ab_action_2 = tt.x2 * GRID_SIZE + tt.y2;
                }
            }
            else
            {
                int s = pp.size();
                ab_action_1 = pp[(int)s - 1].x1 * GRID_SIZE + pp[(int)s - 1].y1;
                ab_action_2 = pp[(int)s - 1].x2 * GRID_SIZE + pp[(int)s - 1].y2;
            }
        }
        return { ab_action_1, ab_action_2 };
    }

    bool TSS(Board& board, int _color, int dep)
    {

        // 如果我方连六，return true；
        int len = (color == SELF_COLOR ? board.table[6][0].size : board.table[0][6].size);

        if (len > 0)
        {
            tss_action_1 = fall_action[0].x1 * GRID_SIZE + fall_action[0].y1;
            tss_action_2 = fall_action[0].x2 * GRID_SIZE + fall_action[0].y2;
            return true;
        }

        //  如果对方存在威胁，但是我方没有威胁，return false；
        if (_color == color && board.get_AllThreats(color) >= 2 && board.get_AllThreats(-color) == 0)
            return false;

        // 如果我方威胁大于3，return true；
        if (_color == -color && (board.get_AllThreats(_color) >= 3))
        {
            if (fall_action.size() == 0)
                return true;
            tss_action_1 = fall_action[0].x1 * GRID_SIZE + fall_action[0].y1;
            tss_action_2 = fall_action[0].x2 * GRID_SIZE + fall_action[0].y2;
            return true;
        }

        // depth为0，return false；
        if (dep == 0)
            return false;

       

        // 行棋
        Move move;
        move.createDoubleThreatsMove(board, _color);
        int num = move.action.size();
        /*cout << "num=" << num << endl;
        for (int i = 0; i < num; i++)
        {
            cout << "(" << move.action[i].x1 << "," << move.action[i].y1 << ") ";
            cout << "(" << move.action[i].x2 << "," << move.action[i].y2 << ") " << endl;
        }*/

        if (num == 0)
            return false;

        for (int i = 0; i < num; i++)
        {
            // if (dep == 13)i = 4;
            board.fallen_Fruit(move.action[i].x1 * GRID_SIZE + move.action[i].y1, _color);
            board.fallen_Fruit(move.action[i].x2 * GRID_SIZE + move.action[i].y2, _color);
            board.get_valid();
            /* cout << "dep=" << dep << endl;
             cout << "curr point:(" << move.action[i].x1 << "," << move.action[i].y1 << ") | (";
             cout << move.action[i].x2 << "," << move.action[i].y2 << ")" << endl;*/

            fall_action.push_back(move.action[i]);
            bool flag1 = TSS(board, -_color, dep - 1);

            fall_action.erase(fall_action.end() - 1);
            board.unfallen_Fruit(move.action[i].x2 * GRID_SIZE + move.action[i].y2, _color);
            board.unfallen_Fruit(move.action[i].x1 * GRID_SIZE + move.action[i].y1, _color);
            board.get_valid();
            if (_color == color)
            {
                flag1 = false || flag1;
                if (flag1)
                    return true;
            }
            else
            {
                flag1 = true && flag1;
                if (!flag1)
                    return false;
            }
        }
        if (_color == color)
            return false;
        else
            return true;

    }

    int alphaBeta(int depth, int alpha, int beta, int color, Board& board)
    {
        
        /*time_t end = clock();
        if (end - start > TIME_LIMIT)
            return alpha;*/
        if (mp.find(board.state_hash) != mp.end() && mp[board.state_hash].length >= depth)//0:exact 1:fall high -1:fall down
        {
            if (mp[board.state_hash].flag == 0) return mp[board.state_hash].score;//若是该子树搜索完后直接返回score即可

            if (mp[board.state_hash].flag == 1 && mp[board.state_hash].score >= beta)
                return beta; // 记忆化
            if (mp[board.state_hash].flag == -1 && mp[board.state_hash].score <= alpha)
                return alpha; // 记忆化
        }
        if (depth == 0)
        {
            if (mp.find(board.state_hash) == mp.end())
            {
                transtable trtable;
                trtable.flag = 0;//exact
                trtable.length = depth;
                trtable.score = board.update_w(color);
                mp.insert(make_pair(board.state_hash, trtable));
            }
            return board.update_w(color);
        }
        //0423改
        //if (board.table[6][0].size + board.table[0][6].size != 0) return alpha+1;
        int temp = -INF;
        bool flag = false;
        transtable trtable;
        trtable.flag = -1;

        Move move;
        // cout << 1 << endl;
        move.createMove(board, color);
        // cout << 2 << endl;
        int num = move.action.size();


        // cout << "num:" << num << endl;
        num = min(num, dep_move[depth]);
        int maxx = -INF;
        // cout << depth<<" "<<num << endl;
        if (depth == DEP - grad)
        {
            for (int i = 0; i < num; i++)
                firstSteps.push_back(move.action[i]);
        }
        for (int i = 0; i < num; i++)
        {
            if (depth == DEP - grad)
            {
                //0423 改 给出最后时间做选择
                time_t end = clock();
                if (turnID > 3 && end - start > TIME_LIMIT)
                    //return alpha;
                    break;
                else if (turnID <= 3 && end - start > PRE_TIME_LIMIT)
                    //return alpha;
                    break;
            }
            board.fallen_Fruit(move.action[i].x1 * GRID_SIZE + move.action[i].y1, color);
            //board.get_valid();
            board.fallen_Fruit(move.action[i].x2 * GRID_SIZE + move.action[i].y2, color);
            board.get_valid();

            if (flag)
            {
                // 空窗试探
                temp = -alphaBeta(depth - 1, -alpha - 1, -alpha, -color, board);
                if (temp > alpha && temp < beta)
                {
                    temp = -alphaBeta(depth - 1, -beta, -alpha, -color, board);
                }
            }
            else
            {
                temp = -alphaBeta(depth - 1, -beta, -alpha, -color, board);
            }
            maxx = max(temp, maxx);
            board.unfallen_Fruit(move.action[i].x1 * GRID_SIZE + move.action[i].y1, color);
            // board.get_valid();
            board.unfallen_Fruit(move.action[i].x2 * GRID_SIZE + move.action[i].y2, color);
            board.get_valid();
            /* if (depth == DEP - grad)
             {
                 move.action[i].w = temp;
                 pp.push_back(move.action[i]);
             }*/
            if (temp >= beta)
            {

                if (mp.find(board.state_hash) == mp.end())//fall high 情况
                {
                    trtable.flag = 1;
                    trtable.length = depth;
                    trtable.score = maxx;
                    mp.insert(make_pair(board.state_hash, trtable));
                }
                else
                {
                    mp[board.state_hash].flag = 1;
                    mp[board.state_hash].length = depth;
                    mp[board.state_hash].score = maxx;
                }
                return beta;
            }
            /*if (depth == DEP - grad)
            {
                move.action[i].w = temp;
                pp.push_back(move.action[i]);
            }*/
            if (temp > alpha)
            {

                alpha = temp;
                flag = true;
                trtable.flag = 0;//exact alpha<value<beta

                if (depth == DEP - grad)
                {
                    move.action[i].w = temp;
                    pp.push_back(move.action[i]);

                }
            }
            
        }

        if (mp.find(board.state_hash) == mp.end())//exact 才有效
        {
            trtable.length = depth;
            trtable.score = maxx;
            mp.insert(make_pair(board.state_hash, trtable));
        }
        else
        {
            mp[board.state_hash].flag = trtable.flag;
            mp[board.state_hash].length = depth;
            mp[board.state_hash].score = maxx;
        }
        if (depth == DEP - grad)
        {
            //sort(pp.begin(), pp.end());
            int nn = (int)pp.size();

            for (int i = nn - 1; i >= 0; i--)
            {
                //cout << nn << endl;
                Action tt = pp[i];
                // cout << tt.x1 << " " << tt.y1 << " "  << tt.x2 << " " << tt.y2 <<endl;
                board.fallen_Fruit(tt.x1 * GRID_SIZE + tt.y1, color);
                // board.get_valid();
                board.fallen_Fruit(tt.x2 * GRID_SIZE + tt.y2, color);
                board.get_valid();
                int t = 0;
                in_abstart = clock();

                if (turnID > 3)
                {
                    for (int j = 1; j <= TSSDEP - 4; j += 2)
                    {
                        t = TSS(board, -color, j);
                        time_t end = clock();
                        if (end - in_abstart > ABTSS_TIME)
                        {
                            //cout << "ontime" << endl;
                            t = 1;
                        }
                        if (t)
                            break;
                        //0423 修改
                        end = clock();
                        if (end - start > TIME_LIMIT + 180000)
                            return alpha;
                    }
                }

                //cout << " " << t << endl;
                if (!t)
                {
                    ab_action_1 = tt.x1 * GRID_SIZE + tt.y1;
                    ab_action_2 = tt.x2 * GRID_SIZE + tt.y2;
                    isFind = 1;
                    return alpha;
                }
                if (i == 0)
                {
                    ab_action_1 = pp[(int)nn - 1].x1 * GRID_SIZE + pp[(int)nn - 1].y1;
                    ab_action_2 = pp[(int)nn - 1].x2 * GRID_SIZE + pp[(int)nn - 1].y2;
                    isFind = 1;
                    return alpha;
                }
                board.unfallen_Fruit(tt.x1 * GRID_SIZE + tt.y1, color);
                // board.get_valid();
                board.unfallen_Fruit(tt.x2 * GRID_SIZE + tt.y2, color);
                board.get_valid();
            }
        }

        return alpha;
    }
};

class Game // 根据moves 更新棋盘状态 包括恢复当前状态与实现下一状态
{
public:
    // Board cur_board;
    int cur_player;
    int chesses;

    bool is_legalfallen(Board& board, int move); // 判断当前落子是否合法 （即当前位置是否可放）

    bool is_end(Board& state, int move); // 根据cur_state状态判断当前是否结束

    void make_fallen(Board& board, int move); // 落子 调用cur_board的落子

    void change_Player(); // 刷新cur_player与chesses
    Game(Board& board)
    {
        // cur_board = board;  // 棋盘初始化
        cur_player = SELF_PLAYER; // 默认己方先手
        chesses = 1;              // 开局只能下一子
    }
    // Game(Board* state, int color) // 继承棋盘以及落子方
    //{
    //	cur_board = Board(state, NULL); // 棋盘继承
    //	cur_player = color;                 // 颜色继承
    //	chesses = 2;                        // 每回合余棋初始化为2
    // }
};
bool Game::is_legalfallen(Board& board, int move) // 判断当前落子是否合法 （即当前位置是否可放）
{
    int x = move / GRID_SIZE;
    int y = move % GRID_SIZE;
    if (x < 0 || y < 0 || x >= GRID_SIZE || y >= GRID_SIZE)
        return false;
    if (board.map[x][y] != 0)
        return false;
}

bool Game::is_end(Board& state, int move) // 根据cur_state状态判断当前是否结束
{
    int x = move / GRID_SIZE, y = move % GRID_SIZE;
    int cur_color = state.map[x][y];
    int count, i, j;

    // 竖排六子
    i = x;
    count = -1;
    while (i >= 0 && state.map[i][y] == cur_color)
    {
        i--;
        count++;
    }
    i = x;
    while (i < GRID_SIZE && state.map[i][y] == cur_color)
    {
        i++;
        count++;
    }
    if (count >= 6)
        return cur_color;

    // 横排六子
    j = y;
    count = -1;
    while (j >= 0 && state.map[x][j] == cur_color)
    {
        j--;
        count++;
    }
    j = y;
    while (j <= GRID_SIZE && state.map[x][j] == cur_color)
    {
        j++;
        count++;
    }
    if (count >= 6)
        return cur_color;

    // 右斜六子
    i = x;
    j = y;
    count = -1;
    while (i >= 0 && j < GRID_SIZE && state.map[i][j] == cur_color)
    {
        i--;
        j++;
        count++;
    }
    i = x;
    j = y;
    while (i < GRID_SIZE && j >= 0 && state.map[i][j] == cur_color)
    {
        i++;
        j--;
        count++;
    }
    if (count >= 6)
        return cur_color;

    // 左斜六子
    i = x;
    j = y;
    count = -1;
    while (i >= 0 && j >= 0 && state.map[i][j] == cur_color)
    {
        i--;
        j--;
        count++;
    }
    i = x;
    j = y;
    while (i < GRID_SIZE && j < GRID_SIZE && state.map[i][j] == cur_color)
    {
        i++;
        j++;
        count++;
    }
    if (count >= 6)
        return cur_color;

    return 0;
}
void Game::make_fallen(Board& board, int move) // 落子 调用cur_board的落子
{
    int x = move / GRID_SIZE, y = move % GRID_SIZE;
    if (x == -1 && y == -1)
        return; // 不落子判断
    board.fallen_Fruit(move, cur_player);
    // board.get_valid();
    chesses--;
    if (chesses == 0)
    {
        change_Player();
    }
}
void Game::change_Player() // 刷新cur_player与chesses
{
    chesses = 2;
    cur_player = -cur_player; // 更换执棋者
}
class AIplayer // 根据已有棋盘 推测下一步行动 返回最大价值的落子
{
public:
    Board tss_board;
    Board* ab_board;
    Tree* root;
    Game* game;
    int color;
    int last_action1, last_action2;
    AIplayer(Board& _cur_state, Game* _game, int _last_action1, int _last_action2)
    {
        game = _game;
        tss_board = _cur_state;
        ab_board = &_cur_state;
        root = new Tree();
        last_action1 = _last_action1;
        last_action2 = _last_action2;
        color = SELF_COLOR;
    }

    pair<int, int> get_fallenmove() // 调用Tree->search() 返回move
    {
        pair<int, int> act = root->fallen_action(tss_board, *ab_board, color);
        return { act.first, act.second };
    }
};
class opening_node
{
public:
    int h;
    int x1;
    int y1;
    int x2;
    int y2;
    opening_node()
    {
        h = 0x3f3f3f3f;
    }
    opening_node(int _h, int _x1, int _y1, int _x2, int _y2)
    {
        h = _h;
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
    }
};

class opening_hash
{
public:
    opening_node h[N];

    int find(int x)
    {
        int k = (x % N + N) % N;
        while (h[k].h != x && h[k].h != 0x3f3f3f3f)
        {
            k++;
            if (k == N)
                k = 0;
        }
        return k;
    }

    int get_hash(int x1, int y1, int x2, int y2)
    {
        return ((x1 * GRID_SIZE + y1) * GRID_SIZE + x2) * GRID_SIZE + y2;
    }

    opening_hash()
    {
        int key;
        int t;
        //*************************************************************
        key = get_hash(7, 8, 8, 7); // 红1
        t = find(key);
        h[t] = { key, 5, 9, 6, 9 };

        key = get_hash(7, 6, 8, 7);
        t = find(key);
        h[t] = { key, 9, 8, 9, 9 };

        key = get_hash(6, 7, 7, 6);
        t = find(key);
        h[t] = { key, 8, 5, 9, 5 };

        key = get_hash(6, 7, 7, 8);
        t = find(key);
        h[t] = { key, 5, 5, 5, 6 };

        key = get_hash(7, 8, 9, 6); // 红2
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };
        key = get_hash(5, 6, 7, 8);
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };

        key = get_hash(6, 5, 8, 7);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };
        key = get_hash(6, 9, 8, 7);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };

        key = get_hash(5, 8, 7, 6);
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };
        key = get_hash(7, 6, 9, 8);
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };

        key = get_hash(6, 7, 8, 9);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };
        key = get_hash(6, 7, 8, 5);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };

        key = get_hash(7, 8, 8, 6); // 红3
        t = find(key);
        h[t] = { key, 8, 8, 9, 9 };
        key = get_hash(6, 6, 7, 8);
        t = find(key);
        h[t] = { key, 6, 8, 5, 9 };

        key = get_hash(6, 6, 8, 7);
        t = find(key);
        h[t] = { key, 8, 6, 9, 5 };
        key = get_hash(6, 8, 8, 7);
        t = find(key);
        h[t] = { key, 8, 8, 9, 9 };

        key = get_hash(6, 8, 7, 6);
        t = find(key);
        h[t] = { key, 5, 5, 6, 6 };
        key = get_hash(7, 6, 8, 8);
        t = find(key);
        h[t] = { key, 8, 6, 9, 5 };

        key = get_hash(6, 7, 8, 8);
        t = find(key);
        h[t] = { key, 6, 8, 5, 9 };
        key = get_hash(6, 7, 8, 6);
        t = find(key);
        h[t] = { key, 5, 5, 6, 6 };
        //*************************************************************
        key = get_hash(7, 8, 8, 9); // 蓝1
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };
        key = get_hash(6, 9, 7, 8);
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };
        key = get_hash(7, 6, 8, 5);
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };
        key = get_hash(6, 5, 7, 6);
        t = find(key);
        h[t] = { key, 6, 7, 8, 7 };

        key = get_hash(8, 7, 9, 6);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };
        key = get_hash(8, 7, 9, 8);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };

        key = get_hash(5, 6, 6, 7);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };
        key = get_hash(5, 8, 6, 7);
        t = find(key);
        h[t] = { key, 7, 6, 7, 8 };

        key = get_hash(6, 8, 9, 9); // 蓝2
        t = find(key);
        h[t] = { key, 5, 7, 6, 6 };
        key = get_hash(5, 9, 8, 8); // 蓝2
        t = find(key);
        h[t] = { key, 8, 6, 9, 7 };

        key = get_hash(8, 8, 9, 5); // 蓝2
        t = find(key);
        h[t] = { key, 6, 8, 7, 9 };
        key = get_hash(8, 6, 9, 9); // 蓝2
        t = find(key);
        h[t] = { key, 7, 5, 6, 6 };

        key = get_hash(5, 5, 8, 6); // 蓝2
        t = find(key);
        h[t] = { key, 8, 8, 9, 7 };
        key = get_hash(6, 6, 9, 5); // 蓝2
        t = find(key);
        h[t] = { key, 5, 7, 6, 8 };

        key = get_hash(5, 9, 6, 6); // 蓝2
        t = find(key);
        h[t] = { key, 8, 6, 7, 5 };
        key = get_hash(5, 5, 6, 8); // 蓝2
        t = find(key);
        h[t] = { key, 7, 9, 8, 8 };

        key = get_hash(6, 8, 7, 9); // 蓝3  
        t = find(key);
        h[t] = { key, 5, 7, 6, 7 };
        key = get_hash(7, 9, 8, 8);
        t = find(key);
        h[t] = { key, 8, 7, 9, 7 };
        key = get_hash(6, 6, 7, 5);
        t = find(key);
        h[t] = { key, 5, 7, 6, 7 };
        key = get_hash(7, 5, 8, 6);
        t = find(key);
        h[t] = { key, 8, 7, 9, 7 };

        key = get_hash(5, 7, 6, 8);
        t = find(key);
        h[t] = { key, 7, 8, 7, 9 };
        key = get_hash(8, 8, 9, 7);
        t = find(key);
        h[t] = { key, 7, 8, 7, 9 };
        key = get_hash(5, 7, 6, 6);
        t = find(key);
        h[t] = { key, 7, 5, 7, 6 };
        key = get_hash(8, 6, 9, 7);
        t = find(key);
        h[t] = { key, 7, 5, 7, 6 };

        /*key = get_hash(7, 5, 8, 6); // 蓝3   //修改3
        t = find(key);
        h[t] = { key, 6, 4, 9, 6 };
        key = get_hash(6, 6, 7, 5);
        t = find(key);
        h[t] = { key, 5, 6, 8, 4 };

        key = get_hash(5, 7, 6, 6);
        t = find(key);
        h[t] = { key, 4, 8, 6, 5 };
        key = get_hash(5, 7, 6, 8);
        t = find(key);
        h[t] = { key, 4, 6, 6, 9 };

        key = get_hash(6, 8, 7, 9);
        t = find(key);
        h[t] = { key, 5, 8, 8, 10 };
        key = get_hash(7, 9, 8, 8);
        t = find(key);
        h[t] = { key, 6, 10, 9, 8 };
        key = get_hash(8, 8, 9, 7);
        t = find(key);
        h[t] = { key, 8, 9, 10, 6 };
        key = get_hash(8, 6, 9, 7);
        t = find(key);
        h[t] = { key, 8, 5, 10, 8 };*/

        //*************************************************************
        key = get_hash(7, 9, 9, 7); // 绿1
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(5, 7, 7, 5); // 绿1
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(7, 5, 9, 7); // 绿1
        t = find(key);
        h[t] = { key, 6, 8, 8, 6 };

        key = get_hash(5, 7, 7, 9); // 绿1
        t = find(key);
        h[t] = { key, 6, 8, 8, 6 };

        key = get_hash(7, 8, 7, 9); // 绿2
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(7, 5, 7, 6); // 绿2
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(8, 7, 9, 7); // 绿2
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(5, 7, 6, 7); // 绿2
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(6, 8, 7, 8); // 绿3
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };
        key = get_hash(8, 6, 8, 7); // 绿3
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };
        key = get_hash(7, 6, 8, 6); // 绿3
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };
        key = get_hash(6, 7, 6, 8); // 绿3
        t = find(key);
        h[t] = { key, 6, 6, 8, 8 };

        key = get_hash(7, 8, 8, 8); // 绿3
        t = find(key);
        h[t] = { key, 6, 8, 8, 6 };
        key = get_hash(8, 7, 8, 8); // 绿3
        t = find(key);
        h[t] = { key, 6, 8, 8, 6 };
        key = get_hash(6, 6, 7, 6); // 绿3
        t = find(key);
        h[t] = { key, 6, 8, 8, 6 };
        key = get_hash(6, 6, 6, 7); // 绿3
        t = find(key);
        h[t] = { key, 6, 8, 8, 6 };

        key = get_hash(6, 8, 8, 6); // 绿4
        t = find(key);
        h[t] = { key, 5, 5, 6, 6 };

        key = get_hash(6, 6, 8, 8); // 绿4
        t = find(key);
        h[t] = { key, 8, 6, 9, 5 };

        //*************************************************************
        key = get_hash(6, 8, 8, 9); // 紫
        t = find(key);
        h[t] = { key, 5, 7, 6, 7 };
        key = get_hash(6, 9, 8, 8); // 紫
        t = find(key);
        h[t] = { key, 8, 7, 9, 7 };
        key = get_hash(8, 8, 9, 6); // 紫
        t = find(key);
        h[t] = { key, 7, 8, 7, 9 };
        key = get_hash(8, 6, 9, 8); // 紫
        t = find(key);
        h[t] = { key, 7, 5, 7, 6 };

        key = get_hash(6, 5, 8, 6); // 紫
        t = find(key);
        h[t] = { key, 8, 7, 9, 7 };
        key = get_hash(6, 6, 8, 5); // 紫
        t = find(key);
        h[t] = { key, 5, 7, 6, 7 };

        key = get_hash(5, 8, 6, 6); // 紫
        t = find(key);
        h[t] = { key, 7, 5, 7, 6 };
        key = get_hash(5, 6, 6, 8); // 紫
        t = find(key);
        h[t] = { key, 7, 8, 7, 9 };

        //*************************************************************
        key = get_hash(6, 8, 8, 8); // 橙
        t = find(key);
        h[t] = { key, 7, 8, 6, 10 };

        key = get_hash(6, 6, 8, 6); // 橙
        t = find(key);
        h[t] = { key, 7, 6, 8, 4 };

        key = get_hash(8, 6, 8, 8); // 橙
        t = find(key);
        h[t] = { key, 8, 7, 10, 8 };

        key = get_hash(6, 6, 6, 8); // 橙
        t = find(key);
        h[t] = { key, 4, 6, 6, 7 };

        //*************************************************************

        //改1总
        key = get_hash(5, 7, 8, 8); //改1
        t = find(key);
        h[t] = { key, 5, 8, 7, 9 };

        key = get_hash(6, 8, 9, 7);
        t = find(key);
        h[t] = { key, 7, 9, 9, 8 };

        key = get_hash(7, 9, 8, 6);
        t = find(key);
        h[t] = { key, 8, 9, 9, 7 };

        key = get_hash(7, 5, 8, 8);
        t = find(key);
        h[t] = { key, 8, 5, 9, 7 };

        key = get_hash(6, 6, 9, 7);
        t = find(key);
        h[t] = { key, 7, 5, 9, 6 };
        key = get_hash(5, 7, 8, 6);
        t = find(key);
        h[t] = { key, 5, 6, 7, 5 };

        key = get_hash(6, 8, 7, 5);
        t = find(key);
        h[t] = { key, 5, 7, 6, 5 };
        key = get_hash(6, 6, 7, 9);
        t = find(key);
        h[t] = { key, 5, 7, 6, 9 };
    }
};

class white_opening_book {
public:
    vector<Action>g[GRID_SIZE][GRID_SIZE];

    white_opening_book() {

        //枚举

        g[0][0].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[0][1].push_back(Action(7 * GRID_SIZE + 7, 6 * GRID_SIZE + 8));
        g[0][2].push_back(Action(7 * GRID_SIZE + 8, 7 * GRID_SIZE + 6));
        g[0][3].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[0][4].push_back(Action(8 * GRID_SIZE + 7, 8 * GRID_SIZE + 9));
        g[0][5].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[0][6].push_back(Action(8 * GRID_SIZE + 6, 8 * GRID_SIZE + 8));
        g[0][7].push_back(Action(7 * GRID_SIZE + 7, 9 * GRID_SIZE + 7));
        g[0][8].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[0][9].push_back(Action(8 * GRID_SIZE + 7, 8 * GRID_SIZE + 5));
        g[0][10].push_back(Action(8 * GRID_SIZE + 7, 8 * GRID_SIZE + 5));
        g[0][11].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[0][12].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 8));
        g[0][13].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 5));
        g[0][14].push_back(Action(7 * GRID_SIZE + 7, 8 * GRID_SIZE + 6));

        g[1][0].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[1][1].push_back(Action(8 * GRID_SIZE + 7, 7 * GRID_SIZE + 8));
        g[1][2].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[1][3].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[1][4].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[1][5].push_back(Action(8 * GRID_SIZE + 7, 8 * GRID_SIZE + 9));
        g[1][6].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[1][7].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[1][8].push_back(Action(9 * GRID_SIZE + 7, 9 * GRID_SIZE + 9));
        g[1][9].push_back(Action(8 * GRID_SIZE + 7, 8 * GRID_SIZE + 9));
        g[1][10].push_back(Action(8 * GRID_SIZE + 6, 8 * GRID_SIZE + 8));
        g[1][11].push_back(Action(8 * GRID_SIZE + 7, 7 * GRID_SIZE + 6));
        g[1][12].push_back(Action(8 * GRID_SIZE + 6, 6 * GRID_SIZE + 6));
        g[1][13].push_back(Action(7 * GRID_SIZE + 6, 8 * GRID_SIZE + 7));
        g[1][14].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));

        g[2][0].push_back(Action(8 * GRID_SIZE + 8, 6 * GRID_SIZE + 8));
        g[2][1].push_back(Action(8 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        g[2][2].push_back(Action(7 * GRID_SIZE + 5, 6 * GRID_SIZE + 6));
        g[2][3].push_back(Action(6 * GRID_SIZE + 5, 5 * GRID_SIZE + 6));
        g[2][4].push_back(Action(6 * GRID_SIZE + 6, 5 * GRID_SIZE + 7));
        g[2][5].push_back(Action(6 * GRID_SIZE + 7, 7 * GRID_SIZE + 7));
        g[2][6].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 7));
        g[2][7].push_back(Action(5 * GRID_SIZE + 5, 6 * GRID_SIZE + 6));
        g[2][8].push_back(Action(5 * GRID_SIZE + 6, 6 * GRID_SIZE + 7));
        g[2][9].push_back(Action(6 * GRID_SIZE + 7, 5 * GRID_SIZE + 6));
        g[2][10].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 8));
        g[2][11].push_back(Action(6 * GRID_SIZE + 8, 5 * GRID_SIZE + 7));
        g[2][12].push_back(Action(5 * GRID_SIZE + 9, 7 * GRID_SIZE + 9));
        g[2][13].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 8));
        g[2][14].push_back(Action(7 * GRID_SIZE + 9, 7 * GRID_SIZE + 7));
        // ok
        g[3][0].push_back(Action(7 * GRID_SIZE + 7, 9 * GRID_SIZE + 7));
        g[3][1].push_back(Action(7 * GRID_SIZE + 8, 9 * GRID_SIZE + 8));
        g[3][2].push_back(Action(5 * GRID_SIZE + 6, 6 * GRID_SIZE + 5));
        g[3][3].push_back(Action(7 * GRID_SIZE + 5, 6 * GRID_SIZE + 6));
        g[3][4].push_back(Action(8 * GRID_SIZE + 5, 7 * GRID_SIZE + 4));
        g[3][5].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 6));
        g[3][6].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 6));
        g[3][7].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[3][8].push_back(Action(5 * GRID_SIZE + 6, 6 * GRID_SIZE + 7));
        g[3][9].push_back(Action(5 * GRID_SIZE + 8, 6 * GRID_SIZE + 9));
        g[3][10].push_back(Action(7 * GRID_SIZE + 9, 7 * GRID_SIZE + 8));
        g[3][11].push_back(Action(7 * GRID_SIZE + 9, 6 * GRID_SIZE + 8));
        g[3][12].push_back(Action(5 * GRID_SIZE + 8, 6 * GRID_SIZE + 9));
        g[3][13].push_back(Action(8 * GRID_SIZE + 6, 8 * GRID_SIZE + 8));
        g[3][14].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 5));

        g[4][0].push_back(Action(8 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));
        g[4][1].push_back(Action(7 * GRID_SIZE + 7, 8 * GRID_SIZE + 8));
        g[4][2].push_back(Action(7 * GRID_SIZE + 5, 6 * GRID_SIZE + 6));
        g[4][3].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));
        g[4][4].push_back(Action(6 * GRID_SIZE + 5, 5 * GRID_SIZE + 6));
        g[4][5].push_back(Action(4 * GRID_SIZE + 7, 7 * GRID_SIZE + 6));
        g[4][6].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 5));
        g[4][7].push_back(Action(7 * GRID_SIZE + 8, 7 * GRID_SIZE + 6));
        g[4][8].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 9));
        g[4][9].push_back(Action(5 * GRID_SIZE + 9, 7 * GRID_SIZE + 8));
        g[4][10].push_back(Action(6 * GRID_SIZE + 9, 5 * GRID_SIZE + 8));
        g[4][11].push_back(Action(6 * GRID_SIZE + 7, 5 * GRID_SIZE + 7));
        g[4][12].push_back(Action(6 * GRID_SIZE + 8, 7 * GRID_SIZE + 9));
        g[4][13].push_back(Action(7 * GRID_SIZE + 7, 8 * GRID_SIZE + 6));
        g[4][14].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 8));

        g[5][0].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[5][1].push_back(Action(8 * GRID_SIZE + 9, 6 * GRID_SIZE + 9));
        g[5][2].push_back(Action(5 * GRID_SIZE + 6, 7 * GRID_SIZE + 6));
        g[5][3].push_back(Action(7 * GRID_SIZE + 5, 6 * GRID_SIZE + 6));
        g[5][4].push_back(Action(5 * GRID_SIZE + 5, 6 * GRID_SIZE + 7));
        g[5][5].push_back(Action(5 * GRID_SIZE + 7, 8 * GRID_SIZE + 6));
        g[5][6].push_back(Action(5 * GRID_SIZE + 5, 6 * GRID_SIZE + 7));
        g[5][7].push_back(Action(5 * GRID_SIZE + 8, 6 * GRID_SIZE + 6));
        g[5][8].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 9));
        g[5][9].push_back(Action(5 * GRID_SIZE + 7, 8 * GRID_SIZE + 8));
        g[5][10].push_back(Action(5 * GRID_SIZE + 9, 6 * GRID_SIZE + 7));
        g[5][11].push_back(Action(7 * GRID_SIZE + 9, 6 * GRID_SIZE + 8));
        g[5][12].push_back(Action(7 * GRID_SIZE + 8, 6 * GRID_SIZE + 8));
        g[5][13].push_back(Action(8 * GRID_SIZE + 5, 6 * GRID_SIZE + 5));
        g[5][14].push_back(Action(7 * GRID_SIZE + 7, 5 * GRID_SIZE + 7));

        g[6][0].push_back(Action(7 * GRID_SIZE + 7, 9 * GRID_SIZE + 7));
        g[6][1].push_back(Action(5 * GRID_SIZE + 8, 7 * GRID_SIZE + 8));
        g[6][2].push_back(Action(7 * GRID_SIZE + 6, 8 * GRID_SIZE + 5));
        g[6][3].push_back(Action(6 * GRID_SIZE + 6, 7 * GRID_SIZE + 5));
        g[6][4].push_back(Action(5 * GRID_SIZE + 6, 7 * GRID_SIZE + 5));
        g[6][5].push_back(Action(5 * GRID_SIZE + 7, 7 * GRID_SIZE + 6));
        g[6][6].push_back(Action(5 * GRID_SIZE + 6, 7 * GRID_SIZE + 5));
        g[6][7].push_back(Action(7 * GRID_SIZE + 6, 8 * GRID_SIZE + 8));
        g[6][8].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 9));
        g[6][9].push_back(Action(5 * GRID_SIZE + 7, 7 * GRID_SIZE + 8));
        g[6][10].push_back(Action(5 * GRID_SIZE + 8, 7 * GRID_SIZE + 9));
        g[6][11].push_back(Action(6 * GRID_SIZE + 8, 7 * GRID_SIZE + 9));
        g[6][12].push_back(Action(5 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));
        g[6][13].push_back(Action(6 * GRID_SIZE + 6, 8 * GRID_SIZE + 6));
        g[6][14].push_back(Action(8 * GRID_SIZE + 5, 8 * GRID_SIZE + 7));

        g[7][0].push_back(Action(7 * GRID_SIZE + 8, 9 * GRID_SIZE + 8));
        g[7][1].push_back(Action(6 * GRID_SIZE + 8, 8 * GRID_SIZE + 8));
        g[7][2].push_back(Action(8 * GRID_SIZE + 6, 9 * GRID_SIZE + 5));
        g[7][3].push_back(Action(6 * GRID_SIZE + 6, 8 * GRID_SIZE + 6));
        g[7][4].push_back(Action(6 * GRID_SIZE + 5, 8 * GRID_SIZE + 6));
        g[7][5].push_back(Action(6 * GRID_SIZE + 7, 8 * GRID_SIZE + 6));
        g[7][6].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 7));//修改
        //改2
        //g[7][6].push_back(Action(6 * GRID_SIZE + 6, 8 * GRID_SIZE + 7));
        //g[7][7].push_back(Action(7 * GRID_SIZE + 8, 8 * GRID_SIZE + 6));
        //g[7][7].push_back(Action(7 * GRID_SIZE + 8, 6 * GRID_SIZE + 6));
        //g[7][7].push_back(Action(7 * GRID_SIZE + 6, 6 * GRID_SIZE + 8));

        //g[7][7].push_back(Action(5 * GRID_SIZE + 7, 8 * GRID_SIZE + 8));//修改+
        g[7][7].push_back(Action(8 * GRID_SIZE + 6, 8 * GRID_SIZE + 8));//修改+
        //g[7][7].push_back(Action(8 * GRID_SIZE + 6, 9 * GRID_SIZE + 8));//修改//封印
        /*g[7][8].push_back(Action(6 * GRID_SIZE + 8, 8 * GRID_SIZE + 7));
        g[7][8].push_back(Action(8 * GRID_SIZE + 7, 9 * GRID_SIZE + 9));*/
        g[7][8].push_back(Action(6 * GRID_SIZE + 9, 5 * GRID_SIZE + 7));//修改
        g[7][8].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 7));
        g[7][9].push_back(Action(7 * GRID_SIZE + 8, 9 * GRID_SIZE + 8));
        g[7][9].push_back(Action(7 * GRID_SIZE + 8, 9 * GRID_SIZE + 8));
        g[7][10].push_back(Action(6 * GRID_SIZE + 9, 8 * GRID_SIZE + 8));
        g[7][10].push_back(Action(6 * GRID_SIZE + 7, 8 * GRID_SIZE + 7));
        g[7][11].push_back(Action(5 * GRID_SIZE + 7, 7 * GRID_SIZE + 7));
        g[7][12].push_back(Action(6 * GRID_SIZE + 7, 7 * GRID_SIZE + 7));
        g[7][13].push_back(Action(6 * GRID_SIZE + 6, 8 * GRID_SIZE + 6));
        g[7][14].push_back(Action(5 * GRID_SIZE + 7, 7 * GRID_SIZE + 7));

        g[8][0].push_back(Action(5 * GRID_SIZE + 7, 7 * GRID_SIZE + 7));
        g[8][1].push_back(Action(6 * GRID_SIZE + 8, 8 * GRID_SIZE + 8));
        g[8][2].push_back(Action(6 * GRID_SIZE + 5, 7 * GRID_SIZE + 6));
        g[8][3].push_back(Action(7 * GRID_SIZE + 5, 8 * GRID_SIZE + 6));
        g[8][4].push_back(Action(7 * GRID_SIZE + 5, 9 * GRID_SIZE + 6));
        g[8][5].push_back(Action(7 * GRID_SIZE + 6, 9 * GRID_SIZE + 7));
        g[8][6].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 7));
        g[8][7].push_back(Action(7 * GRID_SIZE + 6, 8 * GRID_SIZE + 8));
        g[8][8].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 7));
        g[8][9].push_back(Action(7 * GRID_SIZE + 8, 9 * GRID_SIZE + 7));
        g[8][10].push_back(Action(7 * GRID_SIZE + 9, 9 * GRID_SIZE + 8));
        g[8][11].push_back(Action(6 * GRID_SIZE + 9, 7 * GRID_SIZE + 8));
        g[8][12].push_back(Action(6 * GRID_SIZE + 9, 7 * GRID_SIZE + 8));
        g[8][13].push_back(Action(7 * GRID_SIZE + 6, 9 * GRID_SIZE + 6));
        g[8][14].push_back(Action(7 * GRID_SIZE + 5, 7 * GRID_SIZE + 7));

        g[9][0].push_back(Action(6 * GRID_SIZE + 6, 6 * GRID_SIZE + 8));
        g[9][1].push_back(Action(6 * GRID_SIZE + 9, 7 * GRID_SIZE + 8));
        g[9][2].push_back(Action(7 * GRID_SIZE + 6, 9 * GRID_SIZE + 6));
        g[9][3].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 6));
        g[9][4].push_back(Action(8 * GRID_SIZE + 5, 10 * GRID_SIZE + 6));
        g[9][5].push_back(Action(7 * GRID_SIZE + 5, 9 * GRID_SIZE + 6));
        g[9][6].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 7));
        g[9][7].push_back(Action(8 * GRID_SIZE + 6, 7 * GRID_SIZE + 8));
        g[9][8].push_back(Action(7 * GRID_SIZE + 9, 8 * GRID_SIZE + 7));
        g[9][9].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 7));
        g[9][10].push_back(Action(8 * GRID_SIZE + 9, 10 * GRID_SIZE + 8));
        g[9][11].push_back(Action(7 * GRID_SIZE + 9, 8 * GRID_SIZE + 8));
        g[9][12].push_back(Action(9 * GRID_SIZE + 7, 10 * GRID_SIZE + 8));
        g[9][13].push_back(Action(5 * GRID_SIZE + 6, 7 * GRID_SIZE + 6));
        g[9][14].push_back(Action(5 * GRID_SIZE + 7, 7 * GRID_SIZE + 7));

        g[10][0].push_back(Action(6 * GRID_SIZE + 7, 7 * GRID_SIZE + 8));
        g[10][1].push_back(Action(6 * GRID_SIZE + 8, 7 * GRID_SIZE + 7));
        g[10][2].push_back(Action(7 * GRID_SIZE + 5, 8 * GRID_SIZE + 6));
        g[10][3].push_back(Action(8 * GRID_SIZE + 7, 9 * GRID_SIZE + 7));
        g[10][4].push_back(Action(7 * GRID_SIZE + 4, 7 * GRID_SIZE + 6));
        g[10][5].push_back(Action(7 * GRID_SIZE + 6, 10 * GRID_SIZE + 7));
        g[10][6].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 7));
        g[10][7].push_back(Action(8 * GRID_SIZE + 6, 9 * GRID_SIZE + 8));
        g[10][8].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 7));
        g[10][9].push_back(Action(7 * GRID_SIZE + 8, 10 * GRID_SIZE + 7));
        g[10][10].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 8));
        g[10][11].push_back(Action(7 * GRID_SIZE + 8, 8 * GRID_SIZE + 7));
        g[10][12].push_back(Action(7 * GRID_SIZE + 9, 8 * GRID_SIZE + 8));
        g[10][13].push_back(Action(6 * GRID_SIZE + 6, 7 * GRID_SIZE + 7));
        g[10][14].push_back(Action(6 * GRID_SIZE + 7, 7 * GRID_SIZE + 6));


        g[11][0].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[11][1].push_back(Action(5 * GRID_SIZE + 8, 7 * GRID_SIZE + 8));
        g[11][2].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 6));
        g[11][3].push_back(Action(7 * GRID_SIZE + 5, 8 * GRID_SIZE + 6));
        g[11][4].push_back(Action(7 * GRID_SIZE + 5, 7 * GRID_SIZE + 6));
        g[11][5].push_back(Action(8 * GRID_SIZE + 6, 9 * GRID_SIZE + 7));
        g[11][6].push_back(Action(8 * GRID_SIZE + 6, 9 * GRID_SIZE + 7));
        g[11][7].push_back(Action(8 * GRID_SIZE + 7, 9 * GRID_SIZE + 6));
        g[11][8].push_back(Action(8 * GRID_SIZE + 8, 9 * GRID_SIZE + 7));
        g[11][9].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 8));
        g[11][10].push_back(Action(7 * GRID_SIZE + 8, 8 * GRID_SIZE + 7));
        g[11][11].push_back(Action(8 * GRID_SIZE + 8, 9 * GRID_SIZE + 7));
        g[11][12].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 8));
        g[11][13].push_back(Action(7 * GRID_SIZE + 6, 9 * GRID_SIZE + 6));
        g[11][14].push_back(Action(7 * GRID_SIZE + 5, 7 * GRID_SIZE + 7));

        g[12][0].push_back(Action(8 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));
        g[12][1].push_back(Action(8 * GRID_SIZE + 8, 6 * GRID_SIZE + 8));
        g[12][2].push_back(Action(9 * GRID_SIZE + 7, 8 * GRID_SIZE + 6));
        g[12][3].push_back(Action(8 * GRID_SIZE + 5, 9 * GRID_SIZE + 6));
        g[12][4].push_back(Action(8 * GRID_SIZE + 6, 9 * GRID_SIZE + 7));
        g[12][5].push_back(Action(8 * GRID_SIZE + 5, 8 * GRID_SIZE + 7));
        g[12][6].push_back(Action(8 * GRID_SIZE + 7, 9 * GRID_SIZE + 8));
        g[12][7].push_back(Action(9 * GRID_SIZE + 5, 8 * GRID_SIZE + 6));
        g[12][8].push_back(Action(9 * GRID_SIZE + 6, 8 * GRID_SIZE + 7));
        g[12][9].push_back(Action(8 * GRID_SIZE + 7, 8 * GRID_SIZE + 9));
        g[12][10].push_back(Action(8 * GRID_SIZE + 8, 9 * GRID_SIZE + 7));
        g[12][11].push_back(Action(8 * GRID_SIZE + 9, 9 * GRID_SIZE + 7));
        g[12][12].push_back(Action(9 * GRID_SIZE + 9, 7 * GRID_SIZE + 9));
        g[12][13].push_back(Action(7 * GRID_SIZE + 6, 6 * GRID_SIZE + 6));
        g[12][14].push_back(Action(7 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));

        g[13][0].push_back(Action(7 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));
        g[13][1].push_back(Action(6 * GRID_SIZE + 8, 6 * GRID_SIZE + 7));
        g[13][2].push_back(Action(7 * GRID_SIZE + 7, 6 * GRID_SIZE + 7));
        g[13][3].push_back(Action(6 * GRID_SIZE + 8, 6 * GRID_SIZE + 7));
        g[13][4].push_back(Action(7 * GRID_SIZE + 7, 6 * GRID_SIZE + 8));
        g[13][5].push_back(Action(6 * GRID_SIZE + 9, 6 * GRID_SIZE + 7));
        g[13][6].push_back(Action(6 * GRID_SIZE + 5, 6 * GRID_SIZE + 6));
        g[13][7].push_back(Action(6 * GRID_SIZE + 7, 6 * GRID_SIZE + 8));
        g[13][8].push_back(Action(5 * GRID_SIZE + 6, 6 * GRID_SIZE + 5));
        g[13][9].push_back(Action(5 * GRID_SIZE + 6, 6 * GRID_SIZE + 7));
        g[13][10].push_back(Action(6 * GRID_SIZE + 8, 6 * GRID_SIZE + 6));
        g[13][11].push_back(Action(6 * GRID_SIZE + 6, 6 * GRID_SIZE + 7));
        g[13][12].push_back(Action(6 * GRID_SIZE + 6, 5 * GRID_SIZE + 6));
        g[13][13].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 8));
        g[13][14].push_back(Action(7 * GRID_SIZE + 7, 9 * GRID_SIZE + 7));

        g[14][0].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 5));
        g[14][1].push_back(Action(7 * GRID_SIZE + 7, 5 * GRID_SIZE + 5));
        g[14][2].push_back(Action(7 * GRID_SIZE + 8, 7 * GRID_SIZE + 6));
        g[14][3].push_back(Action(7 * GRID_SIZE + 7, 5 * GRID_SIZE + 7));
        g[14][4].push_back(Action(6 * GRID_SIZE + 6, 6 * GRID_SIZE + 8));
        g[14][5].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 9));
        g[14][6].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[14][7].push_back(Action(7 * GRID_SIZE + 8, 7 * GRID_SIZE + 6));
        g[14][8].push_back(Action(7 * GRID_SIZE + 7, 5 * GRID_SIZE + 7));
        g[14][9].push_back(Action(7 * GRID_SIZE + 7, 5 * GRID_SIZE + 7));
        g[14][10].push_back(Action(7 * GRID_SIZE + 8, 7 * GRID_SIZE + 6));
        g[14][11].push_back(Action(7 * GRID_SIZE + 5, 7 * GRID_SIZE + 7));
        g[14][12].push_back(Action(7 * GRID_SIZE + 6, 7 * GRID_SIZE + 8));
        g[14][13].push_back(Action(7 * GRID_SIZE + 7, 7 * GRID_SIZE + 9));
        g[14][14].push_back(Action(7 * GRID_SIZE + 7, 5 * GRID_SIZE + 7));

    }

    Action get_fallen(int x, int y) {
        srand((unsigned)time(NULL));
        int type = rand() % g[x][y].size();
        return g[x][y][type];
    }

};

class during_opening_book {
public:
    int book_map[GRID_SIZE][GRID_SIZE];
    map<unsigned long long, pair<int, int>> book_mp;
    //int book_color;

    unsigned long long get_map_hash()
    {
        unsigned long long temp = 0;
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                temp ^= hash_table[i][j][book_map[i][j] + 1];
            }
        }
        return temp;
    }

    during_opening_book() {
        memset(book_map, 0, sizeof book_map);
        //添加
        book_map[5][7] = book_map[6][9] = 1;
        book_map[4][6] = book_map[5][9] = book_map[7][8] = -1;
        //cout << get_map_hash() << endl;
        book_mp.insert(make_pair(get_map_hash(), make_pair(6 * GRID_SIZE + 10, 7 * GRID_SIZE + 10)));

        memset(book_map, 0, sizeof book_map);
        book_map[5][7] = book_map[6][9] = book_map[6][10] = book_map[7][10] = 1;
        book_map[4][6] = book_map[5][9] = book_map[7][8] = book_map[5][8] = book_map[6][7] = -1;
        book_mp.insert(make_pair(get_map_hash(), make_pair(6 * GRID_SIZE + 11, 6 * GRID_SIZE + 12)));

        memset(book_map, 0, sizeof book_map);



    }

    pair<int, int> get_step(unsigned long long v) {
        if (book_mp.find(v) != book_mp.end())
        {
            return book_mp[v];
        }
        else {
            return { -1,-1 };
        }
    }
};

pair<int, int> main_init(int color, int board_map[GRID_SIZE][GRID_SIZE], int remain_time) //ai方的落子颜色
{
    limit_time = remain_time;
    //cout << 8888 << endl;
    abtss_flag = 0;
    tss_flag = 0;
    srand(time(0)); // 用于生成随机数种子
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis(0, ~0ULL); // 获得64位随机数
    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                hash_table[i][j][k] = dis(gen);
            }
        }
    }

    Board board = Board();
    opening_hash open_idea = opening_hash();
    board.state_hash = board.get_hash();


    int x0, y0, x1, y1;
    int last_action_1 = -1, last_action_2 = -1; // 记录最后两步
    int first_action_x = -1;
    int first_action_y = -1;
    bool get_first_action = false;

    // cin >> turnID; // 读入回合数

    bool firstself = false; // 这回合是否要下这局的第一个棋子

    Game* main_game = new Game(board);   // 创建一场比赛
    main_game->cur_player = OPPO_PLAYER; // 不在乎执白还是执黑，己方的颜色为1

    int cnt_self = 0;
    int x_b, y_b;
    int cnt_oppo = 0;
    int oppo_x1, oppo_y1, oppo_x2, oppo_y2;

    /*for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            cout << board_map[i][j] << " ";
        }
        cout << endl;
    }*/
    //ui的board_map转化为ai中board里面的map
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (board_map[i][j] == 0) continue;

            if ((board_map[i][j] == 1 && color == 1) || (board_map[i][j] == 2 && color == -1))//1是白子 且是己方(color为ai颜色 即ai白) color=1：ai为白
            {
                main_game->cur_player = SELF_PLAYER;
                main_game->make_fallen(board, j * GRID_SIZE + i);
                cnt_self++;
                if (cnt_self == 1) {
                    x_b = j;
                    y_b = i;
                }
            }
            else
            {
                main_game->cur_player = OPPO_PLAYER;
                main_game->make_fallen(board, j * GRID_SIZE + i);
                cnt_oppo++;
                if (cnt_oppo == 1) {
                    oppo_x1 = j;
                    oppo_y1 = i;
                }
                else if (cnt_oppo == 2) {
                    oppo_x2 = j;
                    oppo_y2 = i;
                }
            }
        }
    }
    //if (cnt_self == 0 && cnt_oppo == 1) {
    //    //cout << 11 << endl;
    //    //调用白棋开局库（枚举全局情况）
    //    white_opening_book op = white_opening_book();
    //    Action step = op.get_fallen(oppo_x1, oppo_y1);
    //    printf("%c %d %c %d\n", step.x1 + 'A', step.y1 + 1, step.x2 + 'A', step.y2 + 1);
    //    return { step.y1 + step.x1 * GRID_SIZE,step.y2 + step.x2 * GRID_SIZE };
    //}
    //else if (cnt_self == 1 && cnt_oppo == 2) {
    //    int deta_x = x_b - 9;
    //    int deta_y = y_b - 9;
    //    //cout << 1 << endl;
    //   /* int oppo_x1 = last_action_1 / GRID_SIZE;
    //    int oppo_y1 = last_action_1 % GRID_SIZE;
    //    int oppo_x2 = last_action_2 / GRID_SIZE;
    //    int oppo_y2 = last_action_2 % GRID_SIZE;*/
    //    // cout << "----- " << oppo_x1 << " " << oppo_y1 << " " << oppo_x2 << " " << oppo_y2 << endl;

    //    if (oppo_x1 > oppo_x2)
    //    {
    //        swap(oppo_x1, oppo_x2);
    //        swap(oppo_y1, oppo_y2);
    //    }
    //    else if (oppo_x1 == oppo_x2 && oppo_y1 > oppo_y2)
    //        swap(oppo_y1, oppo_y2);

    //    int key = open_idea.get_hash(oppo_x1 - deta_x - 2, oppo_y1 - deta_y - 2, oppo_x2 - deta_x - 2, oppo_y2 - deta_y - 2);
    //    int idx = open_idea.find(key);
    //    if (open_idea.h[idx].h != 0x3f3f3f3f)
    //    {
    //        int x1_w = open_idea.h[idx].x1 + deta_x + 2;
    //        int y1_w = open_idea.h[idx].y1 + deta_y + 2;
    //        int x2_w = open_idea.h[idx].x2 + deta_x + 2;
    //        int y2_w = open_idea.h[idx].y2 + deta_y + 2;
    //        printf("%c %d %c %d\n", x1_w + 'A', y1_w + 1, x2_w + 'A', y2_w + 1);
    //        return { y1_w + x1_w * GRID_SIZE,y2_w + x2_w * GRID_SIZE };
    //    }
    //}



    /************************************************************************************/
    /***在下面填充你的代码，决策结果（本方将落子的位置）存入startX、startY、resultX、resultY中*****/
    // 下面仅为随机策略的示例代码，且效率低，可删除
    board.get_valid();

    time_t t1 = clock();

    during_opening_book dob = during_opening_book();

    AIplayer* AI = new AIplayer(board, main_game, last_action_1, last_action_2); // 创建AI
    pair<int, int> pair_actions = AI->get_fallenmove();                          // 这个应该返回两个结果(第一次下的和第二次下的）
    int action_1 = pair_actions.first, action_2 = pair_actions.second;
    printf("%c %d %c %d\n", action_1 / GRID_SIZE + 'A', action_1 % GRID_SIZE + 1, action_2 / GRID_SIZE + 'A', action_2 % GRID_SIZE + 1);
    //cout << action_1 / GRID_SIZE << " " << action_1 % GRID_SIZE << " " << action_2 / GRID_SIZE << " " << action_2 % GRID_SIZE << endl;
    return { action_1,action_2 };
    /*
    if (turnID == 57) {
        std::cout << "***************************" << endl;
        set<int>* end_action = main_game->cur_board->get_legal_actions();
        for (auto t = end_action->begin(); t != end_action->end(); t++) {
            std::cout << (*t) / GRID_SIZE << " " << (*t) % GRID_SIZE << endl;
        }
    }


    std::cout << "***************************" << endl;
    std::cout << action_1 << " " << action_2 << endl;
    std::cout << "***************************" << endl;
    */
    /*
    if (action_1 == 0 && action_2 == 0) {
        set<int>* end_action = ab_board.get_legal_actions();
        set<int>::iterator it;
        int cnt = 0;
        for (it = end_action->begin(); it != end_action->end(); it++) {
            if (cnt == 0) action_1 = *it;
            else if (cnt == 1) action_2 = *it;
            else break;
            cnt++;
        }
        //std::cout << action_1 / GRID_SIZE << " " << action_1 % GRID_SIZE << endl;
        //std::cout << action_2 / GRID_SIZE << " " << action_2 % GRID_SIZE << endl;
    }*/
    int cnt = 0;
    if (action_1 == 0 && action_2 == 0)
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                if (board.map[i][j] == 0)
                {
                    cnt++;
                    if (cnt == 1)
                    {
                        action_1 = i * GRID_SIZE + j;
                    }
                    else if (cnt == 2)
                    {
                        action_2 = i * GRID_SIZE + j;
                        break;
                    }
                }
            }
        }
    }

    int startX = action_1 / GRID_SIZE, startY = action_1 % GRID_SIZE, resultX = action_2 / GRID_SIZE, resultY = action_2 % GRID_SIZE;

    /****在上方填充你的代码，决策结果（本方将落子的位置）存入startX、startY、resultX、resultY中****/
    /************************************************************************************/
    cout << 0000 << endl;
    // 决策结束，向平台输出决策结果
    std::cout << startX << ' ' << startY << ' ' << resultX << ' ' << resultY << endl;

    clock_t t2 = clock();


    for (int i = 0; i < AI->root->pp.size(); i++)
    {
        printf("(%d,%d)(%d,%d) ", AI->root->pp[i].x1, AI->root->pp[i].y1, AI->root->pp[i].x2, AI->root->pp[i].y2);
    }
    if (abtss_flag) cout << "s";
    else cout << "u";
    if (tss_flag)
    {
        cout << "s" << t2 - t1 << " " << sp << " ab:" << isfind << "dep" << nowdepth << endl;
    }
    else
    {
        cout << "u" << t2 - t1 << " " << sp << " ab:" << isfind << "dep" << nowdepth << endl;
    }

    /* if (tss_flag)
     {
         cout << "bb" << (t2 - t1) / 10 << endl;
     }
     else
     {
         cout << "uu" << (t2 - t1) / 10 << endl;
     }*/

     //cout << "find:" << yes << endl;
     //cout << "nofind" << no << endl;

   // return 0;
}