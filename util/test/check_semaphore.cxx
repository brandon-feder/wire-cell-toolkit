#include "WireCellUtil/Semaphore.h"

using namespace WireCell;

#include <iostream>
#include <thread>
#include <vector>

const int delay_ms = 0;
std::atomic<int> counter;

void run(FastSemaphore* sem, int threadIdx, int nfor) 
{
    while (nfor-- > 0) {
        sem->acquire();
//        std::cout << "Thread " << threadIdx << " enter critical section" << std::endl;
        counter++;
//        std::cout << "Thread " << threadIdx << " incresed counter to " << counter << std::endl;

        // Do work;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

//        std::cout << "Thread " << threadIdx << " leave critical section" << std::endl;
        sem->release();
    }
}
int main(int argc, char* argv[]) {
    int nsem = 0;
    if (argc > 1) {
        nsem = atoi(argv[1]);
    }

    //Semaphore sem(nsem);
    FastSemaphore sem(nsem);

    const int nthreads = 15;
    std::vector<std::thread> threads;

    auto start = std::chrono::steady_clock::now();

    const int nloops = 1000000;
    for(int ind = 0; ind < nthreads; ++ind) {
        threads.push_back(std::thread(run, &sem, ind, nloops));
    }

    if (nsem == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Started threads, now release-priming semaphore\n";
        sem.release();
    }
    std::cout << "Joining threads\n";

    for(auto& t : threads) {
        t.join();
    }

    auto stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = stop - start;

    double duration = elapsed_seconds.count();
    double rate = counter/duration;

    std::cout
        << "nsem: " << nsem << "\n"
        << "nthreads: " << nthreads << "\n"
        << "nloops: " << nloops << "\n"
        << "delay: " << delay_ms << " ms\n"
        << "count: " << counter << "\n"
        << "time: " << duration << " s\n"
        // << "slept: " << slept << " s\n"
        // << "worked: " << worked << " s\n"
        << "rate: " << rate << " Hz\n";

    return 0;
}
