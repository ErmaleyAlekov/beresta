#ifndef CAPI_SUPPORT_HPP
#define CAPI_SUPPORT_HPP

#include "capi/beresta_c.h"
#include "core/container_node.hpp"
#include "core/document.hpp"
#include "core/element.hpp"
#include "core/node.hpp"
#include "parser/error_handler.hpp"
#include "parser/parser_options.hpp"
#include "serializer/xml_serializer.hpp"
#include "xpath/xpath.hpp"
#include "xsd/schema.hpp"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/*
** Внутренности Си-интерфейса: превращение дескрипторов void* в объекты
** ядра, запись «последней ошибки» и единый барьер для исключений.
** Этот заголовок не входит в публичный API и не нужен обёрткам.
*/
namespace beresta::capi
{
    /*
    ** Исключение «неверный аргумент»: NULL вместо обязательного дескриптора
    ** или строки, дескриптор не того типа (например, текстовый узел там,
    ** где нужен элемент). Превращается в код BERESTA_INVALID_ARGUMENT.
    */
    class InvalidArgumentException : public std::exception
    {
    public:
        explicit InvalidArgumentException(std::string message);

        const char* what() const noexcept override;

    private:
        std::string _message;
    };

    // Размер буфера сообщения об ошибке (с завершающим нулём)
    #define BERESTA_CAPI_MESSAGE_CAPACITY 2048

    /*
    ** Последняя ошибка потока: код beresta_status и текст сообщения.
    ** Тип НАМЕРЕННО тривиальный (массив вместо std::string): thread_local
    ** объект с деструктором внутри динамической библиотеки — источник
    ** падений при выходе из процесса (на MinGW деструкторы TLS могут
    ** сработать после выгрузки DLL). Тривиальному объекту не нужны ни
    ** конструктор, ни деструктор, ни выделение памяти при записи ошибки.
    */
    struct ErrorState
    {
        int code;
        char message[BERESTA_CAPI_MESSAGE_CAPACITY];
    };

    /*
    ** Объекты за дескрипторами, которых нет в ядре в готовом виде.
    ** Каждый дескриптор void* указывает ровно на один из этих типов
    ** (или на beresta::Node для документов и узлов).
    */
    struct XPathHandle
    {
        xpath::XPathExpression expression;
        xpath::XPathContext context;
    };

    struct NodeSetHandle
    {
        std::vector<xpath::XPathItem> items;
    };

    struct SchemaHandle
    {
        std::shared_ptr<const xsd::Schema> schema;
    };

    struct DiagnosticsHandle
    {
        std::vector<Diagnostic> items;
    };

    // ---- Состояние ошибки (capi_error_state.cpp, capi_error_record.cpp) ----

    // Ошибка текущего потока (thread_local — потоки не мешают друг другу)
    ErrorState& errorState() noexcept;

    // Сброс ошибки в начале каждого вызова Си-функции
    void beginCall() noexcept;

    // Запись перехваченного исключения; возвращает соответствующий код
    int recordException(const std::exception& error) noexcept;

    // Запись исключения неизвестного типа (catch (...))
    int recordUnknown() noexcept;

    // ---- Дескрипторы (capi_handles.cpp) ----

    // Узел за дескриптором; NULL — InvalidArgumentException
    Node& toNode(void* handle);

    // Элемент за дескриптором; узел другого типа — InvalidArgumentException
    Element& toElement(void* handle);

    // Документ за дескриптором
    Document& toDocument(void* handle);

    // Узел, который может иметь детей (элемент или документ)
    ContainerNode& toContainer(void* handle);

    // Проверка индекса int64_t из внешнего мира и перевод в size_t
    std::size_t checkedIndex(std::int64_t index, std::size_t size);

    // ---- Строки и преобразования (capi_library.cpp, capi_options.cpp) ----

    // Копия строки в памяти malloc: освобождается beresta_string_free()
    char* duplicateString(std::string_view text);

    // Обязательная строка-аргумент: NULL — InvalidArgumentException
    std::string_view requireText(const char* text, const char* what);

    // Путь из UTF-8 (на Windows std::string трактовался бы как ANSI)
    std::filesystem::path pathFromUtf8(const char* path);

    // Настройки парсера из битовой маски BERESTA_PARSE_*
    ParserOptions parserOptions(std::uint32_t flags, void* schema);

    // Настройки вывода из битовой маски BERESTA_WRITE_*
    SerializerOptions serializerOptions(std::uint32_t flags, const char* encoding);

    // ---- Изменение дерева (capi_node_append.cpp) ----

    // Добавление нового узла в конец детей parent; документ получает корень
    Node* appendNode(void* parent, std::unique_ptr<Node> child);

    // Документ в динамической памяти: дескриптор для внешнего мира
    beresta_document releaseDocument(Document&& document);

    // Дескриптор объекта-обёртки (XPathHandle, SchemaHandle и т.п.)
    template <typename Handle>
    Handle& requireHandle(void* handle, const char* what)
    {
        if (handle == nullptr)
            throw InvalidArgumentException(std::string(what) + ": передан NULL");
        return *static_cast<Handle*>(handle);
    }
}

/*
** Барьер исключений. Ни одно исключение C++ не должно выйти за границу
** extern "C": это неопределённое поведение, а в Java / C# / Python —
** аварийное завершение процесса. Каждая функция Си-интерфейса имеет вид
**
**     BERESTA_CAPI_TRY
**         ... работа с ядром ...
**     BERESTA_CAPI_CATCH(значение_при_ошибке)
**
** Макрос выбран вместо лямбды или функции-обёртки: он не меняет сигнатуру
** функций, не требует захвата аргументов и одинаково работает для любого
** типа результата. BERESTA_CAPI_CATCH_STATUS возвращает сам код ошибки,
** BERESTA_CAPI_CATCH_VOID — вариант для функций без результата.
*/
#define BERESTA_CAPI_TRY \
    beresta::capi::beginCall(); \
    try {

#define BERESTA_CAPI_CATCH(fallback) \
    } catch (const std::exception& capiError) { \
        beresta::capi::recordException(capiError); \
        return fallback; \
    } catch (...) { \
        beresta::capi::recordUnknown(); \
        return fallback; \
    }

#define BERESTA_CAPI_CATCH_VOID \
    } catch (const std::exception& capiError) { \
        beresta::capi::recordException(capiError); \
    } catch (...) { \
        beresta::capi::recordUnknown(); \
    }

#define BERESTA_CAPI_CATCH_STATUS \
    } catch (const std::exception& capiError) { \
        return beresta::capi::recordException(capiError); \
    } catch (...) { \
        return beresta::capi::recordUnknown(); \
    }

#endif
