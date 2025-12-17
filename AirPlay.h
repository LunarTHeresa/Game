#include<easyx.h>
#include"list.h"
#include<conio.h>
#include<windows.h>

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


//初始化函数
void init() {
	//加载图片
	loadimage(&img[0], "res//bg01.png", BGWIDTH, BGHEIGHT);
	loadimage(&img[1], "res//hero.png", myairWIDTH, myairHEIGHT);
	loadimage(&img[2],  "res//enemy0.png", enemyWIDTH, enemyHEIGHT);
	loadimage(&img[3], "res//zd11.png", bulletWIDTH, bulletHEIGHT);

	//我方飞机的初始化
	myplane.x = BGWIDTH / 2 - myairWIDTH;
	myplane.y = BGHEIGHT - myairHEIGHT - 10;
	myplane.speed = 6;//我放飞机移动速度
	myplane.next = NULL;

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
	putimage(100, 100, &img[2]);
	//贴子弹
	putimage(200, 200, &img[3]);
	//结束批量绘图
	EndBatchDraw();
}
//玩游戏函数
void play() {
	// 使用GetAsyncKeyState检测按键状态（非阻塞方式）
	if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000) {
		if(myplane.y >= 0)
		myplane.y = myplane.y - myplane.speed;
	}
	if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
		if(myplane.y <BGHEIGHT - myairHEIGHT)
		myplane.y = myplane.y + myplane.speed;
	}
	if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000) {
		if(myplane.x >= 0)
		myplane.x = myplane.x - myplane.speed;
	}
	if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		if(myplane.x <BGWIDTH-myairWIDTH)
		myplane.x = myplane.x + myplane.speed;
	}
}



//打开窗口
void start() {
	initgraph(BGWIDTH, BGHEIGHT);
	init();//初始化
	DrawMap();//加载图片
	while (1) {
		play();
		DrawMap();
		// 添加延迟，控制游戏速度
		Sleep(10); // 延迟10毫秒
	}
	closegraph();
}
