#ifndef BERESTA_NODE_NAPI_SYMBOLS_HPP
#define BERESTA_NODE_NAPI_SYMBOLS_HPP

#include "addon/napi_abi.hpp"

namespace beresta::node
{
    /*
    ** «Обобщённый» указатель на функцию. Указатель на любую функцию можно
    ** привести к нему и обратно к исходному типу без потерь (это разрешено
    ** стандартом C++), поэтому таблицы хранят функции разных сигнатур в
    ** одном виде, а вызывающий код возвращает им настоящий тип.
    **
    ** A "generic" function pointer. A pointer to any function can be cast
    ** to it and back to the original type without loss (the C++ standard
    ** allows this), so the tables keep functions of different signatures in
    ** one form, and the calling code gives them back their real type.
    */
    using GenericFunction = void (*)();

    // Адрес экспортируемой функции процесса Node по имени или nullptr
    // The address of an exported function of the Node process by name, or nullptr
    GenericFunction findSymbol(const char* name) noexcept;

    // Изменяемая таблица — только для загрузчика / The mutable table — for the loader only
    NapiTable& mutableNapi() noexcept;
}

#endif
