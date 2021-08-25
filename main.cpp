#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <queue>
#include <condition_variable>

thread_local int logicSpaces{};
namespace {std::mutex g_display_mutex, queue_mutex;}
std::condition_variable cv,cv_handler;
#define Log(message) Logger logger(__FILE__, __FUNCTION__, __LINE__, message)

class Clock{
private:
    char timer[sizeof("9999-12-31 23:59:59.999")];
public:
    friend std::ostream& operator<<(std::ostream& out,Clock& clock);
};

std::ostream& operator<<(std::ostream& out,Clock& clock){
    using namespace std::chrono;
    auto timepoint = system_clock::now();
    auto coarse = system_clock::to_time_t(timepoint);
    auto fine = time_point_cast<std::chrono::milliseconds>(timepoint);

    std::snprintf(clock.timer + std::strftime(clock.timer, sizeof(clock.timer) - 3,
                                         "%F %T.", std::localtime(&coarse)),
                  4, "%03lu", fine.time_since_epoch().count() % 1000);

    out<<clock.timer;

    return out;
}

class SafeQueue{
private:

    std::deque<std::string> q;

public:
    void push(const std::string& message){

        bool checked = false;

        std::unique_lock<std::mutex> l(queue_mutex);

        if(q.size() <=3)std::cout<<"hasn't been queued "<<message;

        while(q.size() > 3) {

            std::cout<<"has been queued "<<message;

            cv.wait(l);

            checked = true;

        }

        if(checked) {std::cout<<"after waiting in the queue "<<message;checked = false;}

        q.push_front(message);

    }


    std::string getAndPop(){

        std::unique_lock<std::mutex> l(queue_mutex);

        std::string tmp = q.back();

        std::cout<<"storage Queue before deleting: \n\n";

        for(const auto& message: q){
            std::cout<<"\t--> "<<message<<"\n";
        }

        std::cout<<"Deleting message "<<tmp<<"\n";

        q.pop_back();

        cv.notify_one();

        return tmp;
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
    Logger(const char* file,const char* function,int line,T message) {
        for(int i = 0; i<logicSpaces;++i){

            spaces += "  ";
        }

        buffer << "thread_id ["<<std::hex<<std::this_thread::get_id()<<"] ["<<file<<" | "<<function<<
        ": "<<std::dec<<line<<"] ["<<dateAndTime<<"] "<<spaces<<message<<"\n";
        
        sq.push(buffer.str());
        cv_handler.notify_one();
    }

    ~Logger(){
        --logicSpaces;
    }   


    static void handleLogging(){

        std::unique_lock<std::mutex> l(g_display_mutex);

        while(true) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            while(sq.size() == 0)cv_handler.wait(l);

            if(sq.size() > 0) logFile << sq.getAndPop();
        }
        
    }

    static int getQueueSize(){return sq.size();}
};

SafeQueue Logger::sq;

std::ofstream Logger::logFile("logs.txt",std::ofstream::app);

void foo2(){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ++logicSpaces;
    Log("Hello from foo2!");
}

void foo1(){
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ++logicSpaces;
    Log("Hello from foo1!");
    foo2();
}

int main(){
    std::thread handler([] () {
        Logger::handleLogging();
    });
    handler.detach();
    Log("Hello from main!");
    //no jthread (no C++ 20)
    std::thread t1(foo1);
    std::thread t2(foo1);
    std::thread t3(foo1); 
    std::thread t4(foo1);
    std::thread t5(foo1);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    while(Logger::getQueueSize()!=0){cv_handler.notify_one();}
    
}