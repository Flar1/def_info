#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <openssl/sha.h>
#include <iomanip>

using namespace std;

class RSA {
public:
    uint64_t p, q, n, phi, e, d;

    RSA() {
        generateKeys();
    }

    static uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod) {
        uint64_t res = 1 % mod;
        base %= mod;
        while (exp) {
            if (exp & 1)
                res = (__uint128_t)res * base % mod;
            base = (__uint128_t)base * base % mod;
            exp >>= 1;
        }
        return res;
    }

    static uint64_t gcd(uint64_t a, uint64_t b) {
        while (b) {
            uint64_t t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    static int64_t egcd(int64_t a, int64_t b, int64_t &x, int64_t &y) {
        if (b == 0) { x = 1; y = 0; return a; }
        int64_t x1, y1;
        int64_t g = egcd(b, a % b, x1, y1);
        x = y1;
        y = x1 - (a / b) * y1;
        return g;
    }

    static uint64_t modInverse(uint64_t a, uint64_t m) {
        int64_t x, y;
        int64_t g = egcd(a, m, x, y);
        if (g != 1) return 0;
        int64_t res = x % (int64_t)m;
        if (res < 0) res += m;
        return (uint64_t)res;
    }

    static bool isPrime(uint64_t n) {
        if (n < 2) return false;
        for (uint64_t i = 2; i * i <= n; ++i)
            if (n % i == 0)
                return false;
        return true;
    }

    static uint64_t randomPrime(uint64_t low = 1000, uint64_t high = 10000) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<uint64_t> dist(low, high);
        while (true) {
            uint64_t x = dist(gen);
            if (isPrime(x)) return x;
        }
    }

    void generateKeys() {
        p = randomPrime();
        q = randomPrime();
        n = p * q;
        phi = (p - 1) * (q - 1);
        e = 3;
        while (gcd(e, phi) != 1)
            e += 2;
        d = modInverse(e, phi);
    }

    uint64_t signByte(uint8_t b) const {
        return modPow(b, d, n);
    }

    uint8_t verifyByte(uint64_t s) const {
        return (uint8_t)modPow(s, e, n);
    }

    void saveKeys(const string &pubFile, const string &privFile) const {
        ofstream pub(pubFile);
        pub << e << " " << n;
        ofstream priv(privFile);
        priv << d << " " << n;
    }

    void loadPublic(const string &file) {
        ifstream in(file);
        in >> e >> n;
    }

    void loadPrivate(const string &file) {
        ifstream in(file);
        in >> d >> n;
    }
};

vector<uint8_t> sha256(const string &filename) {
    ifstream file(filename, ios::binary);
    if (!file) throw runtime_error("Не удалось открыть файл для хеша");

    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    vector<unsigned char> buf(4096);
    while (file) {
        file.read((char*)buf.data(), buf.size());
        SHA256_Update(&ctx, buf.data(), file.gcount());
    }

    vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
    SHA256_Final(hash.data(), &ctx);
    return hash;
}

void signFile(const string &infile, const string &sigfile, const RSA &rsa) {
    auto hash = sha256(infile);
    ofstream out(sigfile, ios::binary);
    for (auto b : hash) {
        uint64_t s = rsa.signByte(b);
        out.write((char*)&s, sizeof(s));
    }
    cout << "Подпись сохранена в " << sigfile << endl;
}

bool verifyFile(const string &infile, const string &sigfile, const RSA &rsa) {
    auto hash = sha256(infile);
    ifstream in(sigfile, ios::binary);
    if (!in) throw runtime_error("Не удалось открыть подпись");

    for (size_t i = 0; i < hash.size(); ++i) {
        uint64_t s;
        in.read((char*)&s, sizeof(s));
        if (!in) return false;
        uint8_t orig = rsa.verifyByte(s);
        if (orig != hash[i]) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Использование:\n"
             << "  rsa_sign gen                      — создать ключи\n"
             << "  rsa_sign sign <файл> <sig>        — подписать файл\n"
             << "  rsa_sign verify <файл> <sig>      — проверить подпись\n";
        return 0;
    }

    string cmd = argv[1];
    RSA rsa;

    try {
        if (cmd == "gen") {
            rsa.generateKeys();
            rsa.saveKeys("public.key", "private.key");
            cout << "Ключи сохранены:\n public.key (e,n)\n private.key (d,n)\n";
            cout << "p=" << rsa.p << " q=" << rsa.q << " n=" << rsa.n << "\n";
        } else if (cmd == "sign" && argc == 4) {
            rsa.loadPrivate("private.key");
            signFile(argv[2], argv[3], rsa);
        } else if (cmd == "verify" && argc == 4) {
            rsa.loadPublic("public.key");
            bool ok = verifyFile(argv[2], argv[3], rsa);
            cout << (ok ? "Подпись ВЕРНА ✅" : "Подпись НЕВЕРНА ❌") << endl;
        } else {
            cerr << "Неверные аргументы\n";
        }
    } catch (exception &e) {
        cerr << "Ошибка: " << e.what() << endl;
    }

    return 0;
}
