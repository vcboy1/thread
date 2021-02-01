#pragma once

#include <future>
#include <thread>
#include <vector>
#include "../util/log.h"

using namespace std;
/*
  说明：  async可以启动一个异步task,并返回一个future对象，通过future.get()
          可以阻塞等待异步task的返回。用async+future可以启动线程并获得线程
          的返回结果和异常状态

  作者：  李尚鹏
*/

class FutureTest{

    enum { MAX_SIZE = 10000 * 10 };

public:
    // 线程函数
    template<class Iter, class V>
    static  Iter    find(Iter begin ,Iter end,V v){

        for ( Iter cur = begin; cur != end; ++cur)
            if (*cur == v)
              return cur;
        return end;
    }

public:

    // 测试用例
    static   void  run(){

        int     buf[MAX_SIZE];


        future<int*> result = async(&FutureTest::find<int*,int>, 
                                    (int*)buf ,(int*)(buf+MAX_SIZE),100 );
        
        // get()将阻塞等待线程函数返回
        int*   pos    = result.get();

        if ( pos != buf + MAX_SIZE)
            scout() << "发现目标在位置:" << (pos-buf) <<endl;
        else
            scout() << "没有发现目标\n";
    }


};