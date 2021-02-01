#include "usage_1.h"

using namespace Usage;
using namespace std;;

int main(int argc,char** argv){

    setlocale(LC_ALL,"Chinese-simplified");//设置中文环境

    // 静态成员函数调用
    thread  t1( Usage1::thread_proc);

    // 成员函数重载operator()调用
    Usage2  obj;
    thread  t2(obj);

    //成员函数调用
    Usage3  obj3;
    thread  t3(&Usage3::thread_proc,&obj3);

    //传参成员函数调用
    thread  t4(&Usage3::thread_proc2,&obj3,1,nullptr);

    //模板成员函数调用
    thread  t5(&Usage3::thread_proc3<int>, &obj3,0);

    // lamda函数
    auto  lamda = [](int v)->int{
         std::scout() << "Usage" << v <<"：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";
        return 0;  
    };
    thread  t6(lamda,4);

    // 等待线程结束
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    return 0;
}