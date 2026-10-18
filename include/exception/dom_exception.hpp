#ifndef DOM_EXCEPTION_HPP
#define DOM_EXCEPTION_HPP

#include <exception>
#include <string>

namespace beresta
{
    /*
    ** Перечисление категорий ошибок библиотеки.
    ** Код позволяет программно различать сбои, не разбирая текст сообщения.
    **
    ** The enumeration of library error categories.
    ** The code allows distinguishing failures programmatically without parsing the message text.
    */
    enum class DomErrorCode
    {
        INVALID_NAME,        // Недопустимое XML-имя тега, атрибута или цели PI
                             // An invalid XML name of a tag, an attribute or a PI target
        INVALID_CONTENT,     // Недопустимое содержимое (например, "--" в комментарии)
                             // Invalid content (for example, "--" in a comment)
        HIERARCHY_VIOLATION, // Нарушение структуры дерева (цикл, второй корень и т.п.)
                             // A tree structure violation (a cycle, a second root and so on)
        INDEX_OUT_OF_RANGE,  // Обращение к дочернему узлу по несуществующему индексу
                             // Access to a child node by a non-existent index
        NULL_NODE,           // Передан пустой указатель вместо узла / A null pointer was passed instead of a node
        PARSE_ERROR,         // Синтаксическая ошибка при разборе XML-текста / A syntax error while parsing XML text
        NOT_FOUND,           // Запрошенный узел или объект не найден / The requested node or object is not found
        NAMESPACE_ERROR,     // Нарушение правил пространств имён (необъявленный префикс и т.п.)
                             // A namespace rules violation (an undeclared prefix and so on)
        IO_ERROR,            // Ошибка чтения или записи файла / потока / A file / stream read or write error
        ENCODING_ERROR,      // Неизвестная кодировка или некорректная последовательность байтов
                             // An unknown encoding or an invalid byte sequence
        VALIDATION_ERROR,    // Документ не соответствует DTD или XML Schema
                             // The document does not conform to the DTD or XML Schema
        XPATH_ERROR,         // Синтаксическая или вычислительная ошибка XPath / An XPath syntax or evaluation error
        SCHEMA_ERROR,        // Некорректная или неподдерживаемая XML Schema / An invalid or unsupported XML Schema
        RESOURCE_ERROR,      // Внешний ресурс (DTD, сущность, XInclude) не удалось загрузить
                             // An external resource (DTD, entity, XInclude) could not be loaded
        LIMIT_EXCEEDED       // Превышен защитный лимит (глубина, раскрытие сущностей)
                             // A protective limit is exceeded (depth, entity expansion)
    };

    // Количество кодов ошибок (для таблицы имён)
    // The number of error codes (for the name table)
    #define BERESTA_ERROR_CODE_COUNT 15

    /*
    ** Базовое исключение библиотеки.
    ** Хранит код ошибки и человекочитаемое сообщение. Метод what()
    ** возвращает заранее собранную строку вида "[КОД] сообщение",
    ** чтобы не выделять память в момент обработки исключения.
    **
    ** The base exception of the library.
    ** Keeps the error code and a human-readable message. The what() method
    ** returns a pre-built string like "[CODE] message"
    ** so that no memory is allocated while the exception is handled.
    */
    class DomException : public std::exception
    {
    public:
        DomException(DomErrorCode code, std::string message);

        // Возвращает категорию ошибки
        // Returns the error category
        DomErrorCode getCode() const noexcept;

        // Возвращает «чистое» сообщение без префикса с кодом
        // Returns the "clean" message without the code prefix
        const std::string& getMessage() const noexcept;

        // Возвращает полное сообщение "[КОД] сообщение"
        // Returns the full message "[CODE] message"
        const char* what() const noexcept override;

    private:
        DomErrorCode _code;
        std::string _message;
        std::string _fullText;
    };

    // Преобразует код ошибки в его текстовое имя (например, "PARSE_ERROR")
    // Converts an error code to its text name (for example, "PARSE_ERROR")
    const char* errorCodeToString(DomErrorCode code) noexcept;
}

#endif
