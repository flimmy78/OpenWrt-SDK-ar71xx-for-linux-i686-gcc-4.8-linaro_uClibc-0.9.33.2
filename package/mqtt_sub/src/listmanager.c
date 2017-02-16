#include "listmanager.h"

void ListList(linearlist* list) /* 打印线性顺序表 */
{
    int i;
//    printf("当前线性表的状态:\n");
    printf("the current list  status:\n");
    if(list->iAllNum == 0) /*顺序表为空*/
    {
//        printf("当前顺序表为空");
        printf("the list is empty!");
    }
    else
    {
        for(i = 0; i < (list->iAllNum); i++) /*循环遍历顺序表*/
            printf("taskItem length is [%d]\n",
                   list->taskItem[i].length);
    }
    printf("\n");
}

void Output(linearlist* list) /* 打印说明文档 */
{
    system("cls"); /* 清屏 */
    printf("-             顺序表                -\n"); /* 输入功能菜单 */
    printf("-  a: 追加一个节点 i: 插入一个节点  -\n");
    printf("-  d: 删除一个节点 e: 退出          -\n");
    ListList(list);    /* 打印线性顺序表  */
}

linearlist* CreateList()/* 创建线性顺序表 */
{
    linearlist *list = (linearlist*)malloc(sizeof(linearlist)); /* 分配空间 */
    list->iAllNum = 0; /* 初始化头节点值 */
    return list; /* 返回初始化头节点指针 */
}

//在链表最尾增加一个元素
void AppendNode(linearlist* list, tagTaskItem taskItemTemp)  /* 追加节点 */
{
    if(list->iAllNum < MAXLISTSIZE )  /*顺序表不溢出 */
    {
        list->taskItem[list->iAllNum] = taskItemTemp;/* 初始化节点值 */
        list->iAllNum += 1;/* 顺序表长度加1 */
    }
}

void InsertNode(linearlist* list, tagTaskItem taskItemTemp, int pos) /* 插入节点 */
{
    int j;
    if (pos < 0 || pos > list->iAllNum )
    {
        //        printf("所插入的位置超出顺序表的范围\n");
        printf("the pos of insert out of range!");
    }
    else
    {
        for(j = list->iAllNum; j >= pos; j--)  /*逆向遍历顺序表*/
            list->taskItem[j+1] = list->taskItem[j];  /*元素后移*/
        list->taskItem[pos] = taskItemTemp;  /*指向节点赋值*/
        list->iAllNum++; /* 顺序表长度加1 */
    }
}

//下标是从0开始的
void DeleteNode(linearlist* list, int pos)  /* 删除节点 */
{
    int j;
    if((pos < 0) || (pos > list->iAllNum ))  /* 删除位置超出顺序表的范围 */
        printf("the pos of delete out of range!");
    else
    {
        for(j = pos; j < list->iAllNum; j++)  /*遍历顺序表*/
            list->taskItem[j] = list->taskItem[j+1];  /*元素前移*/
        list->iAllNum--; /* 顺序表长度减1 */
    }
}

tagTaskItem* list_begin(linearlist* list)
{
    return &list->taskItem[0];
}

void pop_front(linearlist* list)
{
    DeleteNode(list, 0);
}

//从最后面插入元素
void push_back(linearlist* list, tagTaskItem taskItemTemp)
{
    AppendNode(list, taskItemTemp);
}

//从最前面插入元素
void push_front(linearlist* list, tagTaskItem taskItemTemp)
{
    InsertNode(list, taskItemTemp, 0);
}
