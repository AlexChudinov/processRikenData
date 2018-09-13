#include "ThreadPool.h"

ThreadPool::ThreadPool()
	:
	m_nThreadNumber(Thread::hardware_concurrency()),
	m_bStopFlag(false)
{
	m_objId = registerObj("ThreadPool", this);
	start();
}

ThreadPool::~ThreadPool()
{
	stop();
}

ThreadPool::Future ThreadPool::addTask(Fun&& task)
{
	m_queueLock.lock();
	m_tasks.emplace(Task(task));
	Future res = m_tasks.back().get_future();
	m_queueLock.unlock();
	m_startCondition.notify_one();
	return res;
}

ThreadPool::Task ThreadPool::getTask()
{
	Locker lock(m_queueLock);
	Task task(std::move(m_tasks.front()));
	m_tasks.pop();
	return std::move(task);
}

ThreadPool::FutureBunch ThreadPool::parForAsync
(
	size_t n, 
	const std::function<void(size_t)>& atomicOp
)
{
	size_t nn = n / threadNumber() + 1;
	std::vector<Future> vFutures;
	if (nn == 1)
	{
		for (size_t i = 0; i < n; ++i)
			vFutures.push_back(addTask([=]() { atomicOp(i); }));
	}
	else
	{
		for (size_t i = 0; i < n; i += nn)
			vFutures.push_back(
				addTask([=]()
		{
			size_t end = i + nn < n ? i + nn : n;
			for (size_t j = i; j < end; ++j)
				atomicOp(j);
		}));
	}
	return std::move(vFutures);
}

void ThreadPool::parFor(size_t n, const std::function<void(size_t)>& atomicOp)
{
	parForAsync(n, atomicOp).wait();
}

std::string ThreadPool::error()
{
	Locker lock(m_globalLock);
	return m_sErrorDescription;
}

size_t ThreadPool::typeId() const
{
	return m_objId;
}

void ThreadPool::threadEvtLoop()
{
	try
	{
		for(;;)
		{
			Locker lock(m_globalLock);
			m_startCondition.wait(lock, [&]()->bool 
			{ return !m_tasks.empty() || m_bStopFlag; });

			if (m_bStopFlag && m_tasks.empty()) return;
			else
			{
				Task task(getTask());
				lock.unlock();
				task();
			}
		}
	}
	catch (const std::exception& ex)
	{
		Locker lock(m_globalLock);
		(m_sErrorDescription += "\n") += ex.what();
	}
}

size_t ThreadPool::threadNumber()
{
	return m_nThreadNumber;
}

void ThreadPool::start()
{
	m_threads.reserve(m_nThreadNumber);
	for (size_t i = 0; i < m_nThreadNumber; ++i)
		m_threads.push_back(Thread(&ThreadPool::threadEvtLoop, this));
}

void ThreadPool::stop()
{
	{
		Locker lock(m_globalLock);
		m_bStopFlag = true;
	}
	m_startCondition.notify_all();
	joinAll();
	if (!error().empty())
	{
		throw std::domain_error(error().c_str());
	}
}

void ThreadPool::joinAll()
{
	std::for_each(m_threads.begin(), m_threads.end(), [](Thread& t) { t.join(); });
}

ThreadPool::FutureBunch::FutureBunch(FutureVector && futures)
	:
	m_futures(std::move(futures))
{
}

void ThreadPool::FutureBunch::wait()
{
	for (Future& f : m_futures)
	{
		f.wait();
	}
}
