//记录飞机，子弹数据
#include<stdlib.h>
#include<assert.h>
//链表结点的结构体
typedef struct NODE {
	int x;
	int y;
	int speed;
	//指向下一个结点的指针
	struct NODE* next;
}Node;
//链表的结点
typedef struct LinkList {
	Node* head;//头指针
	Node* end;//尾指针
}LL;
//创建链表结点函数
Node* Node_init(int x, int y, int speed) {
	Node* temp = (Node*)malloc(sizeof(Node));
	assert(temp);
	temp->x = x;
	temp->y = y;
	temp->speed = speed;
	temp->next = NULL;
	return temp;
}
//单链表结点插入函数 尾插法
void Linklist_insert(LL* list, int x, int y, int speed)
{
	if (list == NULL)
	{
		return;
	}
	if (list->head == NULL)
	{
		list->head = list->end = Node_init(x, y, speed);
	}
	else
	{
		list->end->next = Node_init(x, y, speed);
		list->end = list->end->next;//更新尾指针
	}
}


//单链表结点的删除
void Node_delete(LL* list, Node* p)//p指向要删除的结点
{
	if (list == NULL || list->head == NULL)
	{
		return;
	}

	//删除结点的第一种情况：删除的结点是第一个结点
	if (p == list->head)
	{
		list->head = p->next;
		free(p);
		p = NULL;
		return;
	}
	//删除结点的第二种情况：删除的结点是中间结点或最后一个结点
    //定义一个temp指针,找到p结点的前一个结点
	Node* temp = list->head;
	//循环的目的: temp指向p的前一个结点
	for (; temp != NULL; temp = temp->next)
	{
		if (temp->next == p)
		{
			if (p == list->end)//更新
			{
				list->end = temp;
			}
			temp->next = p->next;
			free(p);
			p = NULL;
			return;
		}
	}
}
//整个单链表结点的释放
void LinkList_ALL(LL *list) {
	if (list == NULL || list->head == NULL) {
		return;
	}
	for (Node* temp1 = list->head; temp1 != NULL;) {
		Node* temp2 = temp1;
		temp1 = temp1->next;
		free(temp2);
	}
	list->head = list->end = NULL;
}