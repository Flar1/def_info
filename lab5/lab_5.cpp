#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Быстрое возведение в степень по модулю
long long modPow(long long a, long long n, long long m) {
    long long res = 1;
    a %= m;
    while (n > 0) {
        if (n % 2 == 1)
            res = (res * a) % m;
        a = (a * a) % m;
        n /= 2;
    }
    return res;
}

// === Шифрование файла ===
// Каждый байт превращаем в число (0..255) и шифруем по Эль-Гамалю
void encryptFile(const string &inputFile, const string &outputFile,
                 long long p, long long g, long long dB, long long k) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile);

    if (!in.is_open() || !out.is_open()) {
        cerr << "Ошибка открытия файла!" << endl;
        return;
    }

    unsigned char byte;
    while (in.read((char*)&byte, 1)) {
        long long m = byte; // сообщение = байт
        long long r = modPow(g, k, p);
        long long e = (m * modPow(dB, k, p)) % p;
        out << r << " " << e << "\n"; // сохраняем как пары чисел
    }

    cout << "Файл " << inputFile << " зашифрован в " << outputFile << endl;
}

// === Расшифровка файла ===
void decryptFile(const string &inputFile, const string &outputFile,
                 long long p, long long xB) {
    ifstream in(inputFile);
    ofstream out(outputFile, ios::binary);

    if (!in.is_open() || !out.is_open()) {
        cerr << "Ошибка открытия файла!" << endl;
        return;
    }

    long long r, e;
    while (in >> r >> e) {
        long long r_inv = modPow(r, p - 1 - xB, p);
        long long m = (e * r_inv) % p;
        unsigned char byte = static_cast<unsigned char>(m);
        out.write((char*)&byte, 1);
    }

    cout << "Файл " << inputFile << " расшифрован в " << outputFile << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");

    long long p = 23;
    long long g = 5;
    long long xB = 13; // секретный ключ получателя B
    long long dB = modPow(g, xB, p); // открытый ключ B

    cout << "p = " << p << ", g = " << g << endl;
    cout << "Секретный ключ B = " << xB << endl;
    cout << "Открытый ключ dB = " << dB << endl;

    cout << "\nВыберите действие:\n";
    cout << "1 - Зашифровать файл\n";
    cout << "2 - Расшифровать файл\n";
    cout << "Ваш выбор: ";

    int choice;
    cin >> choice;

    if (choice == 1) {
        string inFile, outFile;
        long long k;
        cout << "Введите имя исходного файла: ";
        cin >> inFile;
        cout << "Введите имя выходного файла (куда сохранить): ";
        cin >> outFile;
        cout << "Введите случайное число k (1 < k < p-1): ";
        cin >> k;

        encryptFile(inFile, outFile, p, g, dB, k);
    }
    else if (choice == 2) {
        string inFile, outFile;
        cout << "Введите имя зашифрованного файла: ";
        cin >> inFile;
        cout << "Введите имя выходного файла: ";
        cin >> outFile;

        decryptFile(inFile, outFile, p, xB);
    }
    else {
        cout << "Неверный выбор." << endl;
    }

    return 0;
}
