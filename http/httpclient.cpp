#include "httplib.h"
#include <iostream>
#include <stdio.h>
#include <atomic>

using namespace httplib;
using namespace std;




class DownloadFileTask{

public:
    DownloadFileTask(const char* d_host,
                     const char* d_path,
                     const char* s_path,
                     int         d_port=80)
            :host(d_host),path(d_path),save_path(s_path),port(d_port),
             recv_size(0),total_size(0),running_status(SS_IDLE){        
    }

public:
    // 任务运行状态
    enum {  SS_IDLE    = 1,   // 等待调度
            SS_RUNNING = 2,   // 运行中
            SS_FINISH  = 3,   // 运行结束
            };

    int16_t  get_running_status() const{  return running_status;}

public:
    //下载线程
    void    operator()(){
    
        running_status = SS_RUNNING;// 运行中
 
        FILE*   fp      = fopen(save_path.c_str(),"w");
        time_t  bt      = time(0);
        size_t count    = 0;
    
        Client  cli(host,port);
        ContentReceiver reciver = [&](const char *data, size_t len) {

            fwrite(data,1,len,fp);
            count+=len;
            return true;
        };
        Progress        progress = [&](uint64_t current, uint64_t total){

            //if ( total_size == 0)
            //    std::cout <<"线程[" << this_thread::get_id() << "]观察者\n"; 
            recv_size=current;   
            total_size=total;
            return true;
        };        
        auto res = cli.Get(path.c_str(),reciver,progress);


        fclose(fp);
        bt = time(0) - bt; 
        if ( res && res->status == 200){

            cout << "  下载 "<< save_path <<" 在线程[" << this_thread::get_id() << "]耗时:"  
                 << bt << "秒，文件大小:" << count/1024/1024<< "M" << std::endl;
        }
        else
        {
            unlink( save_path.c_str() );
            cout << "  下载 "<< save_path <<" 在线程[" << this_thread::get_id() 
                << "]失败，错误代码:" << (res?res->status:-1) <<std::endl;
        }

        running_status = SS_FINISH;// 运行中
    }

public:
    string  host;      // 下载主机
    string  path;      // 下载路径
    int     port;      // 下载端口
    string  save_path; // 保存地址


    int     status;    // 服务器返回状态码


    std::atomic<uint64_t>   recv_size;  //运行中状态：当前接收字节数  
    std::atomic<uint64_t>   total_size; //运行中状态：总需要接收字节数
    std::atomic_int16_t     running_status;// 任务运行状态
};




int main(int argc,char **argv){

    DownloadFileTask    task[]={

      //  DownloadFileTask("dolphinres.mengxuan.vip","/FlBPhOS8En0Yphm_h_gUz6RUfMAW","./data/1.png"),
      //  DownloadFileTask("dolphinres.mengxuan.vip","/01%2025th_Nevern_the_farm_cow.psd","./data/2.psd")
      DownloadFileTask("outin-58b27df3f96f11eaaa0000163e1c7426.oss-cn-shanghai.aliyuncs.com",
      "/sv/3e7c1c8d-175afd97568/3e7c1c8d-175afd97568.mp4?Expires=1605168551&OSSAccessKeyId=LTAI8bKSZ6dKjf44&Signature=Wsg42G5WXZNULuQQxAIgNVrObD0%3D",
      "./data/2.mp4")

    };

    cout << "主线程["<< this_thread::get_id() <<"]上等待....\n"; 

    ThreadPool          pool(3);
    for ( int i=0; i< sizeof(task)/sizeof(task[0]); ++i)
        pool.enqueue( std::ref(task[i]));


    // 在主线程上观察第二个下载进度
    int  pos = 0;
    for (int16_t rs= 0; (rs= task[1].get_running_status()) != DownloadFileTask::SS_FINISH;)
        if ( rs == DownloadFileTask::SS_RUNNING  && task[1].total_size>0 ){
            
           //打印进度条 
           int cur_pos = task[1].recv_size*100/task[1].total_size;
           for ( ;pos < cur_pos; ++pos)
              cout<<"■";
            cout<< flush;
        }


    pool.shutdown();
}

