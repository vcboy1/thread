#include "lock_1.h"


int main(){

    // 排它锁测试：1读1写
    //MutexTest::r1w1();

    // 排它锁测试：1读2写
    //MutexTest::r1w2();

    // 读写锁测试：用排他锁
    //SharedMutexTest::use_mutex();
    
    // 读写锁测试：用共享锁
    //SharedMutexTest::use_shared_mutex();

    // 原子锁测试
    AtomicTest::cmp();

    // 各种锁速度测试
    //SpeedTest::test();
    return 0;
}



