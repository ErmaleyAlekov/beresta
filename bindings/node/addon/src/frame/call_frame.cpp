#include "addon/call_frame.hpp"
#include "addon/errors.hpp"

namespace beresta::node
{
    /*
    ** Чтение кадра: до MAX_ARGUMENTS аргументов и данные функции — указатель
    ** на её описание Binding (его передал napi_create_function при регистрации).
    **
    ** Reading the frame: up to MAX_ARGUMENTS arguments and the function data —
    ** a pointer to its Binding description (napi_create_function passed it at registration).
    */
    CallFrame::CallFrame(napi_env env, napi_callback_info info) noexcept
        : _env(env), _arguments{}, _count(MAX_ARGUMENTS), _binding(nullptr), _valid(false)
    {
        void* data = nullptr;

        if (napi().getCbInfo(env, info, &_count, _arguments, nullptr, &data) != NAPI_OK)
            return;
        if (_count > MAX_ARGUMENTS)
            _count = MAX_ARGUMENTS;
        _binding = static_cast<const Binding*>(data);
        _valid = _binding != nullptr;
    }

    // Кадр пригоден для вызова / The frame is usable for a call
    bool CallFrame::isValid() const noexcept
    {
        return _valid;
    }

    // Функция Си-интерфейса из описания / The C interface function from the description
    GenericFunction CallFrame::target() const noexcept
    {
        return _binding->target;
    }

    // Недостающие аргументы ведут себя как undefined (как в самом JavaScript)
    // Missing arguments behave like undefined (as in JavaScript itself)
    ValueType CallFrame::typeAt(std::size_t index) const noexcept
    {
        int type = static_cast<int>(ValueType::UNDEFINED);

        if (index >= _count)
            return ValueType::UNDEFINED;
        if (napi().typeOf(_env, _arguments[index], &type) != NAPI_OK)
            return ValueType::UNDEFINED;
        return static_cast<ValueType>(type);
    }

    // Номер аргумента в сообщении — с единицы, как привыкли люди
    // The argument number in the message is one-based, as people are used to
    bool CallFrame::reject(std::size_t index, const char* expected) noexcept
    {
        throwArgumentError(_env, index + 1, _binding->name, expected);
        return false;
    }
}
