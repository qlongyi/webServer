#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <thread>
#include <condition_variable>
#include <future>
#include <mutex>
#include <cstdio>
#include <exception>
#include <vector>
#include <queue>
#include "../CGImysql/sql_connection_pool.h"
#include "../http/http_conn.h"

class threadpool
{

public:
    std::chrono::seconds period{ 5 };  //检查瘦身扩容周期
    int wait_task_num = 10;         //允许等待处理的任务数量
    int m_thread_min_num;           //线程池中的最小线程数
    int m_thread_max_num = 100;           //线程池中的最大线程数
    int m_thread_live_num;          //线程池中的现有线程数
    int m_thread_busy_num;          //线程池中的忙线程数
    int m_thread_exit_num;          //线程池中的需要销毁的线程数
    std::vector<std::thread> threads;           //描述线程池的数组，其大小为m_thread_number       
    std::queue<std::function<void()>> tasks;    //请求队列
    std::mutex tasks_mtx;           //保护请求队列的互斥锁
    int m_actor_model;              //并发模式选择，同步非阻塞和异步非阻塞
    std::condition_variable cv;     //请求队列不为空的条件变量
    int default_vary = 10;      //线程池容量默认变化值
    bool stop;
    connection_pool* m_connPool;  //数据库连接池
    std::thread adjust_thread;

    threadpool(int actor_model, connection_pool* connPool, int thread_number);
    ~threadpool();
    // void add(std::function<void()>);
    template<class F, class... Args>
    auto add(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;
};

void task(http_conn* request, connection_pool* m_connPool, int state);
void task_p(http_conn* request, connection_pool* m_connPool);

template<class F, class... Args>
auto threadpool::add(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);

        // don't allow enqueueing after stopping the pool
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]() { (*task)(); });
    }
    cv.notify_one();
    return res;
}
#endif
