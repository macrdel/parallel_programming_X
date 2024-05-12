#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;

void compare(vector<int>& arr, size_t i, size_t j, bool up)
{
    if (up == arr[i] > arr[j])
    {
        swap(arr[i], arr[j]);
    }
}

void bitonicConnect(vector<int>& arr, size_t lo, size_t n, bool up)
{
    if (n < 2)
        return;

    size_t m = n / 2;
    for (size_t i = lo; i < lo + m; ++i)
    {
        compare(arr, i, i + m, up);
    }
    bitonicConnect(arr, lo, m, up);
    bitonicConnect(arr, lo + m, m, up);
}

void bitonicSortRecursive(vector<int>& arr, size_t lo, size_t n, bool up)
{
    if (n < 2)
        return;

    size_t m = n / 2;
    bitonicSortRecursive(arr, lo, m, true);
    bitonicSortRecursive(arr, lo + m, m, false);
    bitonicConnect(arr, lo, n, up);
}

void printArray(const vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); i++) {
        std::cout << arr[i];
        if (i != arr.size() - 1)
            std::cout << ",";
    }
    std::cout << "]" << endl;
}

int main() {
    setlocale(LC_ALL, "ru");
    size_t n;
    std::cout << "����� ������� (������� ������): ";
    cin >> n;
	n = pow(2, n);
    vector<int> arr(n);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = dis(gen);
    }

    std::cout << "��������������� ������ ������ " << n << endl;
    printArray(arr);

    auto start = chrono::high_resolution_clock::now();

    bitonicSortRecursive(arr, 0, n, true);

    auto duration = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start);

    std::cout << "��������������� ������ " << endl;
    printArray(arr);

    std::cout << "����������� �����: " << duration.count() << " �����������" << endl;

    return 0;
}
