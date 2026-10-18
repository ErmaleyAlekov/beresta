#ifndef CAPI_SUPPORT_HPP
#define CAPI_SUPPORT_HPP

#include "capi/beresta_c.h"
#include "i18n/i18n.hpp"
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
**
** The internals of the C interface: turning void* handles into core
** objects, recording the "last error" and the single exception barrier.
** This header is not part of the public API and is not needed by the wrappers.
*/
namespace beresta::capi
{
    /*
    ** Исключение «неверный аргумент»: NULL вместо обязательного дескриптора
    ** или строки, дескриптор не того типа (например, текстовый узел там,
    ** где нужен элемент). Превращается в код BERESTA_INVALID_ARGUMENT.
    **
    ** The "invalid argument" exception: NULL instead of a mandatory handle
    ** or string, a handle of the wrong type (for example, a text node where
    ** an element is needed). It becomes the BERESTA_INVALID_ARGUMENT code.
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
    // The size of the error message buffer (with the terminating null)
    #define BERESTA_CAPI_MESSAGE_CAPACITY 2048

    /*
    ** Последняя ошибка потока: код beresta_status и текст сообщения.
    ** Тип НАМЕРЕННО тривиальный (массив вместо std::string): thread_local
    ** объект с деструктором внутри динамической библиотеки — источник
    ** падений при выходе из процесса (на MinGW деструкторы TLS могут
    ** сработать после выгрузки DLL). Тривиальному объекту не нужны ни
    ** конструктор, ни деструктор, ни выделение памяти при записи ошибки.
    **
    ** The last error of the thread: the beresta_status code and the message text.
    ** The type is DELIBERATELY trivial (an array instead of std::string): a thread_local
    ** object with a destructor inside a shared library is a source of crashes
    ** at process exit (on MinGW TLS destructors may run after the DLL is
    ** unloaded). A trivial object needs neither a constructor, nor a destructor,
    ** nor a memory allocation when an error is recorded.
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
    **
    ** Objects behind handles that do not exist in the core in a ready form.
    ** Each void* handle points to exactly one of these types
    ** (or to beresta::Node for documents and nodes).
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
    // ---- Error state (capi_error_state.cpp, capi_error_record.cpp) ----

    // Ошибка текущего потока (thread_local — потоки не мешают друг другу)
    // The error of the current thread (thread_local — threads do not interfere)
    ErrorState& errorState() noexcept;

    // Сброс ошибки в начале каждого вызова Си-функции
    // Resets the error at the start of every C function call
    void beginCall() noexcept;

    // Запись перехваченного исключения; возвращает соответствующий код
    // Records a caught exception; returns the matching code
    int recordException(const std::exception& error) noexcept;

    // Запись исключения неизвестного типа (catch (...))
    // Records an exception of an unknown type (catch (...))
    int recordUnknown() noexcept;

    // ---- Дескрипторы (capi_handles.cpp) ----
    // ---- Handles (capi_handles.cpp) ----

    // Узел за дескриптором; NULL — InvalidArgumentException
    // The node behind a handle; NULL — InvalidArgumentException
    Node& toNode(void* handle);

    // Элемент за дескриптором; узел другого типа — InvalidArgumentException
    // The element behind a handle; a node of another type — InvalidArgumentException
    Element& toElement(void* handle);

    // Документ за дескриптором
    // The document behind a handle
    Document& toDocument(void* handle);

    // Узел, который может иметь детей (элемент или документ)
    // A node that can have children (an element or a document)
    ContainerNode& toContainer(void* handle);

    // Проверка индекса int64_t из внешнего мира и перевод в size_t
    // Checks an int64_t index from the outside world and converts it to size_t
    std::size_t checkedIndex(std::int64_t index, std::size_t size);

    // ---- Строки и преобразования (capi_library.cpp, capi_options.cpp) ----
    // ---- Strings and conversions (capi_library.cpp, capi_options.cpp) ----

    // Копия строки в памяти malloc: освобождается beresta_string_free()
    // A copy of a string in malloc memory: freed by beresta_string_free()
    char* duplicateString(std::string_view text);

    // Обязательная строка-аргумент: NULL — InvalidArgumentException
    // A mandatory string argument: NULL — InvalidArgumentException
    std::string_view requireText(const char* text, const char* what);

    // Путь из UTF-8 (на Windows std::string трактовался бы как ANSI)
    // A path from UTF-8 (on Windows std::string would be treated as ANSI)
    std::filesystem::path pathFromUtf8(const char* path);

    // Настройки парсера из битовой маски BERESTA_PARSE_*
    // Parser settings from the BERESTA_PARSE_* bit mask
    ParserOptions parserOptions(std::uint32_t flags, void* schema);

    // Настройки вывода из битовой маски BERESTA_WRITE_*
    // Output settings from the BERESTA_WRITE_* bit mask
    SerializerOptions serializerOptions(std::uint32_t flags, const char* encoding);

    // ---- Изменение дерева (capi_node_append.cpp) ----
    // ---- Changing the tree (capi_node_append.cpp) ----

    // Добавление нового узла в конец детей parent; документ получает корень
    // Appends a new node to the end of the children of parent; a document gets a root
    Node* appendNode(void* parent, std::unique_ptr<Node> child);

    // Документ в динамической памяти: дескриптор для внешнего мира
    // A document in dynamic memory: a handle for the outside world
    beresta_document releaseDocument(Document&& document);

    // Дескриптор объекта-обёртки (XPathHandle, SchemaHandle и т.п.)
    // The handle of a wrapper object (XPathHandle, SchemaHandle and so on)
    template <typename Handle>
    Handle& requireHandle(void* handle, const char* what)
    {
        if (handle == nullptr)
            throw InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {what}));
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
**
** The exception barrier. No C++ exception may leave the extern "C"
** boundary: it is undefined behavior, and in Java / C# / Python — a
** process crash. Every C interface function looks like
**     BERESTA_CAPI_TRY
**         ... work with the core ...
**     BERESTA_CAPI_CATCH(value_on_error)
** A macro was chosen instead of a lambda or a wrapper function: it does not change
** the function signatures, needs no argument capture and works the same for any
** result type. BERESTA_CAPI_CATCH_STATUS returns the error code itself,
** BERESTA_CAPI_CATCH_VOID is the variant for functions without a result.
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
