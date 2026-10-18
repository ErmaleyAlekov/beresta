#include "utils/seeded_hash.hpp"

#include <chrono>
#include <cstdint>
#include <random>

namespace beresta::utils
{
    // Состояние SipHash: четыре 64-битных слова / The SipHash state: four 64-bit words
    struct SipState
    {
        std::uint64_t v0;
        std::uint64_t v1;
        std::uint64_t v2;
        std::uint64_t v3;
    };

    // Секретный 128-битный ключ процесса / The secret 128-bit key of the process
    struct SecretKey
    {
        std::uint64_t k0;
        std::uint64_t k1;
    };

    // Циклический сдвиг влево / A circular left shift
    static std::uint64_t rotateLeft(std::uint64_t value, int bits) noexcept
    {
        return (value << bits) | (value >> (64 - bits));
    }

    // Один раунд SipHash (ARX: сложение, поворот, исключающее ИЛИ) / One SipHash round (ARX: add, rotate, xor)
    static void sipRound(SipState& state) noexcept
    {
        state.v0 += state.v1;
        state.v1 = rotateLeft(state.v1, 13);
        state.v1 ^= state.v0;
        state.v0 = rotateLeft(state.v0, 32);
        state.v2 += state.v3;
        state.v3 = rotateLeft(state.v3, 16);
        state.v3 ^= state.v2;
        state.v0 += state.v3;
        state.v3 = rotateLeft(state.v3, 21);
        state.v3 ^= state.v0;
        state.v2 += state.v1;
        state.v1 = rotateLeft(state.v1, 17);
        state.v1 ^= state.v2;
        state.v2 = rotateLeft(state.v2, 32);
    }

    // До 8 байтов как число little-endian — одинаково на любой платформе
    // Up to 8 bytes as a little-endian number — the same on any platform
    static std::uint64_t readWord(const char* data, std::size_t size) noexcept
    {
        std::uint64_t word = 0;

        for (std::size_t index = 0; index < size && index < 8; ++index)
            word |= static_cast<std::uint64_t>(static_cast<unsigned char>(data[index])) << (8 * index);
        return word;
    }

    /*
    ** Случайный ключ: std::random_device (системный источник энтропии).
    ** Если его нет (редкие встраиваемые системы — он бросает исключение),
    ** ключ собирается из времени и адреса в стеке: слабее, но всё равно
    ** неизвестен атакующему заранее.
    **
    ** A random key: std::random_device (the system entropy source).
    ** If it is absent (rare embedded systems — it throws an exception),
    ** the key is assembled from the time and a stack address: weaker, but still
    ** unknown to an attacker in advance.
    */
    static SecretKey makeSecretKey() noexcept
    {
        std::uint64_t moment = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        SecretKey key{moment ^ 0x9e3779b97f4a7c15ULL, reinterpret_cast<std::uintptr_t>(&moment) ^ 0xc2b2ae3d27d4eb4fULL};

        try
        {
            std::random_device device;
            key.k0 ^= (static_cast<std::uint64_t>(device()) << 32) ^ device();
            key.k1 ^= (static_cast<std::uint64_t>(device()) << 32) ^ device();
        }
        catch (...)
        {
            key.k1 = rotateLeft(key.k1 * 0xff51afd7ed558ccdULL, 29);
        }
        return key;
    }

    /*
    ** SipHash-1-3: по одному раунду на каждые 8 байтов, последний блок
    ** несёт длину строки, затем три завершающих раунда. Ключ создаётся
    ** при первом вызове (потокобезопасная статическая переменная C++11).
    **
    ** SipHash-1-3: one round per every 8 bytes, the last block
    ** carries the string length, then three finalization rounds. The key is created
    ** on the first call (a thread-safe C++11 static variable).
    */
    std::size_t SeededHash::operator()(std::string_view text) const noexcept
    {
        static const SecretKey key = makeSecretKey();
        SipState state{key.k0 ^ 0x736f6d6570736575ULL, key.k1 ^ 0x646f72616e646f6dULL,
            key.k0 ^ 0x6c7967656e657261ULL, key.k1 ^ 0x7465646279746573ULL};
        std::size_t offset = 0;

        for (; offset + 8 <= text.size(); offset += 8)
        {
            std::uint64_t word = readWord(text.data() + offset, 8);
            state.v3 ^= word;
            sipRound(state);
            state.v0 ^= word;
        }
        std::uint64_t last = readWord(text.data() + offset, text.size() - offset)
            | (static_cast<std::uint64_t>(text.size() & 0xff) << 56);
        state.v3 ^= last;
        sipRound(state);
        state.v0 ^= last;
        state.v2 ^= 0xff;
        sipRound(state);
        sipRound(state);
        sipRound(state);
        return static_cast<std::size_t>(state.v0 ^ state.v1 ^ state.v2 ^ state.v3);
    }
}
