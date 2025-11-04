#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <openssl/md5.h>
#include <openssl/sha.h>

class ElGamalSignature {
private:
    long long p;  
    long long g; 
    long long x; 
    long long y; 

    long long mod_pow(long long base, long long exponent, long long modulus) {
        long long result = 1;
        base = base % modulus;
        
        while (exponent > 0) {
            if (exponent % 2 == 1) {
                result = (result * base) % modulus;
            }
            exponent = exponent >> 1;
            base = (base * base) % modulus;
        }
        return result;
    }

    long long gcd(long long a, long long b) {
        while (b != 0) {
            long long temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    long long extended_gcd(long long a, long long b, long long& x, long long& y) {
        if (b == 0) {
            x = 1;
            y = 0;
            return a;
        }
        
        long long x1, y1;
        long long d = extended_gcd(b, a % b, x1, y1);
        x = y1;
        y = x1 - y1 * (a / b);
        return d;
    }

    long long mod_inverse(long long a, long long m) {
        long long x, y;
        long long g = extended_gcd(a, m, x, y);
        if (g != 1) {
            throw std::runtime_error("Обратный элемент не существует");
        }
        return (x % m + m) % m;
    }

public:
    ElGamalSignature() {
        p = 30803;  
        g = 2;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long long> dis(2, p - 2);
        x = dis(gen);
        
        y = mod_pow(g, x, p);
    }

    ElGamalSignature(long long p_val, long long g_val, long long x_val) 
        : p(p_val), g(g_val), x(x_val) {
        y = mod_pow(g, x, p);
    }

    std::vector<long long> get_public_key() {
        return {p, g, y};
    }

    std::vector<unsigned char> compute_hash(const std::vector<unsigned char>& data) {
        std::vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
        SHA256(data.data(), data.size(), hash.data());
        return hash;
    }

    std::vector<std::pair<long long, long long>> sign_file(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Не удалось открыть файл: " + filename);
        }
        
        std::vector<unsigned char> file_data(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        file.close();

        std::vector<unsigned char> hash = compute_hash(file_data);
        
        std::vector<std::pair<long long, long long>> signature;
        
        for (unsigned char byte : hash) {
            long long m = static_cast<long long>(byte);
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<long long> dis(2, p - 2);
            
            long long k;
            do {
                k = dis(gen);
            } while (gcd(k, p - 1) != 1);
            
            long long r = mod_pow(g, k, p);
            
            long long k_inv = mod_inverse(k, p - 1);
            long long s = (k_inv * (m - x * r)) % (p - 1);
            if (s < 0) s += (p - 1);
            
            signature.push_back({r, s});
        }
        
        return signature;
    }

    bool verify_signature(const std::string& filename, 
                         const std::vector<std::pair<long long, long long>>& signature,
                         const std::vector<long long>& public_key) {
        if (public_key.size() != 3) {
            throw std::runtime_error("Неверный формат открытого ключа");
        }
        
        long long p_verify = public_key[0];
        long long g_verify = public_key[1];
        long long y_verify = public_key[2];
        
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Не удалось открыть файл: " + filename);
        }
        
        std::vector<unsigned char> file_data(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        file.close();

        std::vector<unsigned char> hash = compute_hash(file_data);
        
        if (hash.size() != signature.size()) {
            return false;
        }
        
        for (size_t i = 0; i < hash.size(); i++) {
            long long m = static_cast<long long>(hash[i]);
            long long r = signature[i].first;
            long long s = signature[i].second;
            
            if (r <= 0 || r >= p_verify || s <= 0 || s >= p_verify - 1) {
                return false;
            }
            
            long long left = mod_pow(g_verify, m, p_verify);
            long long right = (mod_pow(y_verify, r, p_verify) * mod_pow(r, s, p_verify)) % p_verify;
            
            if (left != right) {
                return false;
            }
        }
        
        return true;
    }

    void save_signature(const std::vector<std::pair<long long, long long>>& signature, 
                       const std::string& signature_file) {
        std::ofstream file(signature_file);
        if (!file) {
            throw std::runtime_error("Не удалось создать файл подписи: " + signature_file);
        }
        
        file << p << " " << g << " " << y << "\n";
        
        for (const auto& pair : signature) {
            file << pair.first << " " << pair.second << "\n";
        }
        
        file.close();
    }

    std::pair<std::vector<long long>, std::vector<std::pair<long long, long long>>> 
    load_signature(const std::string& signature_file) {
        std::ifstream file(signature_file);
        if (!file) {
            throw std::runtime_error("Не удалось открыть файл подписи: " + signature_file);
        }
        
        long long p_val, g_val, y_val;
        file >> p_val >> g_val >> y_val;
        std::vector<long long> public_key = {p_val, g_val, y_val};
        
        std::vector<std::pair<long long, long long>> signature;
        long long r, s;
        while (file >> r >> s) {
            signature.push_back({r, s});
        }
        
        file.close();
        return {public_key, signature};
    }
};

void print_menu() {
    std::cout << "=== Электронная подпись Эль-Гамаля ===\n";
    std::cout << "1. Создать подпись файла\n";
    std::cout << "2. Проверить подпись файла\n";
    std::cout << "3. Выход\n";
    std::cout << "Выберите действие: ";
}

int main() {
    try {
        ElGamalSignature elgamal;
        int choice;
        
        do {
            print_menu();
            std::cin >> choice;
            std::cin.ignore(); 
            
            switch (choice) {
                case 1: {
                    std::string filename, signature_file;
                    std::cout << "Введите имя файла для подписи: ";
                    std::getline(std::cin, filename);
                    std::cout << "Введите имя файла для сохранения подписи: ";
                    std::getline(std::cin, signature_file);
                    
                    auto signature = elgamal.sign_file(filename);
                    elgamal.save_signature(signature, signature_file);
                    
                    auto public_key = elgamal.get_public_key();
                    std::cout << "Файл успешно подписан!\n";
                    std::cout << "Открытый ключ (p, g, y): " 
                              << public_key[0] << ", " 
                              << public_key[1] << ", " 
                              << public_key[2] << "\n";
                    break;
                }
                
                case 2: {
                    std::string filename, signature_file;
                    std::cout << "Введите имя файла для проверки: ";
                    std::getline(std::cin, filename);
                    std::cout << "Введите имя файла с подписью: ";
                    std::getline(std::cin, signature_file);
                    
                    auto [public_key, signature] = elgamal.load_signature(signature_file);
                    bool is_valid = elgamal.verify_signature(filename, signature, public_key);
                    
                    if (is_valid) {
                        std::cout << "Подпись ВЕРНА!\n";
                    } else {
                        std::cout << "Подпись НЕВЕРНА!\n";
                    }
                    break;
                }
                
                case 3:
                    std::cout << "Выход из программы.\n";
                    break;
                    
                default:
                    std::cout << "Неверный выбор. Попробуйте снова.\n";
                    break;
            }
            std::cout << "\n";
            
        } while (choice != 3);
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
