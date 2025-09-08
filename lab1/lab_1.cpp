#include <iostream>

long long modPow(long long a, long long n, long long m) {
    long long res = 1;
    a %= m;
    while (n > 0) {
        if (n % 2 == 1) {         
            res = (res * a) % m;  
        }
        a = (a * a) % m;          
        n /= 2;                    
    }
    return res;
}

bool isPrimeFermat(long long n, int k = 5) {
    if (n < 4) return n == 2 || n == 3;
    for (int i = 0; i < k; i++) {
        long long a = 2 + rand() % (n - 3);
        if (modPow(a, n - 1, n) != 1) {
            return false;
        }
    }
    return true;
}

long long extendedGCD(long long a, long long b, long long &x, long long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    
    long long x1, y1;
    long long gcd = extendedGCD(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return gcd;
}

int main() {
    long long a = 7, n = 57, m = 100;
    long long x, y;

    std::cout << a << "^" << n << " mod " << m << " = " << modPow(a, n, m) << std::endl;
    std::cout << "PrimeNum = " << isPrimeFermat(17) << std::endl;
    std::cout << "GCD = " << extendedGCD(24, 40, x, y) << std::endl;
    std::cout << "X = " << x << '\n';
    std::cout << "Y = " << y << '\n';
    return 0;
}
