#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QtConcurrent>

/**
 * @brief The TreadPool class conteins static functions for thread
 * pool operation
 */
class ThreadPool
{
    static QMutex s_mutex;
public:
    static QFuture<void> parForAsync(size_t n, std::function<void(size_t)> Op);
    static void parFor(size_t n, std::function<void(size_t)> Op);
};

#endif // THREADPOOL_H
