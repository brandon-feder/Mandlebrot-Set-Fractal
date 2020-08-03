#include <chrono>
#include <iostream>

long getTime() {
    std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(time);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);

    return value.count();
}