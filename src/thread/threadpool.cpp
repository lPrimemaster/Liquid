#include "threadpool.h"

internal const u32 MAX_THREAD_CONCURRENCY = std::thread::hardware_concurrency();

ThreadPool::ThreadPool(
    u32 chunkSizeX, u32 chunkSizeY,
    Scene* world,
    Image* img,
    i32 spp,
    void (*jobFunc)(JobContext* ctx, std::mutex* img_mtx),
    std::atomic<bool>* finish,
    f32* gDonePct,
    std::mutex* gDoneMtx
)
{
    this->chunkSizeX = chunkSizeX;
    this->chunkSizeY = chunkSizeY;

    istep = img->w / chunkSizeX;
    jstep = img->h / chunkSizeY;

    numJobs = chunkSizeX * chunkSizeY;
    this->jobFunc = jobFunc;

    this->threadsDone = new std::atomic<bool>[MAX_THREAD_CONCURRENCY];
    this->done = finish;

    for(i32 i = 0; i < (i32)chunkSizeX; i++)
    {
        for(i32 j = 0; j < (i32)chunkSizeY; j++)
        {
            JobContext jc;
            jc.cam = world->renderCamera;
            jc.img = img;
            jc.id = chunkSizeY * i + j;

            jc.istart = istep * i;
            jc.jstart = jstep * j;

            jc.ispan = istep;
            jc.jspan = jstep;
            jc.spp = spp;

            jc.numJobs = numJobs;
            jc.world = world;

            jc.globalDonePct = gDonePct;
            jc.globalDoneMtx = gDoneMtx;

            jobs.push_back(jc);
        }
    }

    if(numJobs < MAX_THREAD_CONCURRENCY)
    {
        std::cerr << "warn: ThreadPool detected " << numJobs << " concurrent jobs, but your system has " << MAX_THREAD_CONCURRENCY << " threads.\n";
        std::cerr << "      There will be " << MAX_THREAD_CONCURRENCY - numJobs << " idle threads.\n";
    }
}

ThreadPool::~ThreadPool()
{
    fence();
    delete this->threadsDone;
}

internal bool AllDone(const std::atomic<bool>* data, const u32 size)
{
    for(u32 i = 0; i < size; i++)
    {
        if(!data[i].load())
            return false;
    }
    return true;
}

internal void SetAllFalse(std::atomic<bool>* data, const u32 size)
{
    for(u32 i = 0; i < size; i++)
    {
        data[i].store(false);
    }
}

internal void CheckIfAllDone(std::atomic<bool>* data, const u32 size, std::atomic<bool>* done)
{
    if(AllDone(data, size))
    {
        SetAllFalse(data, size);
        done->store(false); // NOTE: This is false (rtRender == Means not rendering anymore 'Change misleading name')
    }
}

void ThreadPool::run()
{
    if(numJobs <= MAX_THREAD_CONCURRENCY)
    {
        for(auto& job : jobs)
            threads.push_back(new std::thread([&](){ 
                jobFunc(&job, &image_mtx);
                threadsDone[job.id].store(true); 
                CheckIfAllDone(threadsDone, numJobs, done);
            }));
    }
    else
    {
        std::reverse(jobs.begin(), jobs.end());
        for(i32 i = 0; i < (i32)MAX_THREAD_CONCURRENCY; i++)
        {
            auto f = [&](std::vector<JobContext>* jobsptr, 
                        std::mutex* local_mtx, 
                        std::atomic<bool>* local_run,
                        std::mutex* image_local_mtx,
                        i32 threadID,
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

                threadsDone[threadID].store(true);
                CheckIfAllDone(threadsDone, MAX_THREAD_CONCURRENCY, done);
            };
            threads.push_back(new std::thread(f, &jobs, &mtx, &running, &image_mtx, i, jobFunc));
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
