
#include <iostream>
#include <cassert>
#include "function.h"

using namespace std;

int copies = 0;
int moves = 0;

struct BigData {
    long long a[42];
};


template<typename ForSize>
struct CopyMoveCounter {
    ForSize forSize;

    CopyMoveCounter(const CopyMoveCounter &) {
        copies++;
    }

    CopyMoveCounter(CopyMoveCounter &&) {
        moves++;
    }

    CopyMoveCounter() {
        copies = moves = 0;
    };

    int operator()() const {
        return 42;
    }
};


int foo(int a, int b) {
    return a + b;
}

void test_function_pointer() {
    Function<int(int, int)> f = foo;
    assert(foo(50, 60) == 110);
}


void test_lambda() {
    auto bar = [](int a, int b) {
        return a + b;
    };
    Function<int(int, int)> f = bar;
    assert(foo(50, 60) == 110);
}


void test_copy_small() {
    CopyMoveCounter<int> cmc;
    Function<int()> f = cmc;
    assert(copies == 1);
}


void test_move_small() {
    CopyMoveCounter<int> cmc;
    Function<int()> f = std::move(cmc);
    assert(copies == 0);
}


void test_copy_big() {
    CopyMoveCounter<BigData> cmc;
    Function<int()> f = cmc;
    assert(copies == 1);
}


void test_move_big() {
    CopyMoveCounter<BigData> cmc;
    Function<int()> f = std::move(cmc);
    assert(copies == 0);
}


void test_swap() {
    CopyMoveCounter<int> a;
    auto b = []() { return 10; };
    Function<int()> f = a;
    Function<int()> g = b;

    assert(f() == 42);
    assert(g() == 10);

    f.swap(g);

    assert(f() == 10);
    assert(g() == 42);
}

int main() {
    test_function_pointer();
    test_lambda();
    test_copy_big();
    test_copy_small();
    test_move_big();
    test_move_small();
}