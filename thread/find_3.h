#pragma once

#include <algorithm>
#include <functional>
#include <thread>
#include "../util/log.h"



 /*
   
  说明：  用线程来实现stl算法库中的算法，将其改造成并行算法：
  作者：  李尚鹏

 */


 /*
   说明： 实现std::find(begin,end,v)算法：
          在[begin,end)无序区间搜索指定V, 找到返回位置，未找到返回end,算法复杂度O(N)
          改为并行版本后，理想算法复杂度O(N)/M， M为内核核心数
   注意： 本版本由于未做线程同步，性能不够理想
 */
class  PFind{

public:
    using  Int = int;

    //  线程处理函数：查找指定对象
    static void    find_impl(Int* b, Int* e, Int v, bool& found, Int*& pos){
        
        Int* cur = b;
        for ( ; cur != e && ! found; ++cur)
            
            if ( *cur == v ){
                std::scout() << "thread[" << std::this_thread::get_id() << "] 找到匹配值.....\n";
                pos = cur;  
                found = true;
                return ;
            } 

        if ( cur != e )
            std::scout() << "thread[" << std::this_thread::get_id() << "] 搜索被中断.....\n";
        else
            std::scout() << "thread[" << std::this_thread::get_id() << "] 搜索完成，没有找到....\n";
        
        
    }

public:

    static Int* find(Int* b, Int* e, Int v){

         // 获取硬件核心数,开启核心-1个线程
        int core_cnt = std::thread::hardware_concurrency();

        //  查找对象
        bool    found    = false;
        int     seg_size = (e-b)/core_cnt;

        //  开启多个线程查找
        int     i   = 0;
        Int*    pos = e;
        std::vector<std::thread>  tgrp;
        for ( ; i < core_cnt-1; ++i)
       
            // thread在传递引用对象时，要用std::ref()包裹，因为thread对象会在内部拷贝复制参数
            // 如果不加std::ref()，线程函数引用的就是thread内部复制对象的引用了。
            tgrp.push_back(
                std::thread( PFind::find_impl,
                             b+i*seg_size, b+(i+1)*seg_size, v, std::ref(found), std::ref(pos))
            );

         // 主线程也查找
         find_impl(b+i*seg_size, e,v, std::ref(found), std::ref(pos));
        
         //  等待查询结果
         std::for_each( tgrp.begin(), tgrp.end(), std::mem_fn(&std::thread::join) );

         return pos;
    }

};
