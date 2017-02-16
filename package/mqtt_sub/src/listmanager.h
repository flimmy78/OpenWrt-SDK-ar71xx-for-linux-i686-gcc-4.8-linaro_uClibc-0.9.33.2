#define DOWD_CMD_LENGTH_MAX 1024
#define MAXLISTSIZE    1024
typedef struct    //任务元素的结构体
{
    int  length;
    char data[DOWD_CMD_LENGTH_MAX];
}tagTaskItem;

typedef struct  /* 定义顺序表节点类型 */
{
    tagTaskItem taskItem[MAXLISTSIZE];  /* 顺序表*/
    int iAllNum; /*顺序表元素个数 */
}linearlist;

void ListList(linearlist* list); /* 打印线性顺序表 */
void Output(linearlist* list); /* 打印说明文档 */
linearlist* CreateList();/* 创建线性顺序表 */
linearlist* cmd_list;
tagTaskItem *list_begin(linearlist* list);//删除最前的一个元素
void AppendNode(linearlist* list, tagTaskItem taskItemTemp); /* 追加节点 */
void InsertNode(linearlist* list, tagTaskItem taskItemTemp, int pos); /* 插入节点 */
void DeleteNode(linearlist* list, int pos);  /* 删除节点 */
void pop_front(linearlist* list);//删除最前的一个元素
void push_back(linearlist* list, tagTaskItem taskItemTemp);//从最后面插入元素
void push_front(linearlist* list, tagTaskItem taskItemTemp);//从最前面插入元素

