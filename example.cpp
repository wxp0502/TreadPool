#include <iostream>


#include "ThreadPool.h"

int add(int a, int b) {
    std::cout << "a == " << a << ", b == " << b << std::endl;
    std::cout << "a + b == " << a + b << std::endl;
    return a + b;
}

int main() {

    ThreadPool pool(4);
    std::vector<std::future<int> > results;

    results.reserve(1);
    for (int i = 0; i < 8; ++i) {
//        results.emplace_back(pool.enqueue(add, i, i + 1));
        results.emplace_back(
                pool.enqueue([i] {
                    std::cout << "hello " << i << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    std::cout << "world " << i << std::endl;
                    return i * i;
                })
        );
    }

    for (auto &&result: results)
        std::cout << result.get() << '\n';
    std::cout << std::endl;

    return 0;
}