#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "Client.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <functional>

namespace http
{
	class ThreadPool
	{
	public:
		using ClientHandler = std::function<void(Client)>;
	private:
		ClientHandler m_handler{};
		std::vector<std::thread> m_threads{};
		std::queue<Client> m_queue{};
		std::mutex m_queue_mutex{};
		std::condition_variable m_condition{};
		std::atomic<bool> m_running{ true };

		void workerThread();
	public:
		//Constructor/Destructor
		explicit ThreadPool(ClientHandler handler, 
			std::size_t thread_count = std::thread::hardware_concurrency() * 2);
		~ThreadPool();
		//delete copy/move
		ThreadPool(const ThreadPool& t) = delete;
		ThreadPool& operator=(const ThreadPool& t) = delete;
		ThreadPool(const ThreadPool&& t) = delete;
		ThreadPool& operator=(const ThreadPool&& t) = delete;

		void enqueue(Client client);
	};
}

#endif