#pragma once
#include <queue>
#include <mutex>
#include <future> 
#include <condition_variable>   
#include <vector> 

class ThreadPool;
// ��� ��������� �� �������, ������� �������� �������� ��� ������� �����
typedef void (*FuncType) (std::vector<int>&, int, int, bool, ThreadPool&, int);     
typedef std::packaged_task<void()> task_type;
typedef std::future<void> res_type;

template<class T> class BlockedQueue {
    std::mutex m_locker;
    // ������� �����
    std::queue<T> m_task_queue;
    // �����������
    std::condition_variable m_notifier;
public:
    void push(T& item);    
    void pop(T& item);          // ����������� ����� ��������� �������� �� ������� 
    bool fast_pop(T& item);     // ������������� ����� ��������� �������� �� ������� ���������� false, ���� ������� �����
};

// ��� �������
class ThreadPool {
    // ���������� �������
    int m_thread_count;
    // ������
        std::vector<std::thread> m_threads;
    // ������� ����� ��� �������
    std::vector<BlockedQueue<task_type>> m_thread_queues;
    // ��� ������������ ������������� �����
    int m_index;
public:
    ThreadPool();
    ~ThreadPool();
    // ������:
    void start();
    // ���������:
    void stop();

    //����� �������� ������
    res_type push_task(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size);

    // ������� ����� ��� ������
    void threadFunc(int qindex);
    void run_pending_task();
};

class RequestHandler {
    // ��� �������
    ThreadPool m_tpool;
public:
    RequestHandler();
    ~RequestHandler();

    // �������� ������� �� ����������  
    res_type pushRequest(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size);
};


