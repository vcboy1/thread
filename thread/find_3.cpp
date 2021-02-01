#include "find_3.h"
#include <random>
 

int main(){

    const int  size = 10000 *100;
    int  buf[size];

    // 产生随机数
    srand(time(0));
    for ( int i = 0; i < size; ++i )
        //buf[i] = rand() % size;
        buf[i] = i;

    //开始查找
    int   v = rand()% size;
    int*  pos = PFind::find(buf, buf+ size, v);

    //打印查找结果
    if ( pos == buf+size )
        std::scout() << "查找 " << v << " 的位置:" << "没找到\n";
    else
        std::scout() << "查找 " << v << " 的位置:" << (pos-buf)<< std::endl; 
}