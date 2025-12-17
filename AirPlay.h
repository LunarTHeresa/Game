#include<easyx.h>
#include"list.h"
#include<conio.h>
#include<windows.h>
#include<stdlib.h>
#include<time.h>

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


//定义变量
IMAGE img[4];//图片数组
Node myplane;//我方飞机
LL myBullet;
LL enemy;


//初始化函数
void init() {
	//加载图片
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
}

//把图片贴在图形界面上：贴图函数
void DrawMap() {
	//开始批量绘图
	BeginBatchDraw();
	//贴背景
	putimage(0, 0, &img[0]);
	//贴我方飞机putimage(200, 300, &img[1]);
	putimage(myplane.x, myplane.y, &img[1]);
	//贴敌机
	//putimage(100, 100, &img[2]);
	for (Node* temp = enemy.head; temp != NULL; temp = temp->next) {
		putimage(temp->x, temp->y, &img[2]);
	}
	//贴子弹
	//putimage(200, 200, &img[3]);
	for (Node* temp = myBullet.head; temp != NULL; temp = temp->next) {
		putimage(temp->x, temp->y, &img[3]);
	}
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
//敌机生成函数
void create_enemy() {
	Linklist_insert(&enemy,rand()%(BGWIDTH - enemyWIDTH/2),  0, rand()%5+1);
}

//释放函数
void Delete() {
	//释放越界子弹
	for (Node* temp = myBullet.head; temp != NULL; temp = temp->next) {
		if (temp->y < 0) {
			Node_delete(&myBullet, temp);
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
		// 添加延迟，控制游戏速度
		Sleep(10); // 延迟10毫秒
	}
	
	closegraph();
}
