#include<easyx.h>
#include"list.h"
#include<conio.h>
#include<windows.h>
#include<stdlib.h>
#include<time.h>
#pragma comment(lib,"winmm.lib")
#include"tools.hpp"
#include<new.h>

#define BGWIDTH 400
#define BGHEIGHT 600
//我方飞机大小
#define myairWIDTH 50
#define myairHEIGHT 70
//敌机大小
#define enemyWIDTH 45
#define enemyHEIGHT 45

//子弹大小
#define bulletWIDTH 11
#define bulletHEIGHT 11


// 定义变量
IMAGE img[4];//图片数组
Node myplane;//我方飞机
LL myBullet;
LL enemy;
int isEnd;
int count;


//初始化函数
void init() {
	// 先确保任何已打开的别名被关闭，防止重复打开导致延迟或错误
	mciSendString("stop bgmusic", NULL, 0, NULL);
	mciSendString("close bgmusic", NULL, 0, NULL);

	// 先启动音乐（提前于加载图片），减少进入游戏的延迟
	// 使用别名并设置循环播放，避免播放延迟和保证循环
	mciSendString("open \"res\\bg1.mp3\" type mpegvideo alias bgmusic", NULL, 0, NULL);
	// 设置为毫秒格式，便于精确控制（可选）
	mciSendString("set bgmusic time format ms", NULL, 0, NULL);
	// 立即从头开始循环播放
	mciSendString("play bgmusic from 0 repeat", NULL, 0, NULL);

	//加载图片（可能耗时，将音乐启动放在前面以减少用户感知延迟)
	loadimage(&img[0], "res//bg01.png", BGWIDTH, BGHEIGHT);
	loadimage(&img[1], "res//hero.png", myairWIDTH, myairHEIGHT);
	loadimage(&img[2],  "res//enemy0.png", enemyWIDTH, enemyHEIGHT);
	loadimage(&img[3], "res//zd11.png", bulletWIDTH, bulletHEIGHT);

	//我方飞机的初始化
	myplane.x = BGWIDTH / 2 - myairWIDTH/2;
	myplane.y = BGHEIGHT - myairHEIGHT - 10;
	myplane.speed = 6;//我放飞机移动速度
	myplane.next = NULL;
	//子弹链表的初始化
	myBullet.end = NULL;
	myBullet.head = NULL;
	//敌方飞机的初始化
	enemy.end = NULL;
	enemy.head = NULL;

	//随机产生敌机
	srand((unsigned int)time(NULL));
	isEnd = 0;
	count = 0;
}

//把图片贴在图形界面上：贴图函数
void DrawMap() {
	//开始批量绘图
	BeginBatchDraw();
	//贴背景
	putimage(0, 0, &img[0]);
	//贴我方飞机putimage(200, 300, &img[1]);
	//putimage(myplane.x, myplane.y, &img[1]);
	drawImg(myplane.x, myplane.y,&img[1]);
	//贴敌机
	//putimage(100, 100, &img[2]);
	for (Node* temp = enemy.head; temp != NULL; temp = temp->next) {
		//putimage(temp->x, temp->y, &img[2]);
		drawImg(temp->x, temp->y,&img[2]);
	}
	//贴子弹
	//putimage(200, 200, &img[3]);
	for (Node* temp = myBullet.head; temp != NULL; temp = temp->next) {
		//putimage(temp->x, temp->y, &img[3]);
		drawImg(temp->x, temp->y, &img[3]);
	}
	//显示得分
	TCHAR scoreText[30];
	wsprintf(scoreText, TEXT("当前得分为: %d"), count);
	outtextxy(0, 0, scoreText);
	setbkmode(TRANSPARENT);
	settextcolor(BLACK); // 设置文字颜色为黑色
	//结束批量绘图
	EndBatchDraw();
}
//子弹生成函数
void create_myBullet() {
	Linklist_insert(&myBullet, myplane.x + myairWIDTH / 2 - bulletWIDTH / 2, myplane.y, 5);
}
//移动函数
void move() {
	for (Node* temp = myBullet.head; temp != NULL; temp = temp->next) {
		temp->y = temp->y - temp->speed;
	}
	for (Node* temp = enemy.head; temp != NULL; temp = temp->next) {
		temp->y = temp->y + temp->speed;
	}
}
//检测碰撞函数
int Collision(Node* b,Node* e) {
	int just = 1;
	int bx = b->x + bulletWIDTH / 2;
	int by = b->y + bulletHEIGHT / 2;
	int ex = e->x + enemyWIDTH / 2;
	int ey = e->y + enemyHEIGHT / 2;
	if ((bx - ex) >= enemyWIDTH / 2 || (bx - ex) <= (enemyWIDTH / 2)*(-1)) {
		just = 0;
	}
	if ((by - ey) >= enemyHEIGHT / 2 || (by - ey) <= (enemyHEIGHT / 2)*(-1)) {
		just = 0;
	}
	return just;
}
//敌机生成函数
void create_enemy() {
	static int val = 0;
	if( val >= 100){
		Linklist_insert(&enemy, rand() % (BGWIDTH - enemyWIDTH / 2), 0, rand() % 5 + 1);
		val = 0;
	}
	val++;
}

// 通用矩形碰撞检测（以左上角坐标和宽高判断）
int CollisionRect(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
	int a_cx = ax + aw / 2;
	int a_cy = ay + ah / 2;
	int b_cx = bx + bw / 2;
	int b_cy = by + bh / 2;
	if (abs(a_cx - b_cx) > (aw + bw) / 2) return 0;
	if (abs(a_cy - b_cy) > (ah + bh) / 2) return 0;
	return 1;
}

// 释放函数（安全遍历并使用通用碰撞）
void Delete() {
	// 释放越界子弹（安全遍历）
	for (Node* temp = myBullet.head; temp != NULL; ) {
		Node* next = temp->next;
		if (temp->y < 0) {
			Node_delete(&myBullet, temp);
		}
		temp = next;
	}
	// 释放越界敌机（安全遍历）
	for (Node* temp = enemy.head; temp != NULL; ) {
		Node* next = temp->next;
		if (temp->y > BGHEIGHT) {
			Node_delete(&enemy, temp);
		}
		temp = next;
	}
	// 子弹打中敌机（双重遍历，删除时使用保存的next）
	for (Node* b = myBullet.head; b != NULL; ) {
		Node* bnext = b->next;
		for (Node* e = enemy.head; e != NULL; ) {
			Node* enext = e->next;
			if (CollisionRect(b->x, b->y, bulletWIDTH, bulletHEIGHT, e->x, e->y, enemyWIDTH, enemyHEIGHT)) {
				Node_delete(&myBullet, b);
				Node_delete(&enemy, e);
				count++;
				// 已删除 b 和 e，跳出内层循环
				break;
			}
			e = enext;
		}
		// 如果 b 已被删除，它不再有效；直接继续到保存的 bnext
		b = bnext;
	}
	// 敌机与我方飞机碰撞游戏结束（使用玩家宽高）
	for (Node* e = enemy.head; e != NULL; e = e->next) {
		if (CollisionRect(myplane.x, myplane.y, myairWIDTH, myairHEIGHT, e->x, e->y, enemyWIDTH, enemyHEIGHT)) {
			isEnd = 1;
			return;
		}
	}
}
//玩游戏函数
// 在play函数中添加静态变量
void play() {
	static int spacePressed = 0;  // 记录空格键上次的状态

	// 飞机移动代码
	if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000) {
		if (myplane.y >= 0)
			myplane.y = myplane.y - myplane.speed;
	}
	if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
		if (myplane.y < BGHEIGHT - myairHEIGHT)
			myplane.y = myplane.y + myplane.speed;
	}
	if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000) {
		if (myplane.x >= 0)
			myplane.x = myplane.x - myplane.speed;
	}
	if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		if (myplane.x < BGWIDTH - myairWIDTH)
			myplane.x = myplane.x + myplane.speed;
	}

	// 子弹生成逻辑
	int currentSpaceState = GetAsyncKeyState(' ') & 0x8000;

	// 如果当前空格键被按下，但上次没有被按下（即刚被按下）
	if (currentSpaceState && !spacePressed) {
		create_myBullet();  // 生成子弹
	}

	// 更新空格键状态
	spacePressed = currentSpaceState;
}

//打开窗口
void start() {
	initgraph(BGWIDTH, BGHEIGHT);
	init();//初始化
	DrawMap();//加载图片
	while (1) {
		play();
		move();
		DrawMap();
		create_enemy();
		Delete();
		if (isEnd) {
			// 可以在这里显示“游戏结束”画面或跳出循环
			LinkList_ALL(&myBullet);
			LinkList_ALL(&enemy);
			// 停止并关闭别名音乐，确保下次可以立即重开播放
			mciSendString("stop bgmusic", NULL, 0, NULL);
			mciSendString("close bgmusic", NULL, 0, NULL);
			//判断游戏结束，以及是否开始下一局
			TCHAR endText[50];
			wsprintf(endText, TEXT("游戏结束! 你的得分是: %d\n是否重新开始游戏"), count);
			int restart = MessageBox(GetForegroundWindow(),endText,"游戏结束", MB_YESNO);
			if (restart == IDYES) {
				//重新开始游戏
				init();
				continue;
			}
			else {
				//退出游戏
				break;
			}
		}
		// 添加延迟，控制游戏速度
		Sleep(10); // 延迟10毫秒
	}
	
	closegraph();
}
