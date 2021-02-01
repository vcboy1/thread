#pragma once

#include <thread>
#include "../util/log.h"

namespace Usage{
/*
  说明：  展现std::thread的几种标准调用方法
  作者：  李尚鹏
*/


/*
   用法1：  通过类静态成员函数调用
            std::thread  t( Usage1::thread_proc )
*/
class Usage1{

public:

  static  void thread_proc(){

      std::scout() << "Usage1：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";
  }
};





/*
   用法2：  通过类实例的operator()函数调用
            Usage2   obj;
            std::thread  t(obj);
*/
class Usage2{

public:

   // opertor()可以带若干个参数,注意没有static
   void     operator()(){

        std::scout()  << "Usage2：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";
   }
};





/*
   用法3：  类实例的成员函数调用
            Usage3   obj;
            std::thread  t(obj);

            类实例的成员函数带参数调用
            thread  t3(&Usage3::thread_proc,&obj3);

            模板函数调用
            thread  t5(&Usage3::thread_proc3<int>, &obj3,0);

*/
class  Usage3{

public:

   void    thread_proc(){

      std::scout() << "Usage3：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";     
   }

   void    thread_proc2(int v,char*){

       std::scout() << "Usage3参数版：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";     
   }

   template<class T>
   void   thread_proc3(T v){
       std::scout() << "Usage3模板成员版：进入线程[" <<std::this_thread::get_id()<< "]处理函数.... \n";  
   }
};



};