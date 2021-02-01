#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include "../util/log.h"

using namespace std;
 /*
   
  说明：  条件变量condition_variable提供了线程间通讯的能力。condition_variable只能和mutex
          配合一起使用；而condition_variable_any可以和任何有lock/unlock语义的锁配合。

          condition_variable::wait()提供了在一个条件上挂起线程等待的能力。在这个条件在其它
          的线程中被满足，其它线程调用condition_variable::notify_one()或notify_all()来通知
          挂起的线程被唤醒。如果有多个在此条件上挂起的线程，notify_one()会唤醒其中一个线程，
          而notify_all()唤醒所有挂起线程，线程间进行竞争。
        
          wait()传入mutex必须是加锁的，在wait()内部，会执行如下操作:
          1)首先将传入的mutex解锁: 释放锁让其他竞争线程有机会获得锁，从而有机会让wait()等待
            的条件得以满足
          2)在条件变量上挂起等待，直到条件满足
          3)对mutex加锁,如果获得锁，进行第4步判断，如果没有，挂起
          4)再次判断条件是否满足，如果不满足，goto 1); 之所以要再一次判断条件是否满足，是
            因为notify_all()会唤醒所有等待条件的线程。但是只有一个线程能通过1234步，返回
            wait()执行后面的代码，其他线程均卡在第3步；当卡在第3步的线程获得锁后，很有可能
            条件再次变的不满足，所以需要再次判断一遍。

          wait()函数还有两个重载版本:wait_for()和wait_until()提供超时版的wait()
             
    
  作者： 李尚鹏
 */

/*
  说明：  依然用生产者、消费者的例子来演示条件变量功能。当消费者的速度大于生产者的速度时，
          生产队列经常为空，消费者不断获得锁，然后发现没有产品，师傅锁，不断循环。消费者
          线程浪费了大量了CPU时间做无效的轮询，一个比较好的办法就是：消费者如果发现生产
          队列为空，就挂起等待；当生产者放入一个产品到生产队列中后，再唤醒消费者线程继续
          处理。可以看到，用这种方式处理的消费者线程是没有占用额外CPU时间的。

          这个例子的条件变量为：生产队列非空
*/
class CondTest{


public:
    using   Product  = int;
    enum    {MAX_PRODUCT_NUM = 10000*10  };

public:
    // 生产者线程
    void      produce(){

        for ( int i =0;  i < MAX_PRODUCT_NUM; ++i ){

           {  
              lock_guard<mutex>  g(p_mutex);
              p_queue.push(i);

              // 通知生产队列非空
              p_cond_not_empty.notify_one();
           }


	  	  	if ((i + 1) % 5000 == 0){
		  	   
            	scout() << "----->生产者线程[" << this_thread::get_id() << "]执行， 放入一个元素" << i << endl;      
              this_thread::sleep_for( chrono::milliseconds(10));//加入延时，拖慢生产速度
          }      
        }
    }


    // 消费者线程
    void      consume(int read_count){

          int       i = 0 ;
          Product   p = 0 ;

          while ( i <  read_count ){
            {
                unique_lock<mutex>  lock(p_mutex);

                // 等待条件变量通知
                while ( p_queue.empty() ){
                 /*
                  wait()传入mutex必须是加锁的，在wait()内部，会执行如下操作:
                   1)首先将传入的mutex解锁: 释放锁让其他竞争线程有机会获得锁，从而有机会让wait()等待
                     的条件得以满足
                   2)在条件变量上挂起等待，直到条件满足
                   3)对mutex加锁,如果获得锁，进行第4步判断，如果没有，挂起
                   4)再次判断条件是否满足，如果不满足，goto 1); 之所以要再一次判断条件是否满足，是
                     因为notify_all()会唤醒所有等待条件的线程。但是只有一个线程能通过1234步，返回
                     wait()执行后面的代码，其他线程均卡在第3步；当卡在第3步的线程获得锁后，很有可能
                    条件再次变的不满足，所以需要再次判断一遍。
                 */
                   p_cond_not_empty.wait(lock);
                   scout() << "   ****** 消费者线程["<< this_thread::get_id() << "]被唤醒在:" << i+1 << endl;          
                }
                
                //从生产队列中取出
                p = p_queue.front();
                p_queue.pop();
                ++i;
            }

       		 if ((p + 1) % 5000 == 0)
			        scout() << "   <--消费者线程["<< this_thread::get_id() << "]执行，取出一个元素"<< p << endl;          
          }
    }

public:

    // 测试用例1： 1生产者1消费者
    static  void    run(){

        CondTest    test;
        thread      wt( &CondTest::produce, &test);
        thread      rt( &CondTest::consume, &test,MAX_PRODUCT_NUM);
 
        rt.join();
        wt.join();
    }


protected:

   queue<Product>       p_queue; // 生产队列
   mutex                p_mutex; // 排它锁
   condition_variable   p_cond_not_empty;// 条件变量：输出队列非空
};