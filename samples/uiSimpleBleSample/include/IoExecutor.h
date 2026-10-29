#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <spdlog/spdlog.h>

namespace uiSimpleBleSample {

/**
 * @class IoExecutor
 * @brief Thread pool executor for non-blocking I/O operations.
 *
 * IoExecutor manages a configurable pool of worker threads that execute submitted
 * tasks asynchronously. It uses a thread-safe task queue and condition variables
 * for efficient task distribution.
 *
 * @details
 * - Thread-safe task submission via submit() method
 * - Graceful shutdown that completes remaining tasks before terminating
 * - Exception handling with spdlog error logging
 * - Configurable thread count (default: 2)
 *
 * @example
 * @code
 * // Create executor with 2 threads (default)
 * IoExecutor executor;
 *
 * // Submit a lambda task
 * executor.submit([]() {
 *     std::cout << "Running on worker thread\n";
 * });
 *
 * // Create executor with 4 threads
 * IoExecutor executor4(4);
 * @endcode
 */
class IoExecutor {
private:
    std::queue<std::function<void()>> taskQueue;  ///< Thread-safe task queue
    std::vector<std::thread> workers;              ///< Worker thread pool
    std::mutex queueMutex;                         ///< Protects task queue access
    std::condition_variable condition;             ///< Signals worker threads when tasks available
    std::atomic<bool> shutdown{false};             ///< Flag to signal shutdown
    size_t numThreads;                             ///< Number of worker threads

    /**
     * @brief Worker thread loop for processing tasks.
     *
     * Each worker thread executes this loop, waiting for tasks from the queue.
     * On shutdown, workers finish remaining tasks and exit gracefully.
     * Task exceptions are logged via spdlog without terminating the worker.
     *
     * @internal This method is called by each worker thread in the pool.
     */
    void workerLoop() {
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return !taskQueue.empty() || shutdown; });
            
            if (shutdown && taskQueue.empty()) {
                break;
            }
            
            if (!taskQueue.empty()) {
                auto task = std::move(taskQueue.front());
                taskQueue.pop();
                lock.unlock();
                
                try {
                    task();
                } catch (const std::exception& e) {
                    spdlog::error("Exception in IoExecutor task: {}", e.what());
                }
            }
        }
    }

public:
    /**
     * @brief Constructs an IoExecutor with a specified number of worker threads.
     *
     * Creates a thread pool with the given number of worker threads. Each worker
     * waits on the shared task queue for incoming tasks to execute.
     *
     * @param[in] numThreadsArg Number of worker threads to spawn. Default: 2
     *
     * @note The constructor immediately spawns all worker threads; they begin
     *       waiting for tasks upon creation.
     */
    explicit IoExecutor(size_t numThreadsArg = 2) : numThreads(numThreadsArg) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back(&IoExecutor::workerLoop, this);
        }
    }

    /**
     * @brief Destructor that gracefully shuts down the thread pool.
     *
     * Signals all worker threads to shut down after completing their current tasks.
     * Waits for all threads to finish before returning. This ensures no tasks are lost.
     *
     * @note This is a blocking operation; the destructor will not return until all
     *       worker threads have completed.
     */
    ~IoExecutor() {
        shutdown = true;
        condition.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    /**
     * @brief Submits a task to be executed asynchronously on a worker thread.
     *
     * Enqueues the given function/lambda for execution. One of the worker threads
     * will pick it up and execute it. This method returns immediately without
     * waiting for task completion.
     *
     * @tparam Func Callable type (function pointer, lambda, std::function, etc.)
     * @param[in] func The callable to execute asynchronously
     *
     * @note
     * - The function/lambda must be move-constructible
     * - Capture by value for safety when using lambdas (avoid dangling references)
     * - Exceptions in tasks are caught and logged; they do not propagate to caller
     *
     * @example
     * @code
     * executor.submit([]() { std::cout << "Task executed\n"; });
     *
     * // Capture by value for safety
     * int value = 42;
     * executor.submit([value]() { std::cout << value << "\n"; });
     * @endcode
     */
    template<typename Func>
    void submit(Func&& func) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskQueue.push(std::forward<Func>(func));
        }
        condition.notify_one();
    }
};

}  // namespace uiSimpleBleSample
