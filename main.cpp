#include <iostream>
#include <thread>
#include <mutex>

namespace {std::mutex g_display_mutex;}
#define Log(message) Logger logger(__FILE__, __FUNCTION__, __LINE__, message)



class Logger{
public:
    template<class T>
    Logger(const char* file,const char* function,int line,T message){
        std::unique_lock l(g_display_mutex);
        std::cout<<"thread_id ["<<std::hex<<std::this_thread::get_id()<<"] ["<<file<<" | "<<function<<
        ": "<<std::dec<<line<<"]"<<message<<std::endl;
    }
};

void myFunc2(){
    Log("Hello from myFunc2!");
}

void myFunc1(){
    Log("Hello from myFunc1!");

    myFunc2();
}


int main(){
    Log("Hello from main!");
    std::thread t(myFunc1);
    t.join();
}