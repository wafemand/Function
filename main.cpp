
#include <iostream>
#include "function.h"

using namespace std;

int kek(int, int) {
    return 100;
}


struct QQQ{
    QQQ(const QQQ&){
        cout << "copy QQQ\n";
    }
    QQQ(QQQ&&){
        cout << "move QQQ\n";
    }
    QQQ() = default;
    int operator()(){
        return 228;
    }
};


int main() {
    Function<int(int, int)> lol(kek);

    uint32_t q = 0xFFFFFFFF;
    Function<int(int, int)> mem([q](int a, int b) {
        return a + b;
    });

    Function<int()> qqq = QQQ();
    //const std::function<int()> qqq = QQQ();

    auto qqq2 = std::move(qqq);
    cout << lol(10, 20) << ' ' << mem(20, 30) << ' ' << qqq2();

}