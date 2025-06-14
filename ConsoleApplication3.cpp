#include "pch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <cstdlib>

using namespace std;

// Реализация RC4
class RC4 {
public:
    RC4(const vector<uint8_t>& key) {
        // Инициализация S-блока
        for (int i = 0; i < 256; ++i) {
            S[i] = static_cast<uint8_t>(i);
        }

        // Перемешивание S-блока
        int j = 0;
        for (int i = 0; i < 256; ++i) {
            j = (j + S[i] + key[i % key.size()]) % 256;
            swap(S[i], S[j]);
        }
    }

    void process(istream& in, ostream& out) {
        int i = 0, j = 0;
        char byte;
        while (in.get(byte)) {
            i = (i + 1) % 256;
            j = (j + S[i]) % 256;
            swap(S[i], S[j]);
            uint8_t k = S[(S[i] + S[j]) % 256];
            out.put(byte ^ k);
        }
    }

private:
    uint8_t S[256];
};

// Генерация файла со случайными байтами
void generate_key_file(const string& filename, size_t length) {
    ofstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Невозможно открыть файл-ключ: " + filename);
    }

    // Параметры ЛКГ
    uint64_t seed = random_device{}();
    uint64_t a = 1103515245;
    uint64_t c = 12345;
    uint64_t m = 1ULL << 31;

    for (size_t i = 0; i < length; ++i) {
        seed = (a * seed + c) % m;
        uint8_t byte = static_cast<uint8_t>(seed & 0xFF);
        file.write(reinterpret_cast<const char*>(&byte), 1);
    }
}

// Шифр Вернама (XOR файлов)
void vernam_cipher(const string& input_file,
    const string& key_file,
    const string& output_file) {
    ifstream in(input_file, ios::binary);
    ifstream key(key_file, ios::binary);
    ofstream out(output_file, ios::binary);

    if (!in) throw runtime_error("Невозможно открыть входной файл: " + input_file);
    if (!key) throw runtime_error("Невозможно открыть файл-ключ: " + key_file);
    if (!out) throw runtime_error("Невозможно открыть выходной файл: " + output_file);

    char in_byte, key_byte;
    while (in.get(in_byte)) {
        if (!key.get(key_byte)) {
            throw runtime_error("Ключ короче, чем входной файл");
        }
        out.put(in_byte ^ key_byte);
    }
}

// Шифрование RC4
void rc4_cipher(const string& input_file,
    const string& key_file,
    const string& output_file) {
    // Чтение ключа
    ifstream key_stream(key_file, ios::binary);
    if (!key_stream) {
        throw runtime_error("Невозможно открыть файл-ключ: " + key_file);
    }
    vector<uint8_t> key(
        (istreambuf_iterator<char>(key_stream)),
        istreambuf_iterator<char>()
    );

    // Обработка файла
    ifstream in(input_file, ios::binary);
    ofstream out(output_file, ios::binary);

    if (!in) throw runtime_error("Невозможно открыть входной файл: " + input_file);
    if (!out) throw runtime_error("Невозможно открыть выходной: " + output_file);

    RC4 rc4(key);
    rc4.process(in, out);
}


int main(int argc, char* argv[]) {
    setlocale(LC_CTYPE, "rus");
    try {
        if (argc < 2) {
            cout << "=== (с) Жиляев Максим. ААМ-24 ===" << endl;
            cout << "Как использовать:\n"
                << "  " << argv[0] << " --generate-key <file> <length>  Сгенерировать файл-ключ\n"
                << "  " << argv[0] << " --vernam <input> <key> <output> Шифр Вернама\n"
                << "  " << argv[0] << " --rc4 <input> <key> <output>    Шифр RC4" << endl;
            return 1;
        }

        string mode = argv[1];

        if (mode == "--generate-key" && argc == 4) {
            size_t length = strtoul(argv[3], nullptr, 10);
            generate_key_file(argv[2], length);
            cout << "Сгенерирован файл-ключ: " << argv[2] << " (" << length << " байт)\n";
        }
        else if (mode == "--vernam" && argc == 5) {
            vernam_cipher(argv[2], argv[3], argv[4]);
            cout << "Вернам: " << argv[2] << " -> " << argv[4] << "\n";
        }
        else if (mode == "--rc4" && argc == 5) {
            rc4_cipher(argv[2], argv[3], argv[4]);
            cout << "RC4: " << argv[2] << " -> " << argv[4] << "\n";
        }
        else {
            cout << "=== (с) Жиляев Максим. ААМ-24 ===" << endl;
            cout << "Как использовать:\n"
                << "  crypto --generate-key <file> <length>  Сгенерировать файл-ключ\n"
                << "  crypto --vernam <input> <key> <output> Шифр Вернама\n"
                << "  crypto --rc4 <input> <key> <output>    Шифр RC4" << endl;
            return 1;
        }
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }
    return 0;
}