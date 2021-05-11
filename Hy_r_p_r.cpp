#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <ctime>

using namespace std;

//lamda=0时是HeatS算法，lamda=1时是ProbS算法,(0,1)之间是二者混合算法
double lamda=1;
//L为推荐列表长度
const int L = 50;
//MAXN 标记数组最大容量，根据物品数量决定
const int MAXN=1700;

//o-u物品-用户矩阵
double obj_user[MAXN+5][MAXN+5];

//W[i][j]表示产品 j 愿意 分配给产品 i 的资源配额
double W[MAXN+5][MAXN+5];

//目标资源矩阵
double f_[MAXN+5];

//用户的度
int Ku[MAXN+5];

//物品的度
int Ko[MAXN+5];

//整个推荐算法的互多样性:汉明距离的平均值
double D;
//Qij:用户i和用户j的推荐列表中相同的个数
int Q[1000][1000];
//Hij:用户i和用户j的汉明距离
double H[1000][1000];
//当前测试集中共有多少条边(rating >= 3)
//int testCnt;

//遍历训练集文件，建立o-u矩阵,obj_user[i][j]=1表示用户j选择了物品i
void fileLoadOUmatrix();

//预加载用户和物品的度
void preloadDegree();
    
//cal即calculate 计算W矩阵(W对所有用户通用)
void calForW();

//计算f'和recItemList
void calForF_AndRecList(int desUserId);

//从recItemList中筛去目标用户已选的items
void removeRepe(int desUserId);

//根据目标推荐用户刷新推荐列表
void reFreshRecList(int desUserId);

//加载所有推荐列表
void refreshAllRecList();

//加载在测试集中所有用户所选择的产品
void refreshAlluser_Item();

//rij为当前itemId在对应recItemList中的相对位置
double rij;
//rij_ave为所有rij的平均值，即ranking-score
double rij_ave;
//计算排序准确度
void CalForRanking_Score();

double P_user;
double R_user;
double P_ave;
double R_ave;
double F_ave;
//目标推荐用户
int desUserId;
int existinTest = 0;
//计算精确率和召回率
void CalForPrecisionAndRecall();

//互多样性：汉明距离
void CalForHamming_Distance();
void CalWPow();
//运行入口
void Run();
//每个结构体变量中是物品编号和权值
//recItemList[1]即要推荐的第一个物品
struct recItem{
    double weight;
    int objId;
}recItemList[1000][MAXN+5];

//截取recItem前L个生成最终用户的推荐列表
struct recItemLast{
    double weight;
    int objId;
}recItemListLast[1000][L+5];

struct recItemLast recItemListLastForCalHamming[1000][L+5];

//在Test集合中每个用户已经选择的列表
int userObjList[1000][MAXN+5];

//sum_objhelove[i]:Test集中用户i的度
int sum_objhelove[1000+5];

//降序
int cmp(recItem a,recItem b);

int main(){
    clock_t st=clock();
    //遍历训练集，加载obj_user矩阵
    fileLoadOUmatrix();     
    //预加载用户的度 Ku 和物品的度 Ko (算W时要用度)
    preloadDegree();
    printf("  λ        RankingScore        Precision        Recall        Hamming\n");
    for(lamda = 0; lamda <= 1.05; lamda += 0.05){
        Run();
    }
    return 0;
}

void Run()
{
    //计算W矩阵
    calForW();
    //加载所有用户的推荐列表
    refreshAllRecList();
    //计算排序准确度
    CalForRanking_Score();
    //根据用户分割Test集合
    refreshAlluser_Item();
    //计算精确率和召回率
    CalForPrecisionAndRecall();
    CalForHamming_Distance();
    printf("%.4lf        %.4lf            %.4lf          %.4lf        %.4lf\n", lamda, rij_ave, P_ave, R_ave, 2*D/(943*942));
}
void fileLoadOUmatrix(){
    FILE *fp;
    fp=fopen("/Users/pengchen/workspace/Rs/ml-100k/ua.base","r");
    int userId,itemId,rating;
    char timeStamp[15];
    while(~fscanf(fp,"%d	%d	%d	%s",&userId,&itemId,&rating,timeStamp)){
        if(rating>=3)
            obj_user[itemId][userId]=1;
    }
    fclose(fp);
}

int cmp(recItem a,recItem b){
    return a.weight>b.weight;
}

//预处理用户的度和物品的度
void preloadDegree(){
    //用户的度
    for (int i = 1; i <=MAXN; i++) {
        int cnt=0;
        for (int j = 1; j <=MAXN; j++) {
            if(obj_user[j][i]==1){
                cnt++;
            }
        }
        Ku[i]=cnt;
    }
    //物品的度
    for (int i = 1; i <= MAXN; i++) {
        int cnt=0;
        for (int j = 1; j <= MAXN; j++) {
            if(obj_user[i][j]==1){
                cnt++;
            }
        }
        Ko[i]=cnt;
    }
}

/*
    Wij计算方式：
    同时遍历第i行和第j行，如果对应位置均为1，Wij+=对应user的度
    遍历完以后如果非0，则乘Ko[i]的倒数
*/
void calForW(){
    for (int i = 1; i <= MAXN; i++) {
        for (int j = 1; j <= MAXN; j++) {
            W[i][j]=0.0;
            for (int l = 1; l <= 1000; l++) {
                if (obj_user[i][l]==1&&obj_user[j][l]==1){
                    W[i][j]+=1.0/Ku[l];
                }
            }
            //乘以kj的倒数
            if(W[i][j]){
                W[i][j]*=pow(1.0/Ko[i],1-lamda);    //i : 热传导
                
                W[i][j]*=pow(1.0/Ko[j], lamda);      //j : 物质扩散
            }
        }
    }
}

//用户desUserId的推荐列表已经生成，不过要去除已经选择的，即removeRepe(int desUserId)
void calForF_AndRecList(int desUserId){
    //下面的计算是f'=Wf 即W矩阵和向量f相乘
    for (int i = 1; i <=MAXN; i++) {
        f_[i]=0;
        for (int j = 1; j <=MAXN; j++) {
            f_[i]+=W[i][j]*obj_user[j][desUserId];
        }
        recItemList[desUserId][i].objId=i;
        recItemList[desUserId][i].weight=f_[i];
    }
}

//讲用户desUserId的推荐列表去除已经选择的物品编号
void removeRepe(int desUserId){
    for(int i=1;i<=MAXN;i++){
        //物品i被该用户选择过
        if(obj_user[i][desUserId]){
            recItemList[desUserId][i].weight=0;
        }
    }
}

void reFreshRecList(int desUserId){
    //计算f'和recItemList
    calForF_AndRecList(desUserId);

    //筛去目标用户已选的items
    removeRepe(desUserId);

    //order by desc，生成推荐列表，目标用户最终的推荐列表
    sort(recItemList[desUserId]+1,recItemList[desUserId]+1+MAXN,cmp);
}

void refreshAllRecList(){
    for(int desUserId=1;desUserId<=943;desUserId++) {
        //刷新推荐列表
        reFreshRecList(desUserId);
    }
}


//在Test集中根据用户来分组
void refreshAlluser_Item()
{
    //定义测试集文件指针
    FILE *fp_test;
    fp_test=fopen("/Users/pengchen/workspace/Rs/ml-100k/ua.test","r");
    int userId_t,itemId_t,rating_t;
    char timeStamp_t[15];
    int cnt = 0;
    for(int desUserId=1;desUserId<=943;desUserId++) {
        fseek(fp_test,0L,SEEK_SET);
        cnt = 0;
        //select count(*) from u1test where rating >= 3;
        while(~fscanf(fp_test,"%d	%d	%d	%s",&userId_t,&itemId_t,&rating_t,timeStamp_t)){
            //检索到的文件中userId和目标推荐用户id相同，此时我们要算rij=rj/li
            //rj这个产品在训练结果中的排位，需要循环遍历训练结果来找到这个item
            if(userId_t == desUserId && rating_t >= 3){
                ++cnt;
                userObjList[desUserId][cnt] = itemId_t;
            }
        }
        sum_objhelove[desUserId] = cnt;  //用户desUserId在Test集里面的度
    }
    fclose(fp_test);
}

void CalForRanking_Score()
{
    //rij为当前itemId在对应recItemList中的相对位置
    rij = 0;
    //rij_ave为所有rij的平均值，即ranking-score
    rij_ave=0;
    //目标推荐用户
    int desUserId;
    int testCnt = 0;
    FILE *fp_test;
    fp_test=fopen("/Users/pengchen/workspace/Rs/ml-100k/ua.test","r");
    for(desUserId=1;desUserId<=943;desUserId++){
        //定义测试集文件指针
        fseek(fp_test,0L,SEEK_SET);
        int userId_t,itemId_t,rating_t;
        char timeStamp_t[15];
        //用户没有选择的产品数
        int li = 1682 - Ku[desUserId];
        
        //select count(*) from u1test where rating >= 3;
        while(~fscanf(fp_test,"%d	%d	%d	%s",&userId_t,&itemId_t,&rating_t,timeStamp_t)){
            //检索到的文件中userId和目标推荐用户id相同，此时我们要算rij=rj/li
            //rj这个产品在训练结果中的排位，需要循环遍历训练结果来找到这个item
            //li是一共有多少没有被用户选择
            if(userId_t == desUserId && rating_t >= 3){
                testCnt++;
                for(int i=1;i<=MAXN;i++){
                    if(recItemList[desUserId][i].objId==itemId_t){
                        rij=i*1.0/li;
                        rij_ave+=rij;
                        break;
                    }
                }
            }
        }
    }
    fclose(fp_test);
    rij_ave/=testCnt;
}

//计算精确率和召回率
void CalForPrecisionAndRecall()
{
    P_user = 0.0;
    R_user = 0.0;
    P_ave = 0;
    R_ave = 0;
//    F_ave = 0;
    //目标推荐用户
    int desUserId;
    existinTest = 0;
    
    for(int i = 1; i <= 1000; ++i){
        for(int j = 1; j <= L; ++j){
            recItemListLast[i][j].weight = recItemList[i][j].weight;
            recItemListLast[i][j].objId = recItemList[i][j].objId;
        }
    }
    for(desUserId=1;desUserId<=943;desUserId++){
        int sum = 0;
        for(int i = 1; i <= L; ++i){
            for(int j = 1; j <= sum_objhelove[desUserId]; ++j){
                if(recItemListLast[desUserId][i].objId == userObjList[desUserId][j]){
                    sum++;
                    break;
                }
            }
        }
        if(sum_objhelove[desUserId]){
            existinTest++;
            P_user += 1.0*sum/L;
            R_user += 1.0*sum/sum_objhelove[desUserId];
        }
    }
    P_ave = P_user/existinTest;
    R_ave = R_user/existinTest;
}

//遍历每个用户的最终推荐列表，统计不同用户推荐列表中相同物品的数量
void CalForHamming_Distance(){
    //Qij:用户i和用户j的推荐列表中相同的个数
    //Hij:用户i和用户j的汉明距离
    //整个推荐算法的互多样性:汉明距离的平均值
    D = 0;
    memset(Q, 0, sizeof(int)*1000*1000);
    memset(H, 0, sizeof(double)*1000*1000);
    for(int i = 1; i <= 942; ++i){
        for(int j = i+1; j <= 943; ++j){
            for(int k = 1; k <= L; ++k){
                for(int z = 1; z <= L; ++z){
                    
                    if(recItemListLast[i][k].objId == recItemListLast[j][z].objId){
                        ++Q[i][j];
                        break;
                    }
                }
            }
            H[i][j] = 1 - 1.0*Q[i][j]/L;
            D += H[i][j];
        }
    }
}
