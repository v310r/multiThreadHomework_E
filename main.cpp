#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <fstream>
#include <sstream>

namespace {std::mutex g_display_mutex;}
#define Log(message) Logger logger(__FILE__, __FUNCTION__, __LINE__, message)


class Clock{
private:
    time_t timer;
public:
    friend std::ostream& operator<<(std::ostream& out,Clock& clock);
};

std::ostream& operator<<(std::ostream& out,Clock& clock){
    time(&clock.timer);
    std::string current_time = (ctime(&clock.timer));
    current_time.erase(current_time.end()-1);
    out<<current_time;
    return out;
}

class Logger{
private:
static std::ofstream logFile;
Clock dateAndTime;
std::stringstream buffer;
static std::string spaces;
public:
    template<class T>
    Logger(const char* file,const char* function,int line,T message) {
        spaces += "    ";
        buffer << "thread_id ["<<std::hex<<std::this_thread::get_id()<<"] ["<<file<<" | "<<function<<
        ": "<<std::dec<<line<<"] ["<<dateAndTime<<"] "<<spaces<<message<<"\n";
        //queue maximum size is fixed --> 1
        std::lock_guard<std::mutex> l(g_display_mutex);
        logFile<< buffer.str();
    }
    ~Logger(){
        if(!spaces.empty())
            spaces.resize(spaces.size()-4);
    }
};

std::ofstream Logger::logFile("logs.txt",std::ofstream::app);

std::string Logger::spaces{""};

void foo5(){
    Log("Hello from foo2!");
}

void foo4(){
    Log("Hello from foo2!");
    foo5();
}

void foo3(){
    Log("Hello from foo2!");
    foo4();
}

void foo2(){
    Log("Hello from foo2!");
}

void foo1(){
    Log("Hello from foo1!");
    foo2();
}


int main(){
    Log("Hello from main!");
    std::thread t1(foo1);
    std::thread t2(foo3);
    t1.join();
    t2.join();
}