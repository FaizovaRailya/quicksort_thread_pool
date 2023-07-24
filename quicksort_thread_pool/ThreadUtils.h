#pragma once
#include <queue>
#include <mutex>
#include <future> 
#include <condition_variable>   
#include <vector> 

class ThreadPool;
// тип указатель на функцию, которая является эталоном для функций задач
typedef void (*FuncType) (std::vector<int>&, int, int, bool, ThreadPool&, int);     
typedef std::packaged_task<void()> task_type;
typedef std::future<void> res_type;

template<class T> class BlockedQueue {
    std::mutex m_locker;
    // очередь задач
    std::queue<T> m_task_queue;
    // уведомитель
    std::condition_variable m_notifier;
public:
    void push(T& item);    
    void pop(T& item);          // блокирующий метод получения элемента из очереди 
    bool fast_pop(T& item);     // неблокирующий метод получения элемента из очереди возвращает false, если очередь пуста
};

// пул потоков
class ThreadPool {
    // количество потоков
    int m_thread_count;
    // потоки
        std::vector<std::thread> m_threads;
    // очереди задач для потоков
    std::vector<BlockedQueue<task_type>> m_thread_queues;
    // для равномерного распределения задач
    int m_index;
public:
    ThreadPool();
    ~ThreadPool();
    // запуск:
    void start();
    // остановка:
    void stop();

    //Метод проброса задачи
    res_type push_task(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size);

    // функция входа для потока
    void threadFunc(int qindex);
    void run_pending_task();
};

class RequestHandler {
    // пул потоков
    ThreadPool m_tpool;
public:
    RequestHandler();
    ~RequestHandler();

    // отправка запроса на выполнение  
    res_type pushRequest(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size);
};


