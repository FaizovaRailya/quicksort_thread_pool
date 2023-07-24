#include "ThreadUtils.h"
#include <iostream>
#include <thread>
#include <mutex>

template<class T> void BlockedQueue<T>::push(T& item){
    std::lock_guard <std::mutex> l(m_locker);
    // обычный потокобезопасный push
    m_task_queue.push(std::move(item));
    // делаем оповещение, чтобы поток, вызвавший
    // pop проснулся и забрал элемент из очереди
    m_notifier.notify_one();
}

template<class T> void BlockedQueue<T>::pop(T& item){
    std::unique_lock < std::mutex > l(m_locker);
    if (m_task_queue.empty())
        // ждем, пока вызовут push
        m_notifier.wait(l, [this] {
        return !m_task_queue.empty();
            });
    item = std::move(m_task_queue.front());
    m_task_queue.pop();
}

template<class T> bool BlockedQueue<T>::fast_pop(T& item){
    std::lock_guard < std::mutex > l(m_locker);
    if (m_task_queue.empty())
        // просто выходим
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

void ThreadPool::start() {     //Метод старта потоков
    for (int i = 0; i < m_thread_count; i++) {
        m_threads.emplace_back(&ThreadPool::threadFunc, this, i);
    }
}

void ThreadPool::stop(){    //Метод завершения потока
    for (int i = 0; i < m_thread_count; i++) {
        // кладем задачу-пустышку в каждую очередь
        // для завершения потока
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {
        t.join();
    }
}

res_type ThreadPool::push_task(FuncType f, std::vector<int>& arr, int l, int r, bool enable, ThreadPool& tp, int multi_size) {
    // вычисляем индекс очереди, куда положим задачу
    int queue_to_push = m_index++ % m_thread_count;
// формируем функтор
    task_type task([=, &arr, &tp]() { f(arr, l, r, enable, tp, multi_size); });
    auto res = task.get_future();
    // кладем в очередь
    m_thread_queues[queue_to_push].push(task);
    return res;
}

void ThreadPool::threadFunc(int qindex) {
    while (true) {
        // обработка очередной задачи
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++) {
            // попытка быстро забрать задачу из любой очереди, начиная со своей
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }
        if (!res) {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do.valid()) {
            // чтобы не допустить зависания потока кладем обратно задачу-пустышку
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (!task_to_do.valid()) { return; }
        // выполняем задачу
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
