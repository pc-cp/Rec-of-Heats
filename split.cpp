//
// Created by 彭晨 on 2021/5/16.
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <ctime>

using namespace std;

struct dataset
{
    int user;
    int book_id;
    int rating;

    bool exist;
//    char timeStamp[15];
};

struct dataset arr[1000][2000];
//用户[i]一共选择多少个物品
int userhelove[1000];
//分割的比例
int splituser[1000];
int cmp(struct dataset a,struct dataset b);
//int max_book_id = 0;
//一共有多少个评分
int record = 0;
int main(){
    FILE *fp;
    fp=fopen("/Users/pengchen/workspace/Rs/ml-100k/u.data","r");

    char timeStamp[15];
    int userId, itemId, rating;
    while(~fscanf(fp,"%d        %d      %d  %s",&userId,&itemId,&rating, timeStamp)){
        ++userhelove[userId];
        arr[userId][userhelove[userId]].user = userId;
        arr[userId][userhelove[userId]].book_id = itemId;
        arr[userId][userhelove[userId]].rating = rating;
        arr[userId][userhelove[userId]].exist = false;
        ++record;
//        if(max_book_id < itemId)
//        {
//            max_book_id = itemId;
//        }
    }

    //训练集：测试集 = 9 ：1
    for(int i = 1; i <= 943; ++i)
    {
        splituser[i] = (int)(userhelove[i]*0.1)+1;
    }
//    for(int i = 1; i <= 50; ++i)
//    {
//        printf("splituser = %d\n", splituser[i]);
//    }
    FILE *f_base = NULL, *f_test = NULL;
    f_base = fopen("/Users/pengchen/workspace/Rs/333.base", "w+");  //生成训练集
    f_test = fopen("/Users/pengchen/workspace/Rs/333.test", "w+");  //生产测试集

    srand((unsigned)time(0));
    for(int i = 1; i <= 943; ++i)
    {
        int sumforchoose = 0;
        if(userhelove[i] == 0) //如果改用户没有任何评分记录，提前退出
        {
            continue;
        }

        //只有把该用户的评分分成9:1才进行分割下一个用户
        while (1)
        {
            int index = rand()%userhelove[i]+1;  //产生下标为1～userhelove[i]的随机数
//            printf("%d \n", index);
            if(arr[i][index].exist == false)
            {
                arr[i][index].exist = true;
                fprintf(f_test, "%d    %d  %d   %s\n",arr[i][index].user,arr[i][index].book_id,arr[i][index].rating, "123");
                sumforchoose++;
            }
            if(sumforchoose >= splituser[i])
            {
                break;  //while(1)出口
            }
        }

        for(int j = 1; j <= userhelove[i]; ++j)
        {
            if(arr[i][j].exist == false)
            {
//                printf("i = %d, j = %d\n", i, j);
                fprintf(f_base, "%d    %d  %d   %s\n",arr[i][j].user,arr[i][j].book_id,arr[i][j].rating, "123");
            }
        }
    }

    fclose(fp);
    fclose(f_base);
    fclose(f_test);
    printf("record = %d\n", record);
    return 0;
}

int cmp(struct dataset a,struct dataset b){
    if(a.user != b.user)
    {
        return a.user < b.user;
    }
    else
    {
        return a.book_id < b.book_id;
    }
}