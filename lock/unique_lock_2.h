#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <algorithm>
#include <functional>
#include "../util/log.h"
#include "lock_1.h"

 /*
   
  说明：  lock_1演示了各种锁的用法，所有锁的原始语义是阻塞的，当获取不到锁时，该线程挂起，
          直到获得锁，这是一种浪费CPU时间片的办法。各种锁还提供了一种非阻塞的轮询方法：
          lock.try_lock(),当获取不到锁时，系统立刻返回，以便线程做一些其他的处理工作。

          lock_guard提供了构造函数加锁、析构函数解锁的功能。而unique_lock则提供了一种手工
          lock/unlock的选择。这在线程之间通讯中很有用
        
  作者： 李尚鹏
 */

using namespace std;

/*
   非阻塞lock调用：

   说明:    下面用经典的生产者-消费者模型，来演示非阻塞lock的使用
            1) 生产者、消费者共享生产队列，生产队列是一个FIFO的队列
            2）生产者生产产品，放入生产队列队尾
            3) 消费者消费产品，从生产队列头部取出产品
            4) 当生产者无法获得锁时，将生产的产品放入仓库，继续生产，直到
               获得锁后，将仓库中的产品一次性放入生产队列。

            通过仓库的中转，使生产队列的吞吐量变得更高    
*/

class  TryLockTest:public MutexTest{


public:
    // 生产者线程
    void      produce1(){

        bool  getlock;

        for ( int i =0;  i < MAX_PRODUCT_NUM; ++i ){

            if ( getlock = p_mutex.try_lock() ){
             
                //获得锁：
                //  1) 如果仓库不为空，先把仓库中的产品放入生产队列，清空仓库
                if ( !respo.empty() )
                    scout() << "---------------->将仓库中的库存【"<< respo.size() <<"】个放入生产队列\n"; 

                while ( !respo.empty() ){
                   p_queue.push( respo.front());
                   respo.pop();
                }

                //  2) 放入本次产品
                p_queue.push(i);    
                p_mutex.unlock();

            }else{
                //没获得锁，放入仓库
                respo.push(i);
            }            
            

			if ((i + 1) % 5000 == 0)
				scout() << "----->生产者线程[" << this_thread::get_id() << "]执行， 放入一个元素" 
                        << i << (getlock?"到仓库":"到生产队列") << endl;            
        }
    }

public:

    // 测试用例1： 1生产者1消费者
    static  void    r1w1(){

        TryLockTest   test;
        thread      wt( &TryLockTest::produce1, &test);
        thread      rt( &TryLockTest::consume, &test,MAX_PRODUCT_NUM);
 
        rt.join();
        wt.join();
    }


protected:

   queue<Product>       respo;   // 中转仓库
};



/*
    unique_lock: 默认使用方式同lock_guard,但是提供了更多所有权控制

    unique_lock<mutex>  lock(mtx) :    等同于lock_guard<mutex> lock(mtx),构造函数lock，析构函数unlock
    
    mtx.lock();
    unique_lock<mutex>  lock(mtx,adopt_lock) :
                adopt_lock标记的效果就是假设调用一方已经拥有了互斥量的所有权（已经lock成功了）；
                通知unique_lock不需要再构造函数中lock这个互斥量了


   unique_lock<mutex>  lock(mtx,try_to_lock) :
   if (lock.owns_lock())
      ......
                try_to_lock标记提示unique_lock在构造函数中不要调用lock，而是调用mutex::try_lock,
                这是一种非阻塞的lock。


   unique_lock<mutex>  l(mtx,defer_lock) :
   l.lock();
    ....
   l.unlock();
                defer_lock标记提示unique_lock在构造函数中啥事不要干，初始化了一个没有加锁的mutex

   ---------------------------------------------------------------------------------------------

   综上所述，不同类型的标记，会影响unique_lock的构造函数处理方式，最终影响mutex的所有权，总结如下：

                          构造函数处理方式        mutex所有权
    不带标记：               mutex.lock()            true 
    adopt_lock：                无                   true
    try_to_lock：           mutex.try_lock()       视try_lock结果，成功为true
    defer_lock:                 无                   false

    unique_lock的析构函数很简单，只要拥有所有权，就调用mutex.unlock()

    例子： 
          下面用4个写线程更新计数器，来展示unique_lock的四种用法
*/
class UniqueLockTest{

    enum { MAX_CNT = 10000 * 100};

public:

    // 默认用法，等同于lock_guard<mutex>
    void     write_def(){

        for ( int i= 0; i <MAX_CNT; ++i){

            unique_lock<mutex>   lck(mtx);
            ++cnt;
        }
    }

    // adopt_lock用法，外部传入mtx时必须是锁住的
    void     write_adopt(){

        for ( int i= 0; i <MAX_CNT; ++i){

            mtx.lock();
            unique_lock<mutex>   lck(mtx,adopt_lock);
            ++cnt;
        }
    }

    // try_to_lock用法，外部传入mtx时必须是锁住的
    void     write_trylock(){

        for ( int i= 0; i <MAX_CNT; ){

            unique_lock<mutex>   lck(mtx,try_to_lock);
            if ( lck.owns_lock() ){
                ++cnt;  ++i;
            }
        }
    }

    // defer_lock用法，外部传入mtx时必须是锁住的
    void     write_defer(){

        for ( int i= 0; i <MAX_CNT; ++i ){

            unique_lock<mutex>   lck(mtx,defer_lock);
            lck.lock();
            ++cnt;
        }
    }

public:

    // 测试用例：
    static void  run(){

        UniqueLockTest  test;
        test.cnt = 0;

        thread   tgrp[] ={
            thread( &UniqueLockTest::write_def,     &test),
            thread( &UniqueLockTest::write_adopt,   &test),
            thread( &UniqueLockTest::write_trylock, &test),
            thread( &UniqueLockTest::write_defer,   &test)    
        };

        for_each(tgrp,tgrp+4, mem_fn(&thread::join));

        scout() << "unqiue_lock计数为:" << test.cnt <<endl;
    }

private:

    int     cnt;
    mutex   mtx; //排他锁
};