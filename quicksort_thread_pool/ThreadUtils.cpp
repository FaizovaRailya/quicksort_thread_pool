#include "ThreadUtils.h"
#include <iostream>
#include <thread>
#include <mutex>

template<class T> void BlockedQueue<T>::push(T& item){
    std::lock_guard <std::mutex> l(m_locker);
    // ������� ���������������� push
    m_task_queue.push(std::move(item));
    // ������ ����������, ����� �����, ���������
    // pop ��������� � ������ ������� �� �������
    m_notifier.notify_one();
}

template<class T> void BlockedQueue<T>::pop(T& item){
    std::unique_lock < std::mutex > l(m_locker);
    if (m_task_queue.empty())
        // ����, ���� ������� push
        m_notifier.wait(l, [this] {
        return !m_task_queue.empty();
            });
    item = std::move(m_task_queue.front());
    m_task_queue.pop();
}

template<class T> bool BlockedQueue<T>::fast_pop(T& item){
    std::lock_guard < std::mutex > l(m_locker);
    if (m_task_queue.empty())
        // ������ �������
        return false;
    item = std::move(m_task_queue.front());
    m_task_queue.pop();
    return true;
}


ThreadPool::ThreadPool() :
    m_thread_count(std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4),
    m_thread_queues(m_thread_count),
    m_index(0) { start(); }

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {     //����� ������ �������
    for (int i = 0; i < m_thread_count; i++) {
        m_threads.emplace_back(&ThreadPool::threadFunc, this, i);
    }
}

void ThreadPool::stop(){    //����� ���������� ������
    for (int i = 0; i < m_thread_count; i++) {
        // ������ ������-�������� � ������ �������
        // ��� ���������� ������
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {
        t.join();
    }
}

res_type ThreadPool::push_task(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size) {
    // ��������� ������ �������, ���� ������� ������
    int queue_to_push = m_index++ % m_thread_count;
// ��������� �������
    task_type task([=, &arr, &tp]() { f(arr, l, r, enable, tp, multi_size); });
    auto res = task.get_future();
    // ������ � �������
    m_thread_queues[queue_to_push].push(task);
    return res;
}

void ThreadPool::threadFunc(int qindex) {
    while (true) {
        // ��������� ��������� ������
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) {
            // ������� ������ ������� ������ �� ����� �������, ������� �� �����
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }
        if (!res) {
            // �������� ����������� ��������� �������
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do.valid()) {
            // ����� �� ��������� ��������� ������ ������ ������� ������-��������
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do.valid()) { return; }
        // ��������� ������
        task_to_do();
    }
}

void ThreadPool::run_pending_task() {
    task_type task_to_do;
    bool res;
    int i = 0;
    for (; i < m_thread_count; ++i) {	
        res = m_thread_queues[i % m_thread_count].fast_pop(task_to_do);
        if (res) break;
    }
    if (!task_to_do.valid()) {	
        std::this_thread::yield();
    }
    else { task_to_do(); }
}


RequestHandler::RequestHandler() {
    m_tpool.start();
}
RequestHandler::~RequestHandler() {
     m_tpool.stop();
}

res_type RequestHandler::pushRequest(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size){
    return m_tpool.push_task(f, arr, l, r, enable, tp, multi_size);
}
