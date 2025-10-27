пушков, [27.10.2025 15:37]
return;
        }
        
        if (!output) {
            std::cerr << "Ошибка: не удалось создать выходной файл " << outputFile << std::endl;
            return;
        }
        
        // Получаем размер файла
        input.seekg(0, std::ios::end);
        size_t fileSize = input.tellg();
        input.seekg(0, std::ios::beg);
        
        // Проверяем длину ключа
        if (key.size() < fileSize) {
            std::cerr << "Ошибка: ключ слишком короткий для файла" << std::endl;
            return;
        }
        
        // Читаем, шифруем и записываем данные
        std::vector<unsigned char> buffer(fileSize);
        input.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        
        for (size_t i = 0; i < fileSize; i++) {
            buffer[i] = buffer[i] ^ key[i]; // XOR операция
        }
        
        output.write(reinterpret_cast<char*>(buffer.data()), fileSize);
        
        std::cout << "Операция завершена успешно. Обработано " << fileSize << " байт." << std::endl;
        
        input.close();
        output.close();
    }

    // Сохранение ключа в файл
    void saveKeyToFile(const std::vector<unsigned char>& key, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(key.data()), key.size());
        file.close();
        std::cout << "Ключ сохранен в файл: " << filename << " (" << key.size() << " байт)" << std::endl;
    }

    // Загрузка ключа из файла
    std::vector<unsigned char> loadKeyFromFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Ошибка: не удалось открыть файл ключа " << filename << std::endl;
            return {};
        }
        
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<unsigned char> key(fileSize);
        file.read(reinterpret_cast<char*>(key.data()), fileSize);
        file.close();
        
        std::cout << "Ключ загружен из файла: " << filename << " (" << key.size() << " байт)" << std::endl;
        return key;
    }

    // Создание тестового файла
    void createTestFile(const std::string& filename, size_t size) {
        std::ofstream file(filename, std::ios::binary);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);
        
        std::vector<unsigned char> data(size);
        for (size_t i = 0; i < size; i++) {
            data[i] = distrib(gen);
        }
        
        file.write(reinterpret_cast<char*>(data.data()), size);
        file.close();
        
        std::cout << "Тестовый файл создан: " << filename << " (" << size << " байт)" << std::endl;
    }
};

void printMenu() {
    std::cout << "\n=== Шифр Вернама с Диффи-Хеллманом ===\n";
    std::cout << "1. Создать тестовый файл\n";
    std::cout << "2. Сгенерировать случайный ключ\n";
    std::cout << "3. Сгенерировать ключ Диффи-Хеллмана\n";
    std::cout << "4. Зашифровать файл\n";
    std::cout << "5. Расшифровать файл\n";
    std::cout << "6. Сохранить ключ в файл\n";
    std::cout << "7. Загрузить ключ из файла\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите опцию: ";
}

int main() {
    VernamCipher cipher;
    std::vector<unsigned char> currentKey;
    std::string currentKeyType = "нет";
    
    setlocale(LC_ALL, "Russian");
    
    int choice;
    do {
        printMenu();
        std::cin >> choice;
        std::cin.ignore(); // очистка буфера
        
        switch (choice) {
            case 1: {
                std::string filename;
                size_t size;
                
                std::cout << "Введите имя тестового файла: ";

пушков, [27.10.2025 15:37]
std::getline(std::cin, filename);
                std::cout << "Введите размер файла в байтах: ";
                std::cin >> size;
                
                cipher.createTestFile(filename, size);
                break;
            }
            
            case 2: {
                size_t size;
                std::cout << "Введите размер ключа в байтах: ";
                std::cin >> size;
                
                currentKey = cipher.generateRandomKey(size);
                currentKeyType = "случайный";
                std::cout << "Случайный ключ сгенерирован (" << size << " байт)" << std::endl;
                break;
            }
            
            case 3: {
                size_t size;
                std::cout << "Введите размер ключа в байтах: ";
                std::cin >> size;
                
                currentKey = cipher.generateDiffieHellmanKey(size);
                currentKeyType = "Диффи-Хеллман";
                std::cout << "Ключ Диффи-Хеллмана сгенерирован (" << size << " байт)" << std::endl;
                break;
            }
            
            case 4: {
                if (currentKey.empty()) {
                    std::cout << "Ошибка: ключ не сгенерирован!" << std::endl;
                    break;
                }
                
                std::string inputFile, outputFile;
                std::cout << "Введите имя входного файла: ";
                std::getline(std::cin, inputFile);
                std::cout << "Введите имя выходного файла: ";
                std::getline(std::cin, outputFile);
                
                cipher.vernamCipher(inputFile, outputFile, currentKey);
                std::cout << "Файл зашифрован с использованием " << currentKeyType << " ключа" << std::endl;
                break;
            }
            
            case 5: {
                if (currentKey.empty()) {
                    std::cout << "Ошибка: ключ не сгенерирован!" << std::endl;
                    break;
                }
                
                std::string inputFile, outputFile;
                std::cout << "Введите имя зашифрованного файла: ";
                std::getline(std::cin, inputFile);
                std::cout << "Введите имя выходного файла: ";
                std::getline(std::cin, outputFile);
                
                cipher.vernamCipher(inputFile, outputFile, currentKey);
                std::cout << "Файл расшифрован с использованием " << currentKeyType << " ключа" << std::endl;
                break;
            }
            
            case 6: {
                if (currentKey.empty()) {
                    std::cout << "Ошибка: ключ не сгенерирован!" << std::endl;
                    break;
                }
                
                std::string filename;
                std::cout << "Введите имя файла для сохранения ключа: ";
                std::getline(std::cin, filename);
                
                cipher.saveKeyToFile(currentKey, filename);
                break;
            }
            
            case 7: {
                std::string filename;
                std::cout << "Введите имя файла с ключом: ";
                std::getline(std::cin, filename);
                
                currentKey = cipher.loadKeyFromFile(filename);
                currentKeyType = "загруженный из файла";
                break;
            }
            
            case 0:
                std::cout << "Выход из программы..." << std::endl;
                break;
                
            default:
                std::cout << "Неверный выбор!" << std::endl;
        }
        
    } while (choice != 0);
    
    return 0;
}

пушков, [27.10.2025 15:37]
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <cmath>
#include <algorithm>

class VernamCipher {
private:
    // Генерация случайного числа в диапазоне
    int generateRandomNumber(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(min, max);
        return distrib(gen);
    }

    // Проверка числа на простоту
    bool isPrime(int n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        
        for (int i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        }
        return true;
    }

    // Нахождение первообразного корня по модулю p
    int findPrimitiveRoot(int p) {
        for (int g = 2; g < p; g++) {
            bool isPrimitive = true;
            std::vector<bool> used(p, false);
            long long temp = 1;
            
            for (int i = 0; i < p - 1; i++) {
                temp = (temp * g) % p;
                if (used[temp]) {
                    isPrimitive = false;
                    break;
                }
                used[temp] = true;
            }
            
            if (isPrimitive) return g;
        }
        return -1;
    }

public:
    // Генерация ключа методом Диффи-Хеллмана
    std::vector<unsigned char> generateDiffieHellmanKey(int keySize) {
        // Генерируем простое число p
        int p;
        do {
            p = generateRandomNumber(1000, 10000);
        } while (!isPrime(p));
        
        // Находим первообразный корень g
        int g = findPrimitiveRoot(p);
        
        // Секретные ключи Алисы и Боба
        int a = generateRandomNumber(2, p - 2); // секретный ключ Алисы
        int b = generateRandomNumber(2, p - 2); // секретный ключ Боба
        
        // Открытые ключи
        long long A = 1;
        for (int i = 0; i < a; i++) {
            A = (A * g) % p;
        }
        
        long long B = 1;
        for (int i = 0; i < b; i++) {
            B = (B * g) % p;
        }
        
        // Общий секретный ключ
        long long secretKeyA = 1;
        for (int i = 0; i < a; i++) {
            secretKeyA = (secretKeyA * B) % p;
        }
        
        long long secretKeyB = 1;
        for (int i = 0; i < b; i++) {
            secretKeyB = (secretKeyB * A) % p;
        }
        
        // Преобразуем общий ключ в байтовый массив
        std::vector<unsigned char> key(keySize);
        std::mt19937 gen(secretKeyA); // используем общий ключ как seed
        
        for (int i = 0; i < keySize; i++) {
            key[i] = gen() % 256;
        }
        
        std::cout << "Диффи-Хеллман ключ сгенерирован:\n";
        std::cout << "p = " << p << ", g = " << g << "\n";
        std::cout << "Секретные ключи: a = " << a << ", b = " << b << "\n";
        std::cout << "Общий секретный ключ: " << secretKeyA << std::endl;
        
        return key;
    }

    // Генерация случайного ключа
    std::vector<unsigned char> generateRandomKey(int size) {
        std::vector<unsigned char> key(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);
        
        for (int i = 0; i < size; i++) {
            key[i] = distrib(gen);
        }
        return key;
    }

    // Шифрование/дешифрование методом Вернама
    void vernamCipher(const std::string& inputFile, const std::string& outputFile, 
                     const std::vector<unsigned char>& key) {
        std::ifstream input(inputFile, std::ios::binary);
        std::ofstream output(outputFile, std::ios::binary);
        
        if (!input) {
            std::cerr << "Ошибка: не удалось открыть входной файл " << inputFile << std::endl;
