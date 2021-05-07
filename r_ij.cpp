#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <queue>
#include <algorithm>
using namespace std;
double lamda=0;
const int MAXN=2000;
//物品-用户矩阵
double obj_user[MAXN+5][MAXN+5];
double W[MAXN+5][MAXN+5];
//目标资源矩阵
double f_[MAXN+5];
//用户的度
int Ku[MAXN+5];
//物品的度
int Ko[MAXN+5];
void preloadDegree();
void calForW();
struct userlist{
    double weight;
    int objId;
}userList[MAXN+5];
int cmp(userlist a,userlist b);
int main(){
    FILE *fp;
    fp=fopen("/Users/pengchen/workspace/Rs/ml-100k/ub.base","r");
    int userId,itemId,rating,cnt=0;
    char timeStamp[15];
    while(~fscanf(fp,"%d	%d	%d	%s",&userId,&itemId,&rating,timeStamp)){
        if(rating>=3)
            obj_user[itemId][userId]=1;
    }
    //预处理用户的度 Ku 和物品的度 Ko (算Wij时在求和后要除以Ko)
    preloadDegree();
    //计算W矩阵
    calForW();
    //f'=f*W
    
    double rij,rij_ave=0;
//    int descnt=16417;
    int descnt = 0;
    int desUserId;
    
    for(int uid=1;uid<=943;uid++){
        //目标推荐用户
        desUserId=uid;
        //下面的计算是f'=Wf 即W矩阵和向量f相乘
        for (int i = 1; i <=MAXN; i++) {
            f_[i]=0;
            for (int j = 1; j <=MAXN; j++) {
                f_[i]+=W[i][j]*obj_user[j][desUserId];
            }
            userList[i].objId=i;
            userList[i].weight=f_[i];
        }
        for(int i=1;i<=MAXN;i++){
            //物品i被该用户选择过
            if(obj_user[i][desUserId]){
                userList[i].weight=0;
            }
        }
        sort(userList+1,userList+1+MAXN,cmp);
        //定义测试集文件指针
        FILE *fp_test;
        fp_test=fopen("/Users/pengchen/workspace/Rs/ml-100k/ub.test","r");
        int userId_t,itemId_t,rating_t;
        char timeStamp_t[15];
        
        int li = 1682 - Ku[desUserId];
        
        //select count(*) from u1test where rating >= 3;
//        int descnt=16417; 
        
        while(~fscanf(fp_test,"%d	%d	%d	%s",&userId_t,&itemId_t,&rating_t,timeStamp_t)){
            //检索到的文件中userId和目标推荐用户id相同，此时我们要算rij=rj/li
            //rj这个产品在训练结果中的排位，需要循环遍历训练结果来找到这个item
            //li是一共有多少没有被用户选择
            if(userId_t == desUserId && rating_t >= 3){
                descnt++;
                for(int i=1;i<=MAXN;i++){
                    if(userList[i].objId==itemId_t){
                        
                        rij=i*1.0/li;
                        rij_ave+=rij;
                        break;
                    }
                }
            }
        }
    }
    rij_ave/=descnt;
    cout<<"ave : "<<rij_ave<<endl;
    cout<<descnt<<endl;
    return 0;
}

int cmp(userlist a,userlist b){
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
            //sum
            for (int l = 1; l <= MAXN; l++) {
                if (obj_user[i][l]==1&&obj_user[j][l]==1){
                    W[i][j]+=1.0/Ku[l];
                }
            }
            //乘以kj的倒数
            if(W[i][j]){
                W[i][j]*=pow(1.0/Ko[j],1-lamda);
                W[i][j]*=pow(1.0/Ko[i],lamda);
            }
        }
    }    
}
