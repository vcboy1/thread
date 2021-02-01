#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>

namespace std{


/*
  说明：  线程安全输出类
  作者：  李尚鹏
*/

struct scout : public stringstream {
  
    static std::mutex cout_mutex;

    ~scout() {

        std::lock_guard<mutex> l {cout_mutex};
        std::cout << rdbuf();
        cout.flush();
    }
};

std::mutex scout::cout_mutex;
}