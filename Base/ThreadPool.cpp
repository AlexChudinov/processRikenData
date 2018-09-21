#include "ThreadPool.h"

QMutex ThreadPool::s_mutex;

QFuture<void> ThreadPool::parForAsync(size_t n, std::function<void (size_t)> Op)
{
    QMutexLocker lock(&s_mutex);
    using Sequence = QVector<std::function<void()>>;

    size_t nThreads =
            static_cast<size_t>(QThreadPool::globalInstance()->maxThreadCount());
    size_t nn = n/nThreads + 1;

    static Sequence vFuns;
    vFuns.clear();

    for(size_t i = 0; i < n; i+=nn)
    {
        size_t firstIdx = i,
                lastIdx = i + nn > n ? n : i + nn;

        vFuns.push_back([=]()->void
        {
            for(size_t i = firstIdx; i < lastIdx; ++i)
            {
                Op(i);
            }
        });
    }
    return QtConcurrent::map<Sequence>
            (vFuns, [](std::function<void()> f)
    {
        f();
    });
}

void ThreadPool::parFor(size_t n, std::function<void (size_t)> Op)
{
    parForAsync(n, Op).waitForFinished();
}
