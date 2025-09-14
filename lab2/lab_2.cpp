#include <iostream>
#include <cmath>
#include <unordered_map>

long long modPow(long long a, long long n, long long m) {
    long long res = 1;
    a %= m;
    while (n > 0) {
        if (n % 2 == 1) res = (res * a) % m;
        a = (a * a) % m;
        n /= 2;
    }
    return res;
}

long long extendedGCD(long long a, long long b, long long &x, long long &y) {
    if (b == 0) {
        x = 1; y = 0;
        return a;
    }
    long long x1, y1;
    long long gcd = extendedGCD(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}


long long babyStepGiantStepAlt(long long a, long long y, long long p) {
    long long m = static_cast<long long>(sqrt(p)) + 1;
    long long k = m;

    std::unordered_map<long long, long long> table;
    long long val = y % p;
    for (long long j = 0; j < m; j++) {
        table[val] = j;   
        val = (val * a) % p; 
    }

    long long am = modPow(a, m, p);
    val = 1;
    for (long long i = 1; i <= k; i++) {
        val = (val * am) % p;       
        if (table.find(val) != table.end()) {
            long long j = table[val];
            long long x = i * m - j;
            return x;
        }
    }

    return -1;
}

int main() {
    long long a = 7, y = 57, p = 100;
    long long x, gcdY;

    std::cout << a << "^" << 5 << " mod " << p << " = " << modPow(a, 5, p) << std::endl;

    long long x1, y1;
    std::cout << "GCD = " << extendedGCD(24, 40, x1, y1) << std::endl;
    std::cout << "X = " << x1 << ", Y = " << y1 << std::endl;

    long long a2 = 2, y2 = 9, p2 = 23;
    x = babyStepGiantStepAlt(a2, y2, p2);
    if (x != -1)
        std::cout << "Discrete log: x = " << x << std::endl;
    else
        std::cout << "No solution found." << std::endl;

    return 0;
}
