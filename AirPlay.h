#include<easyx.h>
#include"list.h"
#include<conio.h>
#include<windows.h>
#include<stdlib.h>
#include<time.h>
#pragma comment(lib,"winmm.lib")
#include"tools.hpp"
#include<new.h>
#include<unordered_map>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdio>

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

// boss 大小与初始血量
#define bossWIDTH 120
#define bossHEIGHT 90
#define bossINITHP 30
#define bossBULLETSPEED 5

// 定义变量
IMAGE img[6];//图片数组，新增 img[5] 为 boss 背景
Node myplane;//我方飞机
LL myBullet;
LL enemy;
LL bossBullets; // boss 发射的子弹链表
int isEnd;
int count;

// 为普通敌机维护血量映射（键为 Node*）
static std::unordered_map<Node*, int> enemyHP;
static std::unordered_map<Node*, int> enemyMaxHP;

// 我方飞机在 boss 战时的血量（默认为0表示未进入 boss 战）
int myHP = 0;
int myMaxHP = 0;	

// boss 结构体（单个 boss 使用）
typedef struct Boss {
	int x;
	int y;
	int speed;
	int hp;
	int dir; // -1 左，1 右
} Boss;

Boss* bossPtr = NULL;
int nextBossScore = 50; // 下一次出现 boss 的分数阈值

// 玩家昵称（全局）
static std::string playerName = "Player";
// 排行文件名
static const char* RANK_FILE = "rank.txt";

// 辅助：安全删除敌机并清理映射
void EnemyNode_erase(Node* n) {
	if (n == NULL) return;
	if (enemyHP.find(n) != enemyHP.end()) enemyHP.erase(n);
	if (enemyMaxHP.find(n) != enemyMaxHP.end()) enemyMaxHP.erase(n);
	Node_delete(&enemy, n);
}

// 获取玩家昵称：在控制台中输入，然后关闭控制台
// 伪代码（详细计划）：
// 1. 尝试分配新控制台 (AllocConsole)
//    - 如果成功，标记为已分配
//    - 如果失败，尝试附加到父进程控制台 (AttachConsole(ATTACH_PARENT_PROCESS))
// 2. 重定向 stdout/stderr/stdin 到控制台（使用 freopen_s 打开 "CONOUT$"/"CONIN$"）
// 3. 提示玩家输入昵称并使用 fgets 读取（缓冲区长度 64）
// 4. 修剪读取字符串前后空白并去除末尾换行符
// 5. 如果输入为空，则保持默认名 "Player"
// 6. 关闭已打开的 FILE* 并释放控制台（不论是分配还是附加，都调用 FreeConsole）
// 7. 确保昵称长度不超过 60
void GetPlayerName() {
	// 尝试分配控制台；若失败则尝试附加到父进程控制台
	BOOL allocated = FALSE;
	if (AllocConsole()) {
		allocated = TRUE;
	} else {
		// 若已经有控制台则附加父进程的控制台（例如从 IDE 或 cmd 启动时）
		AttachConsole(ATTACH_PARENT_PROCESS);
	}

	FILE* in = NULL;
	FILE* out = NULL;
	// 重定向标准输出/输入到控制台（忽略返回值，但尽量保证能读写）
	freopen_s(&out, "CONOUT$", "w", stdout);
	freopen_s(&in, "CONIN$", "r", stdin);

	// 若无法打开控制台 I/O，则尝试清理并直接返回（保持默认名字）
	if (in == NULL || out == NULL) {
		if (out) { fclose(out); out = NULL; }
		if (in) { fclose(in); in = NULL; }
		// 只有在我们确实分配过控制台时才释放（对附加的控制台调用 FreeConsole 也可接受）
		if (allocated) FreeConsole();
		return;
	}

	// 提示并读取
	printf("请输入昵称(最多60个字符，回车确认): ");
	char buf[64] = {0};
	if (fgets(buf, sizeof(buf), stdin) != NULL) {
		// 去掉末尾换行
		size_t len = strlen(buf);
		if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
		// 转为 std::string 并修剪前后空白
		std::string tmp = buf;
		size_t start = tmp.find_first_not_of(" \t\r\n");
		size_t end = tmp.find_last_not_of(" \t\r\n");
		if (start != std::string::npos && end != std::string::npos && end >= start) {
			playerName = tmp.substr(start, end - start + 1);
		} else {
			// 若只输入空白或空字符串，则保留默认名或使用非空输入
			if (!tmp.empty()) playerName = tmp;
		}
	}

	// 关闭重定向并释放控制台
	if (out) { fclose(out); out = NULL; }
	if (in) { fclose(in); in = NULL; }
	FreeConsole();

	// 最后确保名字合理
	if (playerName.empty()) playerName = "Player";
	if (playerName.size() > 60) playerName = playerName.substr(0, 60);
}

// 保存分数并显示排行榜
// 伪代码（详细计划）：
// 1. 读取现有排行文件，把相同昵称的记录合并为唯一记录并保留最高分（用 unordered_map<string,int> bestScores）
// 2. 将本次得分与已有得分比较，保留更高的分数
// 3. 把合并后的记录转换为 vector<pair<int,string>>，按分数降序排序
// 4. 覆写排行文件（按排序后的顺序写入），确保文件中每个昵称只有一条记录，且为最高分
// 5. 构造并显示前10名排行文本，同时显示玩家这次（即最终保留的）名次
void SaveScoreAndShowRank(int score) {
	// 读取并合并已有记录，保留每个昵称的最高分
	std::unordered_map<std::string, int> bestScores;
	try {
		std::ifstream ifs(RANK_FILE, std::ios::in | std::ios::binary);
		std::string line;
		while (std::getline(ifs, line)) {
			if (line.empty()) continue;
			size_t pos = line.rfind('|');
			if (pos == std::string::npos) continue;
			std::string name = line.substr(0, pos);
			std::string scs = line.substr(pos + 1);
			// 去掉 name 两端空白
			auto trim = [](std::string &s) {
				const char* ws = " \t\r\n";
				size_t a = s.find_first_not_of(ws);
				size_t b = s.find_last_not_of(ws);
				if (a == std::string::npos) { s.clear(); return; }
				s = s.substr(a, b - a + 1);
			};
			trim(name);
			trim(scs);
			if (name.empty() || scs.empty()) continue;
			try {
				int sc = std::stoi(scs);
				auto it = bestScores.find(name);
				if (it == bestScores.end() || sc > it->second) {
					bestScores[name] = sc;
				}
			} catch (...) {
				continue;
			}
		}
		ifs.close();
	} catch (...) {
		// 读取失败则继续，bestScores 可能为空
	}

	// 更新当前玩家的最高分（保留更高者）
	auto itp = bestScores.find(playerName);
	if (itp == bestScores.end() || score > itp->second) {
		bestScores[playerName] = score;
	}

	// 将合并结果排序（按分数降序）
	std::vector<std::pair<int, std::string>> entries;
	entries.reserve(bestScores.size());
	for (const auto& kv : bestScores) {
		entries.emplace_back(kv.second, kv.first); // pair<score, name>
	}
	std::sort(entries.begin(), entries.end(), [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
		if (a.first != b.first) return a.first > b.first;
		return a.second < b.second;
	});

	// 覆写写回文件（保证文件中每个昵称只有一条且为最高分）
	try {
		std::ofstream ofs(RANK_FILE, std::ios::out | std::ios::trunc | std::ios::binary);
		if (ofs) {
			for (const auto& e : entries) {
				ofs << e.second << "|" << e.first << "\n";
			}
			ofs.close();
		}
	} catch (...) {
		// 写失败则忽略
	}

	// 构造排行榜文本（最多前10）
	std::ostringstream oss;
	oss << "排行榜(前10):\n";
	int limit = (int)entries.size() < 10 ? (int)entries.size() : 10;
	for (int i = 0; i < limit; ++i) {
		oss << (i + 1) << ". " << entries[i].second << " - " << entries[i].first << "\n";
	}
	// 查找玩家最终保留得分和名次
	int playerRank = -1;
	int playerBestScore = -1;
	for (size_t i = 0; i < entries.size(); ++i) {
		if (entries[i].second == playerName) {
			playerRank = (int)i + 1;
			playerBestScore = entries[i].first;
			break;
		}
	}
	if (playerRank != -1) {
		oss << "\n你的本次名次: " << playerRank << "（记录分: " << playerBestScore << "）\n";
	}
	std::string msg = oss.str();
	MessageBoxA(GetForegroundWindow(), msg.c_str(), "排行榜", MB_OK | MB_TOPMOST);
}

// 初始化函数
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
	loadimage(&img[2], "res//enemy0.png", enemyWIDTH, enemyHEIGHT);
	loadimage(&img[3], "res//zd11.png", bulletWIDTH, bulletHEIGHT);
	// boss 图片（可替换为合适资源）
	loadimage(&img[4], "res//boss.png", bossWIDTH, bossHEIGHT);
	// boss 专用背景（当 boss 存在时切换到此背景）
	loadimage(&img[5], "res//bg_boss.png", BGWIDTH, BGHEIGHT);

	//我方飞机的初始化
	myplane.x = BGWIDTH/ 2 - myairWIDTH/2;
	myplane.y = BGHEIGHT - myairHEIGHT - 10;
	myplane.speed = 6;//我放飞机移动速度
	myplane.next = NULL;
	//子弹链表的初始化
	myBullet.end = NULL;
	myBullet.head = NULL;
	//敌方飞机的初始化
	enemy.end = NULL;
	enemy.head = NULL;

	// boss 子弹链表初始化
	bossBullets.end = NULL;
	bossBullets.head = NULL;
	// boss 初始化为空
	if (bossPtr) {
		delete bossPtr;
		bossPtr = NULL;
	}
	nextBossScore = 50;

	// 清理敌机血量映射
	enemyHP.clear();
	enemyMaxHP.clear();

	// 初始化我方飞机血量（默认无血条，只有在 boss 出现时才赋值）
	myHP = 0;
	myMaxHP = 0;

	//随机产生敌机
	srand((unsigned int)time(NULL));
	isEnd = 0;
	count = 0;
}

//把图片贴在图形界面上：贴图函数
void DrawMap() {
	//开始批量绘图
	BeginBatchDraw();
	// 选择背景：有 boss 时切换到 boss 背景
	if (bossPtr != NULL) {
		putimage(0, 0, &img[5]);
	} else {
		putimage(0, 0, &img[0]);
	}
	//贴我方飞机putimage(200, 300, &img[1]);
	//putimage(myplane.x, myplane.y, &img[1]);
	drawImg(myplane.x, myplane.y,&img[1]);
	// 如果当前为 boss 战且我方有血条，则绘制我方飞机血条（在飞机上方）
	if (bossPtr != NULL && myMaxHP > 0) {
		int barW = myairWIDTH;
		int barH = 8;
		int barX = myplane.x;
		int barY = myplane.y - 12;
		setfillcolor(BLACK);
		solidrectangle(barX - 1, barY - 1, barX + barW + 1, barY + barH + 1);
		// 我方血条改为绿色
		setfillcolor(GREEN);
		double ratio = (double)myHP / (double)myMaxHP;
		if (ratio < 0) ratio = 0;
		if (ratio > 1) ratio = 1;
		solidrectangle(barX, barY, barX + (int)(barW * ratio), barY + barH);
	}

	//贴敌机（普通敌机不显示血条）
	//putimage(100, 100, &img[2]);
	for (Node* temp = enemy.head; temp != NULL; temp = temp->next) {
		// 仅绘制敌机贴图，不绘制血条
		drawImg(temp->x, temp->y,&img[2]);
	}

	//贴子弹
	//putimage(200, 200, &img[3]);
	for (Node* temp = myBullet.head; temp != NULL; temp = temp->next) {
		//putimage(temp->x, temp->y, &img[3]);
		drawImg(temp->x, temp->y, &img[3]);
	}

	// 绘制 boss 及其子弹和血条
	if (bossPtr != NULL) {
		// 绘制 boss 图片（如果资源不存在，drawImg 可能无效果）
		drawImg(bossPtr->x, bossPtr->y, &img[4]);
		// 绘制血条（在 boss 之上）
		int barW = bossWIDTH;
		int barH = 8;
		int barX = bossPtr->x;
		int barY = bossPtr->y - 12;
		setfillcolor(BLACK);
		solidrectangle(barX - 1, barY - 1, barX + barW + 1, barY + barH + 1);
		setfillcolor(RED);
		// 当前血量比例
		double ratio = (double)bossPtr->hp / (double)bossINITHP;
		if (ratio < 0) ratio = 0;
		solidrectangle(barX, barY, barX + (int)(barW * ratio), barY + barH);
	}
	// boss 子弹画图
	for (Node* temp = bossBullets.head; temp != NULL; temp = temp->next) {
		drawImg(temp->x, temp->y, &img[3]); // 使用相同的子弹图片
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
// 移动 boss 子弹的生成函数（由 boss 调用）
void create_bossBullet(int bx, int by) {
	Linklist_insert(&bossBullets, bx + bossWIDTH / 2 - bulletWIDTH / 2, by + bossHEIGHT, bossBULLETSPEED);
}

//移动函数
void move() {
	for (Node* temp = myBullet.head; temp != NULL; temp = temp->next) {
		temp->y = temp->y - temp->speed;
	}
	for (Node* temp = enemy.head; temp != NULL; temp = temp->next) {
		temp->y = temp->y + temp->speed;
	}
	// boss 移动以及 boss 子弹移动
	if (bossPtr != NULL) {
		// 简单横向移动并在边缘反向
		bossPtr->x += bossPtr->speed * bossPtr->dir;
		if (bossPtr->x <= 0) {
			bossPtr->x = 0;
			bossPtr->dir = 1;
		}
		if (bossPtr->x >= BGWIDTH - bossWIDTH) {
			bossPtr->x = BGWIDTH - bossWIDTH;
			bossPtr->dir = -1;
		}
		// boss 发射子弹：在 move 中按帧计数决定发射间隔
		static int bossFireTick = 0;
		bossFireTick++;
		if (bossFireTick >= 40) { // 每 40 帧发一枚子弹（可调整）
			create_bossBullet(bossPtr->x, bossPtr->y);
			bossFireTick = 0;
		}
	}
	for (Node* temp = bossBullets.head; temp != NULL; temp = temp->next) {
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
	// 如果 boss 存在，则暂停生成普通敌机
	// （保留原行为：boss 战期间不生成新普通敌机）
	if (bossPtr != NULL) {
		return;
	}
	static int val = 0;
	if( val >= 100){
		Linklist_insert(&enemy, rand() % (BGWIDTH - enemyWIDTH / 2), 0, rand() % 5 + 1);
		// 为新插入的敌机设置默认血量（如果没有记录）
		for (Node* t = enemy.head; t != NULL; t = t->next) {
			if (enemyMaxHP.find(t) == enemyMaxHP.end()) {
				int defaultMax = 1;
				// 若 boss 存在时（理论上不会在此处发生，因为上文已 return），设置更高的血量
				if (bossPtr != NULL) defaultMax = bossINITHP / 10 > 0 ? bossINITHP / 10 : 1;
				enemyMaxHP[t] = defaultMax;
				enemyHP[t] = defaultMax;
			}
		}
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

// 创建 boss（当得分达到阈值时）
void create_boss_if_needed() {
	if (count >= nextBossScore && bossPtr == NULL) {
		bossPtr = new Boss();
		bossPtr->x = BGWIDTH / 2 - bossWIDTH / 2;
		bossPtr->y = 20;
		bossPtr->speed = 3;
		bossPtr->hp = bossINITHP;
		bossPtr->dir = (rand() % 2) ? 1 : -1;
		nextBossScore += 50;
		// boss 出现时，清空现有普通敌机，确保 boss 战期间没有普通敌机出现
		LinkList_ALL(&enemy);
		// 清空敌机血量映射
		enemyHP.clear();
		enemyMaxHP.clear();

		// 为我方飞机分配 boss 战血量：boss 初始血量的 1/10（至少为1）
		myMaxHP = bossINITHP / 10;
		if (myMaxHP < 1) myMaxHP = 1;
		myHP = myMaxHP;
	}
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
			EnemyNode_erase(temp);
		}
		temp = next;
	}
	// 释放越界 boss 子弹（安全遍历）
	for (Node* temp = bossBullets.head; temp != NULL; ) {
		Node* next = temp->next;
		if (temp->y > BGHEIGHT) {
			Node_delete(&bossBullets, temp);
		}
		temp = next;
	}

	// 子弹打中普通敌机（双重遍历，删除时使用保存的next）
	for (Node* b = myBullet.head; b != NULL; ) {
		Node* bnext = b->next;
		bool bDeleted = false;
		for (Node* e = enemy.head; e != NULL; ) {
			Node* enext = e->next;
			if (CollisionRect(b->x, b->y, bulletWIDTH, bulletHEIGHT, e->x, e->y, enemyWIDTH, enemyHEIGHT)) {
				// 如果敌机有血量记录，先减少血量
				auto it = enemyHP.find(e);
				if (it != enemyHP.end()) {
					it->second -= 1;
					// 删除子弹
					Node_delete(&myBullet, b);
					bDeleted = true;
					// 若血量耗尽，删除敌机并计分
					if (it->second <= 0) {
						EnemyNode_erase(e);
						count++;
					}
				} else {
					// 原有行为：直接删除敌机与子弹
					Node_delete(&myBullet, b);
					EnemyNode_erase(e);
					count++;
					bDeleted = true;
				}
				// 已处理这一子弹，跳出内层循环
				break;
			}
			e = enext;
		}
		// 检查是否命中 boss
		if (!bDeleted && bossPtr != NULL) {
			if (CollisionRect(b->x, b->y, bulletWIDTH, bulletHEIGHT, bossPtr->x, bossPtr->y, bossWIDTH, bossHEIGHT)) {
				// 子弹命中 boss，扣血并删除子弹
				bossPtr->hp -= 1;
				Node_delete(&myBullet, b);
				bDeleted = true;
				// 如果 boss 血量耗尽，销毁 boss 并清空其所有子弹
				if (bossPtr->hp <= 0) {
					// 清空 boss 子弹链表
					LinkList_ALL(&bossBullets);
					delete bossPtr;
					bossPtr = NULL;
					// boss 被击败，恢复/清空玩家 boss 战血量
					myHP = 0;
					myMaxHP = 0;
				}
			}
		}
		// 如果 b 已被删除，它不再有效；直接继续到保存的 bnext
		b = bnext;
	}

	// 敌机与我方飞机碰撞游戏结束（使用玩家宽高）
	for (Node* e = enemy.head; e != NULL; e = e->next) {
		if (CollisionRect(myplane.x, myplane.y, myairWIDTH, myairHEIGHT, e->x, e->y, enemyWIDTH, enemyHEIGHT)) {
			// 普通敌机与玩家碰撞仍舊直接结束游戏
			isEnd = 1;
			return;
		}
	}
	// boss 子弹碰撞我方飞机
	for (Node* b = bossBullets.head; b != NULL; ) {
		Node* bnext = b->next;
		if (CollisionRect(myplane.x, myplane.y, myairWIDTH, myairHEIGHT, b->x, b->y, bulletWIDTH, bulletHEIGHT)) {
			if (bossPtr != NULL && myMaxHP > 0) {
				// 在 boss 战时，子弹只扣除我方一格血量并删除子弹
				myHP -= 1;
				Node_delete(&bossBullets, b);
				if (myHP <= 0) {
					isEnd = 1;
					return;
				}
			} else {
				// 非 boss 战时仍舊直接死亡
				isEnd = 1;
				return;
			}
		}
		b = bnext;
	}
	// boss 与我方飞机碰撞（如果 boss 存在）
	if (bossPtr != NULL) {
		if (CollisionRect(myplane.x, myplane.y, myairWIDTH, myairHEIGHT, bossPtr->x, bossPtr->y, bossWIDTH, bossHEIGHT)) {
			// 直接碰撞 boss 仍判定为游戏结束（保持原行为）
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
	// 在正式进入图形界面前，要求玩家输入并确认昵称
	while (true) {
		GetPlayerName(); // 在控制台输入昵称（会分配并释放控制台）
		// 构造确认提示，使用 ANSI MessageBox 显示玩家输入的昵称
		std::string msg = "你的昵称是: ";
		msg += playerName;
		msg += "\n\n确认并开始游戏吗？\n(选择 否 将重新输入昵称)";
		int res = MessageBoxA(GetForegroundWindow(), msg.c_str(), "确认昵称", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST);
		if (res == IDYES) break; // 确认则继续进入游戏
		// 否则循环再次输入
	}

	initgraph(BGWIDTH, BGHEIGHT);
	init();//初始化
	DrawMap();//加载图片
	while (1) {
		play();
		move();
		DrawMap();
		create_enemy();
		// boss 出现检测（基于分数）
		create_boss_if_needed();
		Delete();
		if (isEnd) {
			// 保存分数并显示排行榜（在询问是否重开前）
			SaveScoreAndShowRank(count);

			// 可以在这里显示“游戏结束”画面或跳出循环
			LinkList_ALL(&myBullet);
			LinkList_ALL(&enemy);
			LinkList_ALL(&bossBullets);
			if (bossPtr) {
				delete bossPtr;
				bossPtr = NULL;
			}
			// 停止并关闭别名音乐，确保下次可以立即重开播放
			mciSendString("stop bgmusic", NULL, 0, NULL);
			mciSendString("close bgmusic", NULL, 0, NULL);
			//判断游戏结束，以及是否开始下一局
			TCHAR endText[80];
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
