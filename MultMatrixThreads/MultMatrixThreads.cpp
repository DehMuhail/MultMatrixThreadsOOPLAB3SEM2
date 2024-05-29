#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>

using namespace std;

template<typename T>
class matrix;

template<typename T>
ostream& operator<<(ostream& o, const matrix<T>& M);

template<typename T>
matrix<T> operator*(matrix<T>& M1, matrix<T>& M2);

template<typename T>
class matrix
{
public:
    T** a;
    int m;
    int n;
    matrix();
    matrix(int n, int m);
    ~matrix();
    matrix(const matrix<T>& M);
    matrix<T>& operator=(const matrix<T>& M);
    friend matrix<T> operator*<>(matrix<T>& M1, matrix<T>& M2);
    friend ostream& operator<<<>(ostream& o, const matrix<T>& M);

    T* operator[](int r);
};

template <typename T>
matrix<T>::matrix() : a(nullptr), m(0), n(0) {};

template <typename T>
matrix<T>::matrix(int n, int m) : n(n), m(m) {
    a = new T * [n];
    for (int i = 0; i < n; i++) {
        a[i] = new T[m];
        for (int j = 0; j < m; j++) {
            a[i][j] = 0;
        }
    }
};

template <typename T>
matrix<T>::~matrix() {
    if (a != nullptr) {
        for (int i = 0; i < n; i++) {
            delete[] a[i];
        }
        delete[] a;
        a = nullptr;
    }
};

template<typename T>
matrix<T>::matrix(const matrix& M) : n(M.n), m(M.m) {
    a = new T * [n];
    for (int i = 0; i < n; i++) {
        a[i] = new T[m];
        for (int j = 0; j < m; j++) {
            a[i][j] = M.a[i][j];
        }
    }
};

template<typename T>
matrix<T>& matrix<T>::operator=(const matrix<T>& M) {
    if (this != &M) {
        if (a != nullptr) {
            for (int i = 0; i < n; i++) {
                delete[] a[i];
            }
            delete[] a;
        }
        n = M.n;
        m = M.m;
        a = new T * [n];
        for (int i = 0; i < n; i++) {
            a[i] = new T[m];
            for (int j = 0; j < m; j++) {
                a[i][j] = M.a[i][j];
            }
        }
    }
    return *this;
};

template<typename T>
ostream& operator<<(ostream& o, const matrix<T>& M) {
    for (int i = 0; i < M.n; i++) {
        for (int j = 0; j < M.m; j++) {
            o << M.a[i][j] << '\t';
        }
        o << endl;
    }
    o << endl;
    return o;
};

template<typename T>
void part(matrix<T>& M1, matrix<T>& M2, matrix<T>& C, int i0, int i1) {
    for (int i = i0; i < i1; i++) {
        for (int j = 0; j < M2.m; j++) {
            for (int k = 0; k < M1.m; k++)
                C.a[i][j] += M1.a[i][k] * M2.a[k][j];
        }
    }
}

template<typename T>
matrix<T> operator*(matrix<T>& M1, matrix<T>& M2) {
    matrix<T> temp(M1.n, M2.m);
    if (M1.m != M2.n) {
        cout << "error *" << endl;
        return temp;
    }

    //for classic multiplication 
    for (int i = 0; i < M1.n; i++) {
        for (int j = 0; j < M2.m; j++) {
            for (int k = 0; k < M1.m; k++)
                temp.a[i][j] += M1.a[i][k] * M2.a[k][j];
        }
    }

    return temp;
};

template<typename T>
matrix<T> multiply_multithread(matrix<T>& M1, matrix<T>& M2, int num_threads) {
    matrix<T> temp(M1.n, M2.m);
    if (M1.m != M2.n) {
        cout << "error *" << endl;
        return temp;
    }

    vector<thread> threads;
    int step = M1.n / num_threads;
    for (int i = 0; i < num_threads; i++) {
        int start = i * step;
        int end = (i == num_threads - 1) ? M1.n : (i + 1) * step;
        threads.push_back(thread(part<T>, ref(M1), ref(M2), ref(temp), start, end));
    }
    for (auto& t : threads) {
        t.join();
    }

    return temp;
}

template<typename T>
T* matrix<T>::operator[](int r) {
    if (r >= n) {
        throw std::out_of_range("out of range");
    }
    return this->a[r];
}

int main() {
    const int sizes[] = { 4, 50, 100, 200, 400, 800 }; 
    const int num_threads = 5;

    for (int size : sizes) {
        matrix<int> A(size, size), B(size, size);
        matrix<int> C;


        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                A[i][j] = rand() % 10;
                B[i][j] = rand() % 10;
            }
        }


        auto t0 = chrono::high_resolution_clock::now();
        C = A * B;
        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> single_thread_duration = t1 - t0;


        t0 = chrono::high_resolution_clock::now();
        C = multiply_multithread(A, B, num_threads);
        t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> multi_thread_duration = t1 - t0;

        cout << "Matrix size: " << size << "x" << size << endl;
        cout << "Single-threaded time: " << single_thread_duration.count() << " seconds" << endl;
        cout << "Multi-threaded time (" << num_threads << " threads): " << multi_thread_duration.count() << " seconds" << endl;
        cout << "Speedup: " << single_thread_duration.count() / multi_thread_duration.count() << "x" << endl;
        cout << endl;
    }

    return 0;
}