# ThreadPool
A basic thread-pool using new C++11 features.

## Example

```cpp
#include <iostream>     // std::cout
#include <thread>       // std::this_thread
#include <chrono>       // std::chrono
#include <functional>   // std::function

#include "threadpool.h"

void count() {
    for (volatile size_t i=0; i<1000000; ++i) {
        continue;
    }
}

void count_to(size_t to) {
    for (volatile size_t i=0; i<to; ++i) {
        continue;
    }
}

int main(int argc, const char * argv[]) {

    std::cout << "Hello, World!" << std::endl;
    
    // Initializes a thread-pool with 2 workers thread
    ThreadPool tp(2);
    
    // Does some work
    tp.push(&count);
    tp.emplace<std::function<void(size_t)>, typename std::function<void(size_t)>::argument_type >(&count_to,300000);
    tp.emplace(&count);
    
    // Waits 3 seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // Does some work
    tp.emplace(&count_to,1000000);
    tp.emplace([](size_t from, size_t to){ size_t i=from; while(++i<to){}; } , 0, 5000000);

    return 0;
}
```
