#include "addon/call_frame.hpp"

#include <cstdint>

namespace beresta::node
{
    /*
    ** Дескриптор: BigInt с адресом объекта или null / undefined (NULL).
    ** BigInt выбран вместо числа: 64-битный адрес не обязан помещаться в
    ** 53 бита мантиссы double (метки указателей на ARM64), а два BigInt с
    ** одним адресом равны по ===, что удобно для сравнения узлов.
    **
    ** A handle: a BigInt with the object address or null / undefined (NULL).
    ** BigInt was chosen over a number: a 64-bit address does not have to fit
    ** into the 53 bits of a double mantissa (pointer tags on ARM64), and two
    ** BigInts with the same address are equal by ===, handy for comparing nodes.
    */
    bool CallFrame::handle(std::size_t index, void*& out) noexcept
    {
        ValueType type = typeAt(index);
        std::uint64_t address = 0;
        bool lossless = false;

        out = nullptr;
        if (type == ValueType::NULL_VALUE || type == ValueType::UNDEFINED)
            return true;
        if (type != ValueType::BIGINT)
            return reject(index, "BigInt");
        if (napi().getBigintUint64(_env, _arguments[index], &address, &lossless) != NAPI_OK || !lossless)
            return reject(index, "BigInt");
        out = reinterpret_cast<void*>(static_cast<std::uintptr_t>(address));
        return true;
    }

    // Строка или null / undefined; обязательность проверяет сам Си-интерфейс
    // A string or null / undefined; the C interface itself checks whether it is mandatory
    bool CallFrame::string(std::size_t index, ArgString& out) noexcept
    {
        ValueType type = typeAt(index);

        if (type == ValueType::NULL_VALUE || type == ValueType::UNDEFINED)
            return true;
        if (type != ValueType::STRING || !out.load(_env, _arguments[index]))
            return reject(index, "string");
        return true;
    }

    // Число double (значения переменных XPath) / A double number (XPath variable values)
    bool CallFrame::number(std::size_t index, double& out) noexcept
    {
        if (typeAt(index) != ValueType::NUMBER)
            return reject(index, "number");
        if (napi().getDouble(_env, _arguments[index], &out) != NAPI_OK)
            return reject(index, "number");
        return true;
    }

    // Целое число (индексы) / An integer (indexes)
    bool CallFrame::integer(std::size_t index, std::int64_t& out) noexcept
    {
        if (typeAt(index) != ValueType::NUMBER)
            return reject(index, "number");
        if (napi().getInt64(_env, _arguments[index], &out) != NAPI_OK)
            return reject(index, "number");
        return true;
    }

    // Битовая маска флагов; отсутствие аргумента — 0 (флаги по умолчанию)
    // A flag bit mask; a missing argument is 0 (the default flags)
    bool CallFrame::flags(std::size_t index, std::uint32_t& out) noexcept
    {
        ValueType type = typeAt(index);

        out = 0;
        if (type == ValueType::UNDEFINED)
            return true;
        if (type != ValueType::NUMBER || napi().getUint32(_env, _arguments[index], &out) != NAPI_OK)
            return reject(index, "number");
        return true;
    }
}
