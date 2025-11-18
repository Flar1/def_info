#include <boost/multiprecision/cpp_int.hpp>
#include <openssl/sha.h>

#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using boost::multiprecision::cpp_int;

namespace {

std::vector<std::uint8_t> read_file(const std::string &path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("cannot open file: " + path);
    }
    return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

void write_text(const std::string &path, const std::string &content) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("cannot write file: " + path);
    }
    ofs << content;
}

std::string trim(const std::string &s) {
    auto begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

cpp_int from_hex(const std::string &hex) {
    std::string s = hex;
    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
        s.erase(0, 2);
    }
    if (s.empty()) {
        return cpp_int(0);
    }
    cpp_int v = 0;
    for (char c : s) {
        v <<= 4;
        if (c >= '0' && c <= '9') {
            v += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            v += c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            v += c - 'A' + 10;
        } else {
            throw std::runtime_error("invalid hex digit");
        }
    }
    return v;
}

std::string to_hex(const cpp_int &value) {
    if (value == 0) {
        return "0";
    }
    if (value < 0) {
        throw std::runtime_error("negative value to_hex");
    }
    std::vector<unsigned char> bytes;
    boost::multiprecision::export_bits(value, std::back_inserter(bytes), 8);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto b : bytes) {
        oss << std::setw(2) << static_cast<unsigned int>(b);
    }
    return oss.str();
}

cpp_int mod_pow(cpp_int base, cpp_int exp, const cpp_int &mod) {
    base %= mod;
    if (base < 0) base += mod;
    cpp_int result = 1;
    while (exp > 0) {
        if ((exp & 1) != 0) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

cpp_int extended_gcd(const cpp_int &a, const cpp_int &b, cpp_int &x, cpp_int &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    cpp_int x1{}, y1{};
    cpp_int g = extended_gcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

cpp_int mod_inverse(const cpp_int &a, const cpp_int &mod) {
    cpp_int x{}, y{};
    cpp_int g = extended_gcd(a % mod + mod, mod, x, y);
    if (g != 1) {
        throw std::runtime_error("mod inverse does not exist");
    }
    x %= mod;
    if (x < 0) x += mod;
    return x;
}

std::vector<std::uint8_t> sha256(const std::vector<std::uint8_t> &data) {
    std::vector<std::uint8_t> digest(SHA256_DIGEST_LENGTH);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    if (!data.empty()) {
        SHA256_Update(&ctx, data.data(), data.size());
    }
    SHA256_Final(digest.data(), &ctx);
    return digest;
}

cpp_int bytes_to_int(const std::vector<std::uint8_t> &bytes) {
    cpp_int value = 0;
    for (auto b : bytes) {
        value <<= 8;
        value += b;
    }
    return value;
}

cpp_int random_range(std::mt19937_64 &rng, const cpp_int &min, const cpp_int &max) {
    if (min > max) {
        throw std::runtime_error("random_range invalid bounds");
    }
    cpp_int range = max - min + 1;
    std::size_t bits = boost::multiprecision::msb(range) + 1;
    cpp_int candidate;
    std::uniform_int_distribution<std::uint64_t> dist(0, std::numeric_limits<std::uint64_t>::max());
    do {
        cpp_int value = 0;
        std::size_t produced = 0;
        while (produced < bits) {
            cpp_int chunk = dist(rng);
            std::size_t take = std::min<std::size_t>(64, bits - produced);
            chunk &= (cpp_int(1) << take) - 1;
            value <<= take;
            value |= chunk;
            produced += take;
        }
        candidate = value;
    } while (candidate >= range);
    return min + candidate;
}

bool is_probable_prime(const cpp_int &n, std::mt19937_64 &rng, int rounds = 32) {
    if (n < 2) return false;
    static const int small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    for (int p : small_primes) {
        if (n == p) return true;
        if (n % p == 0) return false;
    }
    cpp_int d = n - 1;
    unsigned int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }
    for (int i = 0; i < rounds; ++i) {
        cpp_int a = random_range(rng, 2, n - 2);
        cpp_int x = mod_pow(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool cont = false;
        for (unsigned int r = 1; r < s; ++r) {
            x = mod_pow(x, 2, n);
            if (x == n - 1) {
                cont = true;
                break;
            }
        }
        if (cont) continue;
        return false;
    }
    return true;
}

cpp_int generate_prime(std::mt19937_64 &rng, std::size_t bits) {
    if (bits < 2) throw std::runtime_error("prime bits too small");
    std::uniform_int_distribution<std::uint64_t> dist(0, std::numeric_limits<std::uint64_t>::max());
    while (true) {
        cpp_int value = 0;
        std::size_t produced = 0;
        while (produced < bits) {
            cpp_int chunk = dist(rng);
            std::size_t take = std::min<std::size_t>(64, bits - produced);
            chunk &= (cpp_int(1) << take) - 1;
            value <<= take;
            value |= chunk;
            produced += take;
        }
        value |= cpp_int(1) << (bits - 1);
        value |= 1; // odd
        if (is_probable_prime(value, rng)) {
            return value;
        }
    }
}

struct GostParams {
    cpp_int p;
    cpp_int q;
    cpp_int a;
};

struct GostPrivateKey {
    GostParams params;
    cpp_int x;
    cpp_int y;
};

struct GostPublicKey {
    GostParams params;
    cpp_int y;
};

GostParams generate_params(std::mt19937_64 &rng, std::size_t p_bits = 512, std::size_t q_bits = 160) {
    cpp_int q = generate_prime(rng, q_bits);
    cpp_int p;
    while (true) {
        cpp_int k = random_range(rng, cpp_int(2), cpp_int(1) << (p_bits - q_bits));
        p = k * q + 1;
        if (boost::multiprecision::msb(p) + 1 != p_bits) continue;
        if (is_probable_prime(p, rng)) break;
    }
    cpp_int exponent = (p - 1) / q;
    cpp_int a;
    while (true) {
        cpp_int g = random_range(rng, 2, p - 2);
        a = mod_pow(g, exponent, p);
        if (a != 1) break;
    }
    return {p, q, a};
}

std::vector<std::uint8_t> hash_data(const std::vector<std::uint8_t> &data) {
    return sha256(data);
}

cpp_int hash_mod_q(const std::vector<std::uint8_t> &data, const cpp_int &q) {
    cpp_int h = bytes_to_int(hash_data(data)) % q;
    if (h == 0) h = 1;
    return h;
}

GostPrivateKey generate_private_key(std::mt19937_64 &rng) {
    GostParams params = generate_params(rng);
    cpp_int x = random_range(rng, 1, params.q - 1);
    cpp_int y = mod_pow(params.a, x, params.p);
    return {params, x, y};
}

std::pair<cpp_int, cpp_int> sign_message(const std::vector<std::uint8_t> &data,
                                         const GostPrivateKey &key,
                                         std::mt19937_64 &rng) {
    cpp_int h = hash_mod_q(data, key.params.q);
    while (true) {
        cpp_int k = random_range(rng, 1, key.params.q - 1);
        cpp_int r = mod_pow(key.params.a, k, key.params.p) % key.params.q;
        if (r == 0) continue;
        cpp_int s = (k * h + key.x * r) % key.params.q;
        if (s == 0) continue;
        return {r, s};
    }
}

bool verify_signature(const std::vector<std::uint8_t> &data,
                      const GostPublicKey &key,
                      const cpp_int &r,
                      const cpp_int &s) {
    if (r <= 0 || r >= key.params.q || s <= 0 || s >= key.params.q) return false;
    cpp_int h = hash_mod_q(data, key.params.q);
    cpp_int v;
    try {
        v = mod_inverse(h, key.params.q);
    } catch (...) {
        return false;
    }
    cpp_int z1 = (s * v) % key.params.q;
    cpp_int z2 = ((key.params.q - r) * v) % key.params.q;
    cpp_int u = (mod_pow(key.params.a, z1, key.params.p) *
                 mod_pow(key.y, z2, key.params.p)) % key.params.p;
    u %= key.params.q;
    return u == r;
}

std::string serialize_private(const GostPrivateKey &key) {
    std::ostringstream oss;
    oss << "p=" << to_hex(key.params.p) << "\n"
        << "q=" << to_hex(key.params.q) << "\n"
        << "a=" << to_hex(key.params.a) << "\n"
        << "x=" << to_hex(key.x) << "\n"
        << "y=" << to_hex(key.y) << "\n";
    return oss.str();
}

std::string serialize_public(const GostPublicKey &key) {
    std::ostringstream oss;
    oss << "p=" << to_hex(key.params.p) << "\n"
        << "q=" << to_hex(key.params.q) << "\n"
        << "a=" << to_hex(key.params.a) << "\n"
        << "y=" << to_hex(key.y) << "\n";
    return oss.str();
}

std::vector<std::string> split_lines(const std::string &text) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(trim(line));
    }
    return lines;
}

GostPrivateKey parse_private(const std::string &text) {
    GostPrivateKey key;
    bool has_p = false, has_q = false, has_a = false, has_x = false, has_y = false;
    for (const auto &line : split_lines(text)) {
        if (line.empty()) continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) throw std::runtime_error("invalid private key line");
        auto field = trim(line.substr(0, pos));
        auto value = trim(line.substr(pos + 1));
        if (field == "p") {
            key.params.p = from_hex(value);
            has_p = true;
        } else if (field == "q") {
            key.params.q = from_hex(value);
            has_q = true;
        } else if (field == "a") {
            key.params.a = from_hex(value);
            has_a = true;
        } else if (field == "x") {
            key.x = from_hex(value);
            has_x = true;
        } else if (field == "y") {
            key.y = from_hex(value);
            has_y = true;
        } else {
            throw std::runtime_error("unknown private key field");
        }
    }
    if (!(has_p && has_q && has_a && has_x && has_y)) {
        throw std::runtime_error("incomplete private key");
    }
    return key;
}

GostPublicKey parse_public(const std::string &text) {
    GostPublicKey key;
    bool has_p = false, has_q = false, has_a = false, has_y = false;
    for (const auto &line : split_lines(text)) {
        if (line.empty()) continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) throw std::runtime_error("invalid public key line");
        auto field = trim(line.substr(0, pos));
        auto value = trim(line.substr(pos + 1));
        if (field == "p") {
            key.params.p = from_hex(value);
            has_p = true;
        } else if (field == "q") {
            key.params.q = from_hex(value);
            has_q = true;
        } else if (field == "a") {
            key.params.a = from_hex(value);
            has_a = true;
        } else if (field == "y") {
            key.y = from_hex(value);
            has_y = true;
        } else {
            throw std::runtime_error("unknown public key field");
        }
    }
    if (!(has_p && has_q && has_a && has_y)) {
        throw std::runtime_error("incomplete public key");
    }
    return key;
}

void print_usage() {
    std::cout << "Usage:\n"
              << "  gost94 keygen <private_key> <public_key>\n"
              << "  gost94 sign <private_key> <input_file> <signature_file>\n"
              << "  gost94 verify <public_key> <input_file> <signature_file>\n";
}

} // namespace

int main(int argc, char *argv[]) {
    try {
        if (argc < 2) {
            print_usage();
            return 0;
        }

        std::string command = argv[1];
        std::random_device rd;
        std::mt19937_64 rng(rd() ^ (static_cast<std::uint64_t>(
                        std::chrono::high_resolution_clock::now().time_since_epoch().count())));

        if (command == "keygen") {
            if (argc != 4) {
                print_usage();
                return 1;
            }
            auto priv = generate_private_key(rng);
            GostPublicKey pub{priv.params, priv.y};
            write_text(argv[2], serialize_private(priv));
            write_text(argv[3], serialize_public(pub));
            std::cout << "keys generated\n";
        } else if (command == "sign") {
            if (argc != 5) {
                print_usage();
                return 1;
            }
            auto priv_bytes = read_file(argv[2]);
            std::string priv_text(priv_bytes.begin(), priv_bytes.end());
            auto priv = parse_private(priv_text);
            auto message = read_file(argv[3]);
            auto [r, s] = sign_message(message, priv, rng);
            std::ostringstream oss;
            oss << to_hex(r) << ":" << to_hex(s) << "\n";
            write_text(argv[4], oss.str());
            std::cout << "signature written\n";
        } else if (command == "verify") {
            if (argc != 5) {
                print_usage();
                return 1;
            }
            auto pub_bytes = read_file(argv[2]);
            std::string pub_text(pub_bytes.begin(), pub_bytes.end());
            auto pub = parse_public(pub_text);
            auto message = read_file(argv[3]);
            auto sig_bytes = read_file(argv[4]);
            std::string sig_text = trim(std::string(sig_bytes.begin(), sig_bytes.end()));
            auto pos = sig_text.find(':');
            if (pos == std::string::npos) {
                throw std::runtime_error("invalid signature format");
            }
            cpp_int r = from_hex(sig_text.substr(0, pos));
            cpp_int s = from_hex(sig_text.substr(pos + 1));
            if (verify_signature(message, pub, r, s)) {
                std::cout << "signature is valid\n";
                return 0;
            }
            std::cout << "signature is INVALID\n";
            return 2;
        } else {
            print_usage();
            return 1;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}

