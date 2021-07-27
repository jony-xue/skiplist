#include<iostream>
#include"skiplist.h"
#include<string>

int main(){
    SkipList<int, std::string> skiplist(10);
    skiplist.insert_element(1,"我是");
    skiplist.insert_element(2,"xue");
    skiplist.insert_element(3,"ying");
    skiplist.insert_element(4,"gang");
    skiplist.insert_element(5,"正在写");
    skiplist.insert_element(6,"跳表");
    
    std::cout<<"skip size:"<<skiplist.size()<<std::endl;

    skiplist.search_element(1);
    skiplist.display_list();

    skiplist.insert_element(7,"!");
    skiplist.display_list();

    skiplist.delete_element(7);
    skiplist.display_list();

    return 0;
}