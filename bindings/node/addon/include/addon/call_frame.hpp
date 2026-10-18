#ifndef BERESTA_NODE_CALL_FRAME_HPP
#define BERESTA_NODE_CALL_FRAME_HPP

#include "addon/napi_symbols.hpp"

#include <cstddef>
#include <cstdint>

namespace beresta::node
{
    /*
    ** Описание одной экспортируемой функции: имя в JavaScript, переходник
    ** (Adapter) для её сигнатуры и сама функция Си-интерфейса. Один
    ** переходник обслуживает все функции с одинаковой сигнатурой: функция
    ** приходит к нему как данные вызова (паттерн Command — «что вызвать»
    ** отделено от «как разобрать аргументы и вернуть результат»).
    **
    ** The description of one exported function: the name in JavaScript, the
    ** adapter for its signature and the C interface function itself. One
    ** adapter serves all functions with the same signature: the function
    ** comes to it as call data (the Command pattern — "what to call" is
    ** separated from "how to unpack the arguments and return the result").
    */
    struct Binding
    {
        const char* name;
        napi_callback adapter;
        GenericFunction target;
    };

    // Все экспортируемые функции (binding_table.cpp) / All exported functions (binding_table.cpp)
    const Binding* bindingTable(std::size_t& count) noexcept;

    /*
    ** Строковый аргумент в UTF-8 с завершающим нулём. Короткие строки (имена,
    ** префиксы) помещаются во встроенный буфер без выделения памяти, длинные
    ** (текст, XML) — в буфер malloc, который освобождает деструктор (RAII).
    ** null и undefined из JavaScript превращаются в NULL.
    **
    ** A UTF-8 string argument with a terminating zero. Short strings (names,
    ** prefixes) fit into the built-in buffer without memory allocation, long
    ** ones (text, XML) go into a malloc buffer that the destructor frees (RAII).
    ** null and undefined from JavaScript become NULL.
    */
    class ArgString
    {
    public:
        ArgString() noexcept;
        ~ArgString();
        ArgString(const ArgString&) = delete;
        ArgString& operator=(const ArgString&) = delete;

        // Чтение строки из значения JavaScript; false — нет памяти или не строка
        // Reads a string from a JavaScript value; false — out of memory or not a string
        bool load(napi_env env, napi_value value) noexcept;

        // Указатель для Си-функции (NULL, если строки нет) / The pointer for the C function (NULL if there is no string)
        const char* get() const noexcept;

    private:
        static constexpr std::size_t LOCAL_SIZE = 256;

        char _local[LOCAL_SIZE];
        char* _heap;
        const char* _pointer;
    };

    // Непрерывный участок байтов из Buffer / Uint8Array / A contiguous byte range from a Buffer / Uint8Array
    struct ByteView
    {
        const void* data;
        std::int64_t size;
    };

    /*
    ** Кадр вызова: аргументы, пришедшие из JavaScript, и описание функции.
    ** Каждый метод чтения проверяет тип аргумента; при несовпадении он сам
    ** бросает TypeError с понятным текстом и возвращает false — переходнику
    ** остаётся лишь вернуть nullptr.
    **
    ** A call frame: the arguments that came from JavaScript and the function
    ** description. Each read method checks the argument type; on a mismatch it
    ** throws a TypeError with a clear text itself and returns false — the
    ** adapter only has to return nullptr.
    */
    class CallFrame
    {
    public:
        static constexpr std::size_t MAX_ARGUMENTS = 4;

        CallFrame(napi_env env, napi_callback_info info) noexcept;

        // Удалось ли прочитать аргументы / Whether the arguments could be read
        bool isValid() const noexcept;

        // Функция Си-интерфейса, которую нужно вызвать / The C interface function to call
        GenericFunction target() const noexcept;

        // ---- Чтение аргументов (call_frame_values.cpp) / Reading arguments ----
        bool handle(std::size_t index, void*& out) noexcept;
        bool string(std::size_t index, ArgString& out) noexcept;
        bool number(std::size_t index, double& out) noexcept;
        bool integer(std::size_t index, std::int64_t& out) noexcept;
        bool flags(std::size_t index, std::uint32_t& out) noexcept;

        // Байты документа или схемы (call_frame_bytes.cpp) / The bytes of a document or a schema
        bool bytes(std::size_t index, ByteView& out) noexcept;

    private:
        // Тип аргумента; недостающий аргумент считается undefined
        // The argument type; a missing argument counts as undefined
        ValueType typeAt(std::size_t index) const noexcept;

        // TypeError «неверный аргумент»; всегда false / TypeError "invalid argument"; always false
        bool reject(std::size_t index, const char* expected) noexcept;

        napi_env _env;
        napi_value _arguments[MAX_ARGUMENTS];
        std::size_t _count;
        const Binding* _binding;
        bool _valid;
    };
}

#endif
