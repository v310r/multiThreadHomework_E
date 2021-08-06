#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <fstream>
#include <sstream>
#include <queue>
#include <condition_variable>

namespace {std::mutex g_display_mutex, queue_mutex;}
std::condition_variable cv;
#define Log(message,spaces) Logger logger(__FILE__, __FUNCTION__, __LINE__, message,spaces)

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

class SafeQueue{
private:

    std::queue<std::string> q;

public:
    void push(std::string message){

        std::unique_lock<std::mutex> l(queue_mutex);

        while(q.size() > 5) {

            cv.wait(l);

            //print queued messages
            std::cout<<this->checkFront()<<'\n';
        }

        q.push(message);
    }
    std::string getAndPop(){

        std::unique_lock<std::mutex> l(queue_mutex);

        std::string tmp = q.front();

        q.pop();

        cv.notify_one();

        return tmp;
    }
    std::string checkFront(){

        return q.front();
    }

    int size(){
        return q.size();
    }


};

class Logger{
private:

static std::ofstream logFile;

Clock dateAndTime;

std::stringstream buffer;

static SafeQueue sq;

std::string spaces = "";

public:
    template<class T>
    Logger(const char* file,const char* function,int line,T message,int spaces) {

        for(int i = 0; i<spaces;++i){

            this->spaces += "  ";
        }

        buffer << "thread_id ["<<std::hex<<std::this_thread::get_id()<<"] ["<<file<<" | "<<function<<
        ": "<<std::dec<<line<<"] ["<<dateAndTime<<"] "<<this->spaces<<message<<"\n";
        
        sq.push(buffer.str());
    }

    static int getQueueSize(){return sq.size();}

    static void handleLogging(){
        // emulate latency, so that we can check queued messages
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        while(true){
            if(sq.size()>0)
                logFile << sq.getAndPop();
        }
    }
};

SafeQueue Logger::sq;

std::ofstream Logger::logFile("logs.txt",std::ofstream::app);

void foo2(int spaces){
    Log("Hello from foo2!",++spaces);
}

void foo1(int spaces){
    Log("Hello from foo1!",++spaces);
    foo2(spaces);
}

int main(){
    int spaces = 0;
    std::thread handler([] () {
        Logger::handleLogging();
    });
    handler.detach();
    Log("Hello from main!",spaces);
    std::thread t1(foo1,spaces);
    std::thread t2(foo1,spaces);
    std::thread t3(foo1,spaces); 
    std::thread t4(foo1,spaces);
    std::thread t5(foo1,spaces);
    std::thread t6(foo1,spaces);
    std::thread t7(foo1,spaces);
    std::thread t8(foo1,spaces);
    std::thread t9(foo1,spaces);
    std::thread t10(foo1,spaces);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();
}