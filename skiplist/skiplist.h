#include<iostream>
#include<string>
#include<mutex>
#include<cmath>
#include<cstdlib>
#include<string.h>

#define STORE_FILE "store/dumpFile"

std::mutex mtx; 

template<typename Key,typename Val>
class Node{
public:
    Node(){}
    Node(Key k,Val v,int);
    ~Node();
    Key getKey() const;
    Val getVal() const;

    void setValue(Val);
    
    //指向不同的level的指针
    Node<Key,Val> **forword;  
    int node_level;
private:
    Key key;
    Val val;
};

template<typename Key,typename Val>
Node<Key,Val>::Node(const Key key,const Val val,int level){
    this->key=key;
    this->val=val;
    this->node_level=level;

    //index是从0-level，所以是level+1
    this->forword=new Node<Key,Val>*[level+1];   //开始指向最高层；

    memset(this->forword,0,sizeof(Node<Key,Val>*)*(level+1));
}

template<typename Key,typename Val>
Node<Key,Val>::~Node(){
    delete []forword;
}

template<typename Key,typename Val>
Key Node<Key,Val>::getKey() const {
    return key;
}

template<typename Key,typename Val>
Val Node<Key,Val>::getVal() const {
    return val;
}

template<typename Key,typename Val>
void Node<Key,Val>::setValue(Val value) {
    this->val=value;
}




////////跳表正式开始
template<typename Key,typename Val>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<Key,Val>* create_node(Key,Val,int);
    int insert_element(Key,Val);
    void display_list();
    bool search_element(Key);
    void delete_element(Key key);
    int size();
    void dump_file();
    void load_file();

private:
    bool is_valid_string(const std::string& str);
    void get_key_value_from_string(const std::string& str,std::string* key,std::string* value);

    //最大深度
    int max_level;
    
    //当前层数
    int SkipList_level;
    
    Node<Key,Val>* header;    //跳表的头指针

    int element_count;
};

template<typename Key,typename Val>
Node<Key,Val>* SkipList<Key,Val>::create_node(const Key key,const Val val,int level){

    Node<Key,Val>* n=new Node<Key,Val>(key,val,level);
    return n;
}

template<typename Key,typename Val>
int SkipList<Key,Val>::insert_element(Key key,Val value){

    mtx.lock();   //加锁
    Node<Key,Val>* current=this->header;

    //创建一个更新矩阵并且初始化
    //＋1是因为还有个第0层；
    Node<Key,Val>* update[max_level+1];

    //大小是max_level+1个节点；
    memset(update,0,sizeof(Node<Key,Val>*) *(max_level+1));

    //从最高层对每一层进行插入元素，插入的位置是大于其key的前一个位置；
    //从跳表的顶层开始
    for(int i=SkipList_level;i>=0;i--){
        while(current->forword[i]!=NULL && current->forword[i]->getKey()<key)
        {
            current=current->forword[i];
        }
        update[i]=current;
    }

    current=current->forword[0];

    //
    if(current != NULL  && current->getKey()==key){
        std::cout<<"key:"<<key<<"exist"<<std::endl;
        mtx.unlock();
        return 1;
    }
    //如果没有合适的插入位置,则从尾部插入
    if(current ==NULL || current->getKey() !=key){
        //产生一个随机数，这个随机数是要从底到高要插入的层数
        int random_level=get_random_level();   

        //需要添加新的层
        if(random_level > SkipList_level){
            
            for(int i=SkipList_level+1;i<random_level+1;i++){
                update[i]=header;
            }
            //更新层数
            SkipList_level=random_level;
        }
        //在生成的随机层上插入节点
        Node<Key,Val>* insert_node=create_node(key,value,random_level);
        for(int i=0;i<=random_level;i++){
            insert_node->forword[i]=update[i]->forword[i];
            update[i]->forword[i]=insert_node;
        }
        std::cout<<"insert successfully!"<<std::endl;
        element_count++;
    }
    mtx.unlock();
    return 0;
}


template<typename Key,typename Val>
void SkipList<Key,Val>::display_list(){
    std::cout<<"\n**********skip list***********\n";
    for(int i=0;i<=SkipList_level;i++){
        Node<Key,Val>* node=this->header->forword[i];
        std::cout<<"Level"<<i<<" : ";
        while(node != NULL){
            std::cout<<node->getKey()<<" :"<<node->getVal()<<" ";
            node=node->forword[i];
        }
        std::cout<<std::endl;
    }
}

template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return element_count;
}


template<typename Key,typename Val>
void SkipList<Key,Val>::delete_element(Key key){
    mtx.lock();
    Node<Key,Val>* cur=this->header;
    Node<Key,Val>* update[max_level+1];

    memset(update,0,sizeof(Node<Key,Val>*)*(max_level+1));

    //从最高层开始删除
    for(int i=SkipList_level;i>=0;i--){
        while(cur->forword[i] !=NULL && cur->forword[i]->getKey() < key)
        {
            cur=cur->forword[i];
        }
        update[i]=cur;
    }

    cur=cur->forword[0];
    if(cur != NULL &&cur->getKey()==key){
        for(int i=0;i<=SkipList_level;i++){
            if(update[i]->forword[i]!=cur){
                break;
            }
            update[i]->forword[i]=cur->forword[i];
        }
        //如果某一层的删除之后没有其他元素了，则层数需要减少;
        while(SkipList_level >0 && header->forword[SkipList_level]==0){
            SkipList_level--;
        }
        std::cout<<"successfully delete key:"<<key<<std::endl;
        element_count--;
    }
    mtx.unlock();
    return ;
}

template<typename Key,typename Val>
bool SkipList<Key,Val>::search_element(Key key){
    std::cout<<"search element"<<std::endl;
    Node<Key,Val>* cur=header;

    for(int i=SkipList_level;i>=0;i--){
        while(cur->forword[i] && cur->forword[i]->getKey() <key)
        {
            cur=cur->forword[i];
        }
    }

    cur=cur->forword[0];

    if( cur && cur->getKey() ==key){
        std::cout<<"Found key: "<<key<<",value:"<<cur->getVal()<<std::endl;
        return true; 
    }

    std::cout<<"Not find"<<std::endl;
    return false;
}

template<typename Key,typename Val> 
SkipList<Key,Val>::SkipList(int maxlev){
    this->max_level=maxlev;
    this->SkipList_level=0;
    this->element_count=0;

    Key key;
    Val val;
    this->header=new Node<Key,Val>(key,val,maxlev);
}

template<typename Key,typename Val>
int SkipList<Key,Val>::get_random_level(){
    int k=1;
    while(rand()%2){
        k++;
    }
    k=(k<max_level)?k:max_level;
    return k;
};


template<typename Key,typename Val>
SkipList<Key,Val>::~SkipList(){
    delete header;
}

template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0]; 

    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

// Load data from disk
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}