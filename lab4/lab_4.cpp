#include <iostream>
#include <fstream>
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
    if (b == 0) { x = 1; y = 0; return a; }
    long long x1, y1;
    long long gcd = extendedGCD(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

long long modInverse(long long a, long long m) {
    long long x, y;
    long long g = extendedGCD(a, m, x, y);
    if (g != 1) return -1;
    return (x % m + m) % m;
}

void shamirFileProcess(const std::string &inputFile, const std::string &outputFile,
                       long long exp, long long p) {
    std::ifstream in(inputFile, std::ios::binary);
    std::ofstream out(outputFile, std::ios::binary);

    if (!in || !out) {
        std::cerr << "Ошибка открытия файлов: " << inputFile << " или " << outputFile << std::endl;
        return;
    }

    unsigned char byte;
    while (in.read(reinterpret_cast<char*>(&byte), 1)) {
        unsigned char transformed = static_cast<unsigned char>(modPow(byte, exp, p));
        out.write(reinterpret_cast<char*>(&transformed), 1);
    }

    std::cout << "Файл обработан: " << outputFile << std::endl;
}

int main() {
    long long a = 7, y = 57, p = 100;
    std::cout << a << "^" << 5 << " mod " << p << " = " << modPow(a, 5, p) << std::endl;

    long long x1, y1;
    std::cout << "GCD = " << extendedGCD(24, 40, x1, y1) << std::endl;
    std::cout << "X = " << x1 << ", Y = " << y1 << std::endl;


    std::string input = "input.bin";
    std::string step1 = "step1.bin";
    std::string step2 = "step2.bin";
    std::string step3 = "step3.bin";
    std::string output = "output.bin";

    long long pFile = 257; 
    long long cA = 7, cB = 5;
    long long dA = modInverse(cA, pFile - 1);
    long long dB = modInverse(cB, pFile - 1);

    shamirFileProcess(input, step1, cA, pFile); 
    shamirFileProcess(step1, step2, cB, pFile);
    shamirFileProcess(step2, step3, dA, pFile);  
    shamirFileProcess(step3, output, dB, pFile);  
    return 0;
}
