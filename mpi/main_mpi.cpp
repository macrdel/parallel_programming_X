#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <mpi.h>

using namespace std;

int* local_arr, * temp_arr, * merge_arr;
double start, stop;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> dis(1, 1000);
bool debug = true;

void mergeLow(int arr_size, int* arr1, int* arr2) {
    int index1 = 0;
    int index2 = 0;
    merge_arr = new int[arr_size];
    for (int i = 0; i < arr_size; i++)
        if (arr1[index1] <= arr2[index2]) {
            merge_arr[i] = arr1[index1];
            index1++;
        }
        else {
            merge_arr[i] = arr2[index2];
            index2++;
        }

    for (int i = 0; i < arr_size; i++)
        arr1[i] = merge_arr[i];
    delete [] merge_arr;
}

void mergeHigh(int arr_size, int* arr1, int* arr2) {
    int index1 = arr_size - 1;
    int index2 = arr_size - 1;
    merge_arr = new int[arr_size];
    for (int i = arr_size - 1; i >= 0; i--)
        if (arr1[index1] >= arr2[index2]) {
            merge_arr[i] = arr1[index1];
            index1--;
        }
        else {
            merge_arr[i] = arr2[index2];
            index2--;
        }

    for (int i = 0; i < arr_size; i++)
        arr1[i] = merge_arr[i];
    delete [] merge_arr;
}

void mergeSplit(int arr_size, int* local_arr, bool flag, int partner, MPI_Comm comm) {
    MPI_Status status;
    temp_arr = new int[arr_size];
    MPI_Sendrecv(local_arr, arr_size, MPI_INT, partner, 0, temp_arr, arr_size, MPI_INT, partner, 0, comm, &status);
    if (flag)
        mergeHigh(arr_size, local_arr, temp_arr);
    else
        mergeLow(arr_size, local_arr, temp_arr);
    delete[] temp_arr;
}

void bitonic_increase(int list_size, int* local_arr, int proc, MPI_Comm comm) {
    unsigned eor_bit;
    int partner, rank;
    MPI_Comm_rank(comm, &rank);

    proc = log2(proc);
    eor_bit = 1 << (proc - 1);
    for (int i = 0; i < proc; i++)
    {
        partner = rank ^ eor_bit;
        if (rank < partner)
            mergeSplit(list_size, local_arr, false, partner, comm);
        else
            mergeSplit(list_size, local_arr, true, partner, comm);
        eor_bit = eor_bit >> 1;
    }
}

void bitonic_decrease(int list_size, int* local_arr, int proc, MPI_Comm comm) {
    unsigned eor_bit;
    int partner, rank;
    MPI_Comm_rank(comm, &rank);

    proc = log2(proc);
    eor_bit = 1 << (proc - 1);
    for (int i = 0; i < proc; i++)
    {
        partner = rank ^ eor_bit;
        if (rank > partner)
            mergeSplit(list_size, local_arr, false, partner, comm);
        else
            mergeSplit(list_size, local_arr, true, partner, comm);
        eor_bit = eor_bit >> 1;
    }
}

void printArray(const int* arr, size_t size) {
    std::cout << "[";
    for (size_t i = 0; i < size; i++) {
        std::cout << arr[i];
        if (i != size - 1)
            std::cout << ",";
    }
    std::cout << "]" << std::endl;
}


int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");
    int arr_size, n, rank, p;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = atoi(argv[1]);
    n = pow(2, n);
    debug = debug == atoi(argv[2]);

    arr_size = n / p;
    local_arr = new int[arr_size];

    for (int i = 0; i < arr_size; i++)
    {
        local_arr[i] = dis(gen);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (debug)
    {
        cout << "Процесс " << rank << ":\n";
        printArray(local_arr, arr_size);
    }

    start = MPI_Wtime();

    std::sort(local_arr, local_arr + arr_size);

    for (int proc = 2, andBit = 2; proc <= p; proc *= 2, andBit <<= 1)
        if ((rank & andBit) == 0)
            bitonic_increase(arr_size, local_arr, proc, MPI_COMM_WORLD);
        else
            bitonic_decrease(arr_size, local_arr, proc, MPI_COMM_WORLD);
    // MPI_Barrier(MPI_COMM_WORLD);
    if (!rank)
    {
        stop = MPI_Wtime();
        cout << "Затраченное время: " << (stop - start) * 1000000.0 << " микросекунд\n";
    }
    
    if (debug)
    {
        int* final = new int[n];
        MPI_Gather(local_arr, arr_size, MPI_INT, final, arr_size, MPI_INT, 0, MPI_COMM_WORLD);
        if (!rank)
        {
            cout << "N = " << n << "\n";
            cout << "P = " << p << "\n";

            std::cout << "Отсортированный массив" << endl;
            printArray(final, n);
            // delete[] final;
        }
    }

    MPI_Finalize();

    return 0;
}