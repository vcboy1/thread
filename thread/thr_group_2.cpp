#include <functional>
#include "usage_1.h"
#include <vector>
#include <algorithm>


 /*
   
  说明：  用STL容器来管理多个线程是实现多线程调度的基础，需要注意的是：
          std::thread是stl::move语义的，它没有拷贝构造、拷贝赋值函数，
          只有移动构造、移动赋值函数：
          std::thread  t1;
          std::thread  t2 = t1;  // 错误，拷贝构造语义

          std::thread  t1;
          std::thread  t2 = std::move(t1); // OK
          std::thread  t3(std::move(t1));  // OK

          std::move是转移所有权的操作，相当于swap

  作者：  李尚鹏

 */
using namespace std;
using namespace Usage;


int main(){

  thread          grp[]={
                          thread(Usage1::thread_proc),
                          thread([](int v)->int{
                                    std::scout() << "Usage" << v <<"：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";
                                        return 0;  
                                },10)
                        };
  int             size = 2;
  vector<thread>  tgrp;

  // 因为thread是右值，所以调用的是push_back(value_type&& __x)
  // std::move实际上是一个左值到右值的转换，即将 &转换成 &&
  // static_cast<typename std::remove_reference<_Tp>::type&&>(__t)
  for (int i=0; i < size; ++i)
        tgrp.push_back( std::move(grp[i]) );




  // 对每个线程调用join
  for_each(tgrp.begin(), tgrp.end(), std::mem_fun(&thread::join) );

  return 0;
}