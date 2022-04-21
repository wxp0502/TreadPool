#include <iostream>
#include <random>

#include "../include/ThreadPool.h"

void simulate_hard_computation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 + rand() % 2000));
}

void add(int a, int b) {
    simulate_hard_computation();
    std::cout << a << " + " << b << " == " << a + b << std::endl;
}

int add_return(int a, int b) {
    simulate_hard_computation();
    std::cout << a << " + " << b << " == " << a + b << std::endl;
    return a + b;
}


int main() {

    ThreadPool pool(4);

    for (int i = 0; i < 8; ++i) {
        for (int j = i + 1; j < 8; ++j) {
            pool.enqueue(add, i, j);
        }
    }

    auto future1 = pool.enqueue(add_return, 50, 60);
    std::cout << future1.get() << std::endl;

    pool.shutDown();

    return 0;
}