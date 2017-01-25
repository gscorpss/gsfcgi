#ifndef JOBQUEUE_H
#define JOBQUEUE_H
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

namespace gssystem
{

class Job
{
public:
    virtual ~Job() {}
    virtual void execute() = 0;
};

class JobQueue
{
    struct SlistNode
    {
        SlistNode* next;
        Job* job;
    };
public:
    ~JobQueue();

    bool start();
    bool stop();
    void executeJob(Job* job);

private:
    static void threadMainFunc(JobQueue* queue);
    void threadMain();

    SlistNode* head;
    SlistNode* tail;
    std::vector<SlistNode*> nodeCache;
    std::mutex nodeCachemutex;
    std::mutex  slistMutex;
    std::unique_ptr<std::thread> thread;
    std::condition_variable condVar;
    bool stoped = false;
};

} // namespace gssystem

#endif //JOBQUEUE_H