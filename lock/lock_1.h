#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <map>
#include <algorithm>
#include <functional>
#include <shared_mutex>
#include <atomic>
#include <ctime>
#include "../util/log.h"

 /*
   
  说明： 用各种锁来保证线程间共享数据的一致性，不同级别的锁对并发性的影响是不一样的
         
          排它锁：  std::mutex 
                    一旦加锁，其他加锁动作阻塞等待，并发性最差
          读写锁：  std::shared_mutex
                    并发读不锁，有读锁时加写阻塞；有写锁时所有加读锁阻塞，并发性较好
          原子锁：  std::atomic<>
                    提供硬件级别的原子操作语义的基本类型读写锁，并发性能更高
   
         从竞争性的角度而言，锁住的代码段越少，并发性能越高。锁住的内存区域越小，产生竞争性
         的可能也就越少。有时候，即使访问的内存区不一样，也需要加锁，比如vector的插入会影响
         内部数据块的expend，这是全局影响。

         即使使用的类是线程安全的，不正确的使用依然会造成并发竞争，比如：
         ThreadSafeStack  stack;

         if ( !stack.empty() )
              stack.pop();
         empty和pop都是线程安全函数，但是运行在empty和pop之间，系统切换其他线程，并调用了pop，
         再回到原线程执行时，非empty的条件已经不满足
        
  作者： 李尚鹏

 */
using namespace std;


/*
   排它锁： 最简单的锁，任何一个线程获得锁后，其他线程请求锁均被阻塞等待

   说明:    下面用经典的生产者-消费者模型，来演示排它锁的使用
            1) 生产者、消费者共享生产队列，生产队列是一个FIFO的队列
            2）生产者生产产品，放入生产队列队尾
            3) 消费者消费产品，从生产队列头部取出产品

            在后端，异步消息队列是是一个广泛被使用的生产者、消费者模型    
*/
class  MutexTest{

public:
    using   Product  = int;
    enum    {MAX_PRODUCT_NUM = 10000*10  };

public:
    // 生产者线程
    void      produce(){

        for ( int i =0;  i < MAX_PRODUCT_NUM; ++i ){

            p_mutex.lock();
               
                // 放入生产队列
                p_queue.push(i);
            
            p_mutex.unlock();

			if ((i + 1) % 5000 == 0)
				scout() << "----->生产者线程[" << this_thread::get_id() << "]执行， 放入一个元素" << i << endl;            
        }
    }


    // 消费者线程
    void      consume(int read_count){

          int       i = 0 ;
          Product   p = 0 ;

          while ( i <  read_count ){

              {
                /*
                    lock_guard是一个工具类，它在构造函数lock，析构函数unlock
                    如果程序代码有多条路径返回，或者有可能有异常产生，用lock_guard
                    类可以保证所有情况下都正常调用了unlock,避免死锁
                */  
                lock_guard<mutex>  gurad(p_mutex);

                //从生产队列中取出
                if ( !p_queue.empty() ){
                
                    p = p_queue.front();
                    p_queue.pop();
                    ++i;
                }
                else
                    continue;    
              }

       		 if ((p + 1) % 5000 == 0)
			  scout() << "   <--消费者线程["<< this_thread::get_id() << "]执行，取出一个元素"<< p << endl;          
          }
    }

public:

    // 测试用例1： 1生产者1消费者
    static  void    r1w1(){

        MutexTest   test;
        thread      wt( &MutexTest::produce, &test);
        thread      rt( &MutexTest::consume, &test,MAX_PRODUCT_NUM);
 
        rt.join();
        wt.join();
    }

    // 测试用例2：  2生产者1消费者
    static  void    r1w2(){

        MutexTest   test;
        thread      wt1( &MutexTest::produce, &test);
        thread      wt2( &MutexTest::produce, &test);
        thread      rt( &MutexTest::consume, &test,MAX_PRODUCT_NUM*2);
 
        rt.join();
        wt1.join();
        wt2.join();        
    }

protected:

   queue<Product>       p_queue; // 生产队列
   mutex                p_mutex; // 排它锁
};




/*
   读写锁：  相比排他锁，读写锁允许更高的并行性。读写锁也叫做“共享-独占锁”，
            当读写锁以读模式锁住时，它是以共享模式锁住的；当它以写模式锁住时，
            它是以独占模式锁住的。 文件打开锁就是一个典型的读写锁，同一个文件
            允许多个并发读，但是有读锁时，就无法用写模式打开文件

            排它锁是mutex, 读写锁是shared_mutex,和排他锁一样，它有lock\unlock函数
            加读锁时用工具类: std::shared_lock<shared_mutex> 
            加写锁时用工具类：std::lock_guard<shared_mutex>
            这两个工具类都是构造函数lock，析构函数unlock

   应用场景：读多写少场景，比如缓存、字典等

   注意：    c++14才支持

   说明：    实现了一个基于读写锁的缓存共享访问。在读密集场景下，访问缓存是无锁的，
             这极大提高了多线程的并发能力。如果采用排它锁，即使多个线程都是读访问，
             也必须排队访问。
*/

class SharedMutexTest{

public:
    enum  { KEY =1};

    SharedMutexTest(){

        cache[KEY] = 0;
    }

public:

   // 读线程从缓存上读
   void     reader(int index){

       int v;

       for (int i =0; i < 2; ++i){
            
            {
                    lock_guard<mutex>    g(emutex);

                    /*  查找缓存，并延时一秒，如果使用排它锁，三个读要耗费三秒的时间
                        如果使用读写锁，三个读可以并发在一秒完成
                    */
                    v = cache[KEY];
                    this_thread::sleep_for(chrono::seconds(1));
           

            }
            scout() << "  读线程#" << index << "查找缓存完毕:" << v << endl ; 
       }
   }



  // 写线程更新缓存
   void    writer(){

       for (int i =0; i < 2; ++i){
            {
                 lock_guard<mutex>    g(emutex);
                 //lock_guard<shared_mutex>    g(smutex);

                /*  每三秒更新一次缓存 */
                cache[KEY] = cache[KEY]+1;
               
            scout() << "写线程更新缓存完毕\n";

            }
      
            this_thread::sleep_for(chrono::seconds(3));
       }
   }

   // 共享读线程从缓存上读
   void     shared_reader(int index){

       int v;

       for (int i =0; i < 2 *3; ++i){
            
            {
                    shared_lock<shared_mutex>    g(smutex);

                    /*  查找缓存，并延时一秒，如果使用排它锁，三个读要耗费三秒的时间
                        如果使用读写锁，三个读可以并发在一秒完成
                    */
                    v = cache[KEY];
                    this_thread::sleep_for(chrono::seconds(1));

            }
            scout() << "  读线程#" << index << "查找缓存完毕:" << v << endl ;           
            
       }
   }

public:

    // 测试用例: 用排他锁测试并发性
    static void    use_mutex(){

        SharedMutexTest  test;

        thread    wt( &SharedMutexTest::writer, &test);
        this_thread::sleep_for(chrono::milliseconds(100));

        thread    rts[] = {
                    thread( &SharedMutexTest::reader, &test,1),
                    thread( &SharedMutexTest::reader, &test,2),
                    thread( &SharedMutexTest::reader, &test,3)
        } ;

        wt.join();
        for_each( rts, rts + 3, mem_fn(&thread::join) );
    }

    // 测试用例: 用读写锁测试并发性
    static void    use_shared_mutex(){

        SharedMutexTest  test;

        thread    wt( &SharedMutexTest::writer, &test);
        this_thread::sleep_for(chrono::milliseconds(100));

        thread    rts[] = {
                    thread( &SharedMutexTest::shared_reader, &test,1),
                    thread( &SharedMutexTest::shared_reader, &test,2),
                    thread( &SharedMutexTest::shared_reader, &test,3)
        } ;

        wt.join();
        for_each( rts, rts + 3, mem_fn(&thread::join) );
    }


private:
    map<int,int>       cache;   // 缓存字典 
    mutex              emutex;  // 排他锁
    shared_mutex       smutex;  // 读写锁
};





/*
   原子锁：  提供硬件级别的原子操作语义的基本类型读写锁。它支持的类型有：
            bool char  short int long float double....
            原子锁atomic<>是个模板类，并重载相关操作符，使用这些操作符来
            改变值是线程安全的
   
   例子：    atomic<int>  cnt(0);

             线程安全的表达式：
             ++cnt;   --cnt;  cnt=v;   v=cnt;
             cnt+=n;  cnt-=n;  cnt*=n; cnt/=n; |= &=
             
             非线程安全的表达式
             cnt = cnt + n;
   
   应用场景：计数器、开关标志等基于基本类型的锁定

   说明：    
*/
class AtomicTest{

public:
    enum {  MAX_TIMES = 10000 * 100 };

 public:

    // 多个线程同步更新计数器,带原子锁版本
    void    writer_atomic(){

        for (int i=0; i < MAX_TIMES; ++i)
            counter++;
    }

    // 多个线程同步更新计数器,裸奔版本
    void    writer(){

        /*
           counter2++编译成汇编代码至少包含三个指令：
              lea  reg_a, counter2
              add  reg_a
              sea  reg_a, counter2

            如果counter放入寄存器后，切换到其他线程完成++后，再切换回来执行2、3语句
            会把其他线程写的++值覆盖  
        */
        for (int i=0; i < MAX_TIMES; ++i)
            counter2++;
    }

public:
    // 测试用例：比较裸奔版本和带原子锁版本
    static  void   cmp(){

        AtomicTest  test;
        
        test.counter  =0;
        test.counter2 =0;

        // 开启两个裸奔线程
        thread      tg[]={
            thread( &AtomicTest::writer,&test),
            thread( &AtomicTest::writer,&test),
        };
        for_each( tg,tg+2, mem_fn(&thread::join));


        // 开启两个原子锁版本
         thread      tga[]={
            thread( &AtomicTest::writer_atomic,&test),
            thread( &AtomicTest::writer_atomic,&test),
        };
        for_each( tga,tga+2, mem_fn(&thread::join));       

        // 打印求和结果
        scout() << "原子版本求和:" << test.counter << "\n裸奔版本求和:" << test.counter2 << endl;
    }

private:
    int             counter2; //  原始计数器
    atomic<int>     counter;  //  计数器
};






/*
   说明 ：  比较各种锁的开销：
*/
class SpeedTest{

    enum {  MAX_TIMES = 10000 * 10000 };

    // 测试排它锁速度
    void    speed_mutex(){

        clock_t  t = clock();
        for ( int i =0,v=0; i < MAX_TIMES; ++i){

            lock_guard<mutex>  g(emutex);
            ++v;
        }        
        scout() << "排它锁运行求和时间：" << (clock()-t)/1000 << " ms\n";
    }


    // 测试排它锁速度
    void    speed_shared_mutex(){

        clock_t  t = clock();
        for ( int i =0,v=0; i < MAX_TIMES; ++i){

            lock_guard<shared_mutex>  g(smutex);
            ++v;
        }        
        scout() << "读写锁运行求和时间：" <<(clock()-t)/1000 << " s\n";
    }


    // 测试原子锁速度
    void    speed_atomic(){

        clock_t  t = clock();
        for ( int i =0,v=0; i < MAX_TIMES; ++i)
            ++amutex;
                
        scout() << "原子锁运行求和时间：" << (clock()-t)/1000<< " s\n";
    }

    // 测试无锁速度
    void    speed_no_mutex(){

        clock_t  t = clock();
        for ( int i =0,v=0; i < MAX_TIMES; ++i)
            ++v;
                
        scout() << "  无锁运行求和时间：" <<(clock()-t)/1000 << " s\n";
    }

public:

    // 测试用例
    static  void    test(){

        SpeedTest test;

        test.speed_mutex();
        test.speed_shared_mutex();
        test.speed_atomic();
        test.speed_no_mutex();
    }

private:
    mutex           emutex; // 排它锁
    shared_mutex    smutex; // 读写锁
    atomic<int>     amutex; // 原子锁
};

