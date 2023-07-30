#include "threadpool.h"

threadpool::threadpool(int actor_model, connection_pool* connPool, int thread_number) : m_actor_model(actor_model), m_thread_min_num(thread_number), m_connPool(connPool),
m_thread_live_num(thread_number), m_thread_busy_num(0), m_thread_exit_num(0), threads(std::vector<std::thread>(m_thread_max_num)) {
    auto work = [this]() {
        while (true) {
            std::function<void()> task; {
                std::unique_lock<std::mutex> lck(tasks_mtx);
                cv.wait(lck, [this]() {
                    return stop || !tasks.empty();
                    });
                if (stop && tasks.empty()) return;
                if (m_thread_exit_num > 0) {
                    m_thread_exit_num--;
                    if (m_thread_live_num > m_thread_min_num) {
                        return;
                    }
                    else {
                        continue;
                    }
                }
                task = tasks.front();
                tasks.pop();
            }
            m_thread_busy_num++;
            task();
            m_thread_busy_num--;
        }
    };
    auto adjust = [this, work](){
    while (!stop) {
        std::this_thread::sleep_for(period);
        std::unique_lock<std::mutex> lck(tasks_mtx);
        int queue_size = tasks.size();
        int thread_live_num = m_thread_live_num;
        lck.unlock();
        int thread_busy_num = m_thread_busy_num;
        if (queue_size >= wait_task_num && thread_live_num < m_thread_max_num) {
            lck.lock();
            int add = 0;
            std::cout << "扩容线程" << std::endl;
            for (int i = 0; i < m_thread_max_num && add < default_vary
                && m_thread_live_num < m_thread_max_num; i++) {
                if (threads[i].get_id() == std::thread::id()) {
                    threads[i] = std::thread(work);
                    add++;
                    m_thread_live_num++;
                }
            }
            lck.unlock();
        }

        if ((thread_busy_num * 2) < thread_live_num && thread_live_num > m_thread_min_num) {
            lck.lock();
            std::cout << "缩减线程" << std::endl;
            m_thread_exit_num = default_vary;
            lck.unlock();
            cv.notify_all();
        }

    }
};
    for (int i = 0; i < m_thread_min_num; ++i) {
        threads[i] = std::thread(work);
    }
	adjust_thread = std::thread(adjust);
}




threadpool::~threadpool() {
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        stop = true;
    }
    cv.notify_all();
    for (std::thread& th : threads) {
        if (th.joinable())
            th.join();
    }
}

void task(http_conn* request, connection_pool* connPool, int state) {
    if (!request)
        return;
    if (!state)  //
    {
        if (request->read_once())
        {
            request->improv = 1;
            connectionRAII mysqlcon(&request->mysql, connPool);
            request->process();    // 
        }
        else
        {
            request->improv = 1;
            request->timer_flag = 1;
        }
    }
    else
    {
        if (request->write())       //½«Ð´»º³åÇøÊý¾ÝÐ´³öÈ¥,·¢ËÍÍê×¢²á¶Á
        {
            request->improv = 1;
        }
        else
        {
            request->improv = 1;
            request->timer_flag = 1;
        }
    }
}

void task_p(http_conn* request, connection_pool* connPool) {
    connectionRAII mysqlcon(&request->mysql, connPool);
    request->process();
}
