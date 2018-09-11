#include <iostream>
#include <fstream>
#include <utility>
#include <iomanip>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

using namespace std;

vector<pair<string,string>> strs;
vector<pthread_t> tids;
vector<double> reses;

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock(&mutex);
//pthread_mutex_unlock(&mutex);

inline bool isLetter(const char ch)
{
    return (ch<='z'&&ch>='a')||(ch<='Z'&&ch>='A');
}

int Union(const vector<string>& strs1,const vector<string>&strs2)//there is no duplicates in both strs1 and strs2
{
    int res=0;
    unordered_set<string> rec;
    for(auto str:strs1)
    {
        rec.insert(str);
        res++;
    }
    for(auto str:strs2)
    {
        if(rec.count(str)==0)
        {
            res++;
        }
    }

    return res;
}

int Intersection(const vector<string>& strs1,const vector<string>& strs2)
{
    int res=0;
    unordered_set<string> rec;
    for(auto str:strs1)
    {
        rec.insert(str);
    }
    for(auto str:strs2)
    {
        if(rec.count(str)!=0)
        {
            res++;
        }
    }
    return res;
}

void Break(string str,vector<string>& sts )
{
    unordered_set<string> rec;
    str=str+' ';
    string tmp="";
    for(auto ch:str)
    {
        if(isLetter(ch))
        {
            tmp+=ch;
        }else
        {
            if(rec.count(tmp)==0)
            {
                sts.push_back(tmp);
                rec.insert(tmp);
            }
            tmp="";
        }
    }
}
double Jaccard(string str1,string str2)
{
    vector<string> strs1;
    vector<string> strs2;
    Break(str1,strs1);
    Break(str2,strs2);
    int u=Union(strs1,strs2);
    int i=Intersection(strs1,strs2);
    return (double)i/(double)u;
}

void* Thread(void* ptr)//pr is pair<string,string>
    //return new double
{
    clock_t begin,end;//time
    begin=clock();
    int idx=*((int*)ptr);
    free(ptr);

    pthread_mutex_lock(&mutex);
    cout<<"[TID="<<tids[idx]<<"] DocID:"<<strs[idx].first<<endl;
    pthread_mutex_unlock(&mutex);
    double res=0;

    pthread_mutex_lock(&mutex);
    for(int i=0;i<strs.size();i++)
    {
        if(i==idx) continue;
        string str=strs[i].first;
        double tmp;
        tmp=Jaccard(strs[idx].second,strs[i].second);
        res+=tmp;
        cout<<"[TID="<<tids[idx]<<"] J("<<strs[idx].first<<'.'<<strs[i].first<<")="<<setprecision(5)<<tmp<<"\n";
    }
    pthread_mutex_unlock(&mutex);

    double * ret=new double(res/(strs.size()-1));
    pthread_mutex_lock(&mutex);
    cout<<"[TID="<<tids[idx]<<"] "<<"AvgJ:"<<(*ret)<<"\n";
    pthread_mutex_unlock(&mutex);
    end=clock();
    double cpu_time=((double)(end-begin)/CLOCKS_PER_SEC);
    pthread_mutex_lock(&mutex);
    cout<<"[TID="<<tids[idx]<<"] "<<"CPU time:"<<cpu_time<<"ms\n";
    pthread_mutex_unlock(&mutex);

    pthread_exit((void*)ret);
}

int main(int argc,char** argv)
{
    clock_t begin,end;//time
    begin=clock();

    if(argc<2) {cout<<"No input file"<<endl;return 1;}


    ifstream in;
    in.open(argv[1]);
    if(!in.is_open())
    {
        cout<<"Can't open "<<argv[1]<<endl;
        return 1;
    }
    string fid;
    string content;
    while(in.peek()!=EOF)
    {
        fid="";
        content="";
        getline(in,fid);
        getline(in,content);
        strs.push_back(make_pair(fid,content));
    }
    in.close();


    for(int i=0;i<strs.size();i++)
    {
        pthread_t t;
        int *p=new int(i);
        pthread_create(&t,NULL,Thread,(void*)p);
        tids.push_back(t);
        cout<<"[Main thread]: create TID:"<<t<<",DocId:"<<strs[i].first<<"\n";
    }
    int i=0;
    for(int i=strs.size()-1;i>=0;i--)
    {
        void* ret;
        pthread_join(tids[i],&ret);
        double * db=(double*) ret;
        reses.push_back(*db);
        delete db;
    }
    int idx=0;
    for(int i=1;i<strs.size();i++)
    {
        if(reses[i]>reses[idx])
            idx=i;
    }



    cout<<"[Main Thread] KeyDocID:"<<strs[idx].first<<" HighestJ:"<<setprecision(5)<<reses[idx]<<endl;
    end=clock();
    double cpu_time=((double)(end-begin)/CLOCKS_PER_SEC);
    cout<<"[Main Thread] CPU time:"<<cpu_time<<"ms"<<endl;
    return 0;
}
