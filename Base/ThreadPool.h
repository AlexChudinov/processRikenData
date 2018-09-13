#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <future>
#include <algorithm>

#include "BaseObject.h"

class ThreadPool : public BaseObject
{
	Q_DISABLE_COPY(ThreadPool)
	
	BASE_OBJECT_DUMMY_STATE

public:
	using Fun = std::function<void()>;
	using Task = std::packaged_task<void()>;
	using Tasks = std::queue<Task>;
	using Future = std::future<void>;
	using Thread = std::thread;
	using Threads = std::vector<Thread>;
	using Mutex = std::mutex;
	using Locker = std::unique_lock<Mutex>;
	using ConditionVar = std::condition_variable;
	using String = std::string;

private:

	ConditionVar m_startCondition;

	size_t m_nThreadNumber;
	bool m_bStopFlag;
	Threads m_threads;

	Tasks m_tasks;

	String m_sErrorDescription;

	Mutex m_globalLock;
	Mutex m_queueLock;

	size_t m_objId;
public:
	ThreadPool();
	~ThreadPool();

	//Waiting for a vector of futures
	class FutureBunch
	{
	public:
		using FutureVector = std::vector<Future>;

		FutureBunch(FutureVector&& futures);

		void wait();

	private:
		FutureVector m_futures;
	};

	//Adds task to task queue
	Future addTask(Fun&& task);

	//Splits array into subarrays and does parallel operation on it
	FutureBunch parForAsync(size_t n, const std::function<void(size_t)>& atomicOp);
	void parFor(size_t n, const std::function<void(size_t)>& atomicOp); 

	//Returns error string from a thread pool
	String error();

	virtual size_t typeId() const;
private:
	Task getTask();

	void threadEvtLoop();

	//Returns a number of current threads
	size_t threadNumber();

	//Starts thread event loops
	void start();

	//Stops thread event loops
	void stop();

	//Joins to all current threads
	void joinAll();
};
