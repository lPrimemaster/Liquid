#include "threadpool.h"

internal const u32 MAX_THREAD_CONCURRENCY = std::thread::hardware_concurrency();

ThreadPool::ThreadPool(
    u32 chunkSizeX, u32 chunkSizeY,
    Scene* world,
    Image* img, Camera* cam, 
    void (*jobFunc)(JobContext* ctx, std::mutex* img_mtx)
)
{
    this->chunkSizeX = chunkSizeX;
    this->chunkSizeY = chunkSizeY;

    istep = img->w / chunkSizeX;
    jstep = img->h / chunkSizeY;

    numJobs = chunkSizeX * chunkSizeY;
    this->jobFunc = jobFunc;

    for(i32 i = 0; i < (i32)chunkSizeX; i++)
    {
        for(i32 j = 0; j < (i32)chunkSizeY; j++)
        {
            JobContext jc;
            jc.cam = cam;
            jc.img = img;
            jc.id = chunkSizeY * i + j;

            jc.istart = istep * i;
            jc.jstart = jstep * j;

            jc.ispan = istep;
            jc.jspan = jstep;

            jc.numJobs = numJobs;
            jc.world = world;

            jobs.push_back(jc);
        }
    }

    if(numJobs < MAX_THREAD_CONCURRENCY)
    {
        std::cerr << "warn: ThreadPool detected " << numJobs << " concurrent jobs, but your system has " << MAX_THREAD_CONCURRENCY << " threads.\n";
        std::cerr << "      There will be " << MAX_THREAD_CONCURRENCY - numJobs << " idle threads.\n";
    }
}

void ThreadPool::run()
{
    if(numJobs <= MAX_THREAD_CONCURRENCY)
    {
        for(auto& job : jobs)
            threads.push_back(new std::thread(jobFunc, &job, &image_mtx));
    }
    else
    {
        std::reverse(jobs.begin(), jobs.end());
        for(i32 i = 0; i < (i32)MAX_THREAD_CONCURRENCY; i++)
        {
            auto f = [](std::vector<JobContext>* jobsptr, 
                        std::mutex* local_mtx, 
                        std::atomic<bool>* local_run,
                        std::mutex* image_local_mtx,
                        void (*local_jobFunc)(JobContext* ctx, std::mutex* img_mtx)) -> void
            {
                while(local_run->load())
                {
                    std::unique_lock<std::mutex> lock(*local_mtx);
                    if(jobsptr->size() == 0)
                        break;

                    JobContext ctx = jobsptr->back();
                    jobsptr->pop_back();
                    lock.unlock();
                    local_jobFunc(&ctx, image_local_mtx);
                }
            };
            threads.push_back(new std::thread(f, &jobs, &mtx, &running, &image_mtx, jobFunc));
        }
    }
}

void ThreadPool::fence() const
{
    for(auto t : threads)
    {
        t->join();
        delete t;
    }
}
