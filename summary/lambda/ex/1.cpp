#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Adder {
    Adder(int v) : value(v) {}

    int operator()(int x) const {
        return x + value;
    }

private:
    int value;
};

void test_trans_1() {
    vector<int> v = {1, 2, 3, 4, 5};
    int add_value = 10;

    // Using a functor to add a value to each element in the vector
    transform(v.begin(), v.end(), v.begin(), Adder(add_value));

    // Output the modified vector
    for (const auto& elem : v) {
        cout << elem << " ";
    }
    cout << endl;
}

void test_trans_2() {
    vector<int> v = {1, 2, 3, 4, 5};
    int add_value = 10;

    // Using a lambda function to add a value to each element in the vector
    transform(v.begin(), v.end(), v.begin(), [add_value](int x) {
        return x + add_value;
    });

    // Output the modified vector
    for (const auto& elem : v) {
        cout << elem << " ";
    }
    cout << endl;
}

void test_trans_3() {
    vector<int> v = {1, 2, 3, 4, 5};
    int add_value = 10;

    // Using a lambda function to add a value to each element in the vector
    transform(v.begin(), v.end(), v.begin(), [add_value](int x) {
        return x + add_value;
    });

    // Output the modified vector
    for (const auto& elem : v) {
        cout << elem << " ";
    }
    cout << endl;
}

void test_trans_4() {
    vector<int> v = {1, 2, 3, 4, 5};
    int add_value = 10;

    transform(v.begin(), v.end(), v.begin(), bind2nd(plus<int>(), add_value));

    // Output the modified vector
    for (const auto& elem : v) {
        cout << elem << " ";
    }
    cout << endl;
}


void test() {
    cout << "approach 1: " << endl;
    test_trans_1();
    cout << "approach 2: " << endl;
    test_trans_2();
    cout << "approach 3: " << endl;
    test_trans_3();
    cout << "approach 4: " << endl;
    test_trans_4();

}

int main() {
    test();
    return 0;
}
