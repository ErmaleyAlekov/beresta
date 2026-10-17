#ifndef DOM_EXCEPTION_HPP
#define DOM_EXCEPTION_HPP

#include <exception>
#include <string>

namespace beresta
{
    /*
    ** Перечисление категорий ошибок библиотеки.
    ** Код позволяет программно различать сбои, не разбирая текст сообщения.
    */
    enum class DomErrorCode
    {
        INVALID_NAME,        // Недопустимое XML-имя тега, атрибута или цели PI
        INVALID_CONTENT,     // Недопустимое содержимое (например, "--" в комментарии)
        HIERARCHY_VIOLATION, // Нарушение структуры дерева (цикл, второй корень и т.п.)
        INDEX_OUT_OF_RANGE,  // Обращение к дочернему узлу по несуществующему индексу
        NULL_NODE,           // Передан пустой указатель вместо узла
        PARSE_ERROR,         // Синтаксическая ошибка при разборе XML-текста
        NOT_FOUND,           // Запрошенный узел или объект не найден
        NAMESPACE_ERROR,     // Нарушение правил пространств имён (необъявленный префикс и т.п.)
        IO_ERROR,            // Ошибка чтения или записи файла / потока
        ENCODING_ERROR,      // Неизвестная кодировка или некорректная последовательность байтов
        VALIDATION_ERROR,    // Документ не соответствует DTD или XML Schema
        XPATH_ERROR,         // Синтаксическая или вычислительная ошибка XPath
        SCHEMA_ERROR,        // Некорректная или неподдерживаемая XML Schema
        RESOURCE_ERROR,      // Внешний ресурс (DTD, сущность, XInclude) не удалось загрузить
        LIMIT_EXCEEDED       // Превышен защитный лимит (глубина, раскрытие сущностей)
    };

    // Количество кодов ошибок (для таблицы имён)
    #define BERESTA_ERROR_CODE_COUNT 15

    /*
    ** Базовое исключение библиотеки.
    ** Хранит код ошибки и человекочитаемое сообщение. Метод what()
    ** возвращает заранее собранную строку вида "[КОД] сообщение",
    ** чтобы не выделять память в момент обработки исключения.
    */
    class DomException : public std::exception
    {
    public:
        DomException(DomErrorCode code, std::string message);

        // Возвращает категорию ошибки
        DomErrorCode getCode() const noexcept;

        // Возвращает «чистое» сообщение без префикса с кодом
        const std::string& getMessage() const noexcept;

        // Возвращает полное сообщение "[КОД] сообщение"
        const char* what() const noexcept override;

    private:
        DomErrorCode _code;
        std::string _message;
        std::string _fullText;
    };

    // Преобразует код ошибки в его текстовое имя (например, "PARSE_ERROR")
    const char* errorCodeToString(DomErrorCode code) noexcept;
}

#endif
