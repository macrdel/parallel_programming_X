#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <omp.h>

using namespace std;

int m;

void bitonicSortPar(vector<int>& seq, int start, int length, bool up)
{
    int i;
    int split_length;

    if (length == 1)
        return;

    split_length = length / 2;

    // bitonic split
#pragma omp parallel for shared(seq, up, start, split_length) private(i)
    for (i = start; i < start + split_length; i++)
    {
        if (up)
        {
            if (seq[i] > seq[i + split_length])
                swap(seq[i], seq[i + split_length]);
        }
        else
        {
            if (seq[i] < seq[i + split_length])
                swap(seq[i], seq[i + split_length]);
        }
    }

    if (split_length > m)
    {
        // m is the size of sub part-> n/numThreads
        bitonicSortPar(seq, start, split_length, up);
        bitonicSortPar(seq, start + split_length, split_length, up);
    }
}

void bitonicSortSeq(vector<int>& seq, int start, int length, bool up)
{
    int i;
    int split_length;

    if (length == 1)
        return;

    split_length = length / 2;

    for (i = start; i < start + split_length; i++)
    {
        if (up)
        {
            if (seq[i] > seq[i + split_length])
                swap(seq[i], seq[i + split_length]);
        }
        else
        {
            if (seq[i] < seq[i + split_length])
                swap(seq[i], seq[i + split_length]);
        }
    }

    bitonicSortSeq(seq, start, split_length, up);
    bitonicSortSeq(seq, start + split_length, split_length, up);
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
    int n;
    std::cout << "              (              ): ";
    cin >> n;
    vector<int> arr(n);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000);
    for (int i = 0; i < n; ++i) {
        arr[i] = dis(gen);
    }

    std::cout << "                              " << n << endl;
    printArray(arr);

    int numThreads = 2; //omp_get_max_threads();
    m = n / numThreads;

    int i, j;
    bool flag;

    auto start = chrono::high_resolution_clock::now();

    for (i = 2; i <= m; i = 2 * i)
    {
#pragma omp parallel for shared(i, arr) private(j, flag)
        for (j = 0; j < n; j += i)
        {
            flag = (j / i) % 2 == 0;
            bitonicSortSeq(arr, j, i, flag);
        }
    }
   
    for (i = 2; i <= numThreads; i = 2 * i)
    {
        for (j = 0; j < numThreads; j += i)
        {
            flag = (j / i) % 2 == 0;
            bitonicSortPar(arr, j * m, i * m, flag);
        }
#pragma omp parallel for shared(j)
        for (j = 0; j < numThreads; j++)
        {
            flag = j < i;
            bitonicSortSeq(arr, j * m, m, flag);
        }
    }

    auto duration = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start);

    std::cout << "                       " << endl;
    printArray(arr);

    std::cout << "                 : " << duration.count() << "            " << endl;

    return 0;
}
