#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <vector>

using namespace std;

long long modPow(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1)
            result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

bool isPrime(long long n) {
    if (n < 2) return false;
    for (long long i = 2; i * i <= n; ++i)
        if (n % i == 0)
            return false;
    return true;
}

long long gcd(long long a, long long b) {
    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

long long modInverse(long long a, long long m) {
    long long m0 = m, t, q;
    long long x0 = 0, x1 = 1;

    if (m == 1)
        return 0;

    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m; a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0)
        x1 += m0;

    return x1;
}

long long generatePrime() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<long long> dist(100, 300);
    while (true) {
        long long candidate = dist(gen);
        if (isPrime(candidate)) return candidate;
    }
}


void rsaFile(const string &inputFile, const string &outputFile, long long key, long long n, bool encrypt) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);

    unsigned char buffer[2];
    while (in.read(reinterpret_cast<char *>(buffer), 2)) {
        long long block = (buffer[0] << 8) | buffer[1];
        long long processed = modPow(block, key, n);

        unsigned char outBuf[2];
        outBuf[0] = (processed >> 8) & 0xFF;
        outBuf[1] = processed & 0xFF;
        out.write(reinterpret_cast<char *>(outBuf), 2);
    }

    if (in.gcount() == 1) {
        long long block = buffer[0];
        long long processed = modPow(block, key, n);
        unsigned char outBuf[2];
        outBuf[0] = (processed >> 8) & 0xFF;
        outBuf[1] = processed & 0xFF;
        out.write(reinterpret_cast<char *>(outBuf), 2);
    }

    in.close();
    out.close();
}


int main() {
    long long p = generatePrime();
    long long q = generatePrime();
    long long n = p * q;
    long long phi = (p - 1) * (q - 1);

    long long e = 3;
    while (gcd(e, phi) != 1)
        e += 2;

    long long d = modInverse(e, phi);

    cout << "p=" << p << ", q=" << q << endl;
    cout << "n=" << n << ", phi=" << phi << endl;
    cout << "open (e=" << e << ", n=" << n << ")\n";
    cout << "close: (d=" << d << ", n=" << n << ")\n";

    rsaFile("input.bin", "encrypted.bin", e, n, true);

    rsaFile("encrypted.bin", "decrypted.bin", d, n, false);

    return 0;
}
