#include "ThreadPool.h"

namespace http
{
	ThreadPool::ThreadPool(ClientHandler handler, std::size_t thread_count)
		: m_handler{ std::move(handler) }
	{
		if (thread_count == 0)
			thread_count = 1;
		//reserve threads
		m_threads.reserve(thread_count);
		for (std::size_t i{ 0 }; i < thread_count; ++i)
		{
			m_threads.emplace_back( [this]()
									{
										workerThread();
									});
		}
	}

	ThreadPool::~ThreadPool()
	{
		m_running = false;
		//wake up threads
		m_condition.notify_all();
		//free threads
		for (auto& t : m_threads)
		{
			if (t.joinable())
				t.join();
		}
	}

	void ThreadPool::enqueue(Client client)
	{
		//move client into queue
		{
			std::lock_guard<std::mutex> lock{ m_queue_mutex };
			m_queue.push(std::move(client));
		}
		//wake up one thread to handle new client
		m_condition.notify_one();
	}

	void ThreadPool::workerThread()
	{
		while (true)
		{
			Client client{};
			{
				std::unique_lock<std::mutex> lock{ m_queue_mutex };
				//sleep while queue is empty and pool is running
				m_condition.wait(lock, [this]()
								{
									return !m_queue.empty() || !m_running;
								});
				//only return when queue is empty
				if (!m_running && m_queue.empty())
					return;
				//first client in queue gets moved and deleted from queue
				client = std::move(m_queue.front());
				m_queue.pop();
			}
			//give ownership back to WebServer
			m_handler(std::move(client));
		}
	}
}