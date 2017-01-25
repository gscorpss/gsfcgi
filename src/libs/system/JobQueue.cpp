#include "JobQueue.h"

namespace gssystem
{

JobQueue::~JobQueue()
{
    stop();
}

bool JobQueue::start()
{
    if (thread)
        return false;
    thread.reset(new std::thread(JobQueue::threadMainFunc, this));
    return true;
}

bool JobQueue::stop()
{
    stoped = true;
    if (thread)
    {
        thread->join();
        thread.reset();
    }
}

void JobQueue::executeJob (Job* job)
{
    SlistNode* node = nullptr;
    {
        std::unique_lock<std::mutex> lock (nodeCachemutex);
        if (!nodeCache.empty())
        {
            node = *nodeCache.rbegin();
            nodeCache.pop_back();
        }
    }

    if (!node)
        node = new SlistNode;
    node->next = nullptr;
    node->job = job;

    std::unique_lock<std::mutex> lock (slistMutex);
    if (!tail)
    {
        head = tail = node;
        condVar.notify_one();
    }
    else
    {
        tail->next = node;
        tail = node;
    }
}

void JobQueue::threadMainFunc (JobQueue* queue)
{
    queue->threadMain();
}

void JobQueue::threadMain()
{
    while (!stoped)
    {
        SlistNode* node = head;
        if (node)
        {
            node->job->execute();
            if (node->next)
                head = node->next;
            else
            {
                std::unique_lock<std::mutex> lock (slistMutex);
                if (!node->next)
                    tail = nullptr;
            }
            std::unique_lock<std::mutex> lock (nodeCachemutex);
            nodeCache.push_back (node);
        }
        else
        {
            std::unique_lock<std::mutex> lock (slistMutex);
            if (!head)
                condVar.wait (lock);
        }
    }
}

} // namespace gssystem
