#ifndef PARSER_OPTIONS_HPP
#define PARSER_OPTIONS_HPP

#include <cstddef>
#include <memory>
#include <string>

namespace beresta
{
    class ErrorHandler;
    class ResourceResolver;

    namespace xsd
    {
        class Schema;
    }

    /*
    ** Настройки парсера (объект параметров вместо длинного списка аргументов
    ** и вместо строковых «фич» DOMConfiguration в Xerces — опечатку в имени
    ** поля здесь поймает компилятор).
    **
    ** Указатели errorHandler и resolver — НЕвладеющие: объекты должны жить,
    ** пока идёт разбор. schema — разделяемая неизменяемая схема.
    **
    ** Parser settings (a parameter object instead of a long argument list
    ** and instead of the string "features" of DOMConfiguration in Xerces — a typo
    ** in a field name is caught by the compiler here).
    ** The errorHandler and resolver pointers are NON-owning: the objects must live
    ** while parsing goes on. schema is a shared immutable schema.
    */
    struct ParserOptions
    {
        // ---- Текст и форматирование ----
        // ---- Text and formatting ----

        // Сохранять текстовые узлы из одних пробелов (отступы между тегами)
        // Keep text nodes of whitespace only (indentation between tags)
        bool preserveWhitespaceText = false;

        // Отбрасывать комментарии при построении DOM
        // Discard comments when building the DOM
        bool discardComments = false;

        // Превращать CDATA-секции в обычный текст (сливая с соседним текстом)
        // Turn CDATA sections into ordinary text (merging with adjacent text)
        bool coalesceCData = false;

        // Записывать строку и столбец каждого узла (Node::getSourceLocation)
        // Record the line and column of every node (Node::getSourceLocation)
        bool trackLocations = false;

        // ---- Пространства имён ----
        // ---- Namespaces ----

        // Проверять правила Namespaces in XML (необъявленный префикс — ошибка)
        // Check the Namespaces in XML rules (an undeclared prefix is an error)
        bool namespaceAware = true;

        // ---- DTD ----

        // Проверять документ по DTD (ошибки валидности — в errorHandler)
        // Validate the document against the DTD (validity errors go to errorHandler)
        bool validateDtd = false;

        // Загружать внешний DTD (DOCTYPE ... SYSTEM "file.dtd") через resolver
        // Load the external DTD (DOCTYPE ... SYSTEM "file.dtd") through resolver
        bool loadExternalDtd = false;

        // Добавлять атрибуты со значениями по умолчанию из <!ATTLIST>
        // Add attributes with default values from <!ATTLIST>
        bool applyDtdDefaults = true;

        // ---- XInclude ----

        // Обрабатывать <xi:include> после разбора (нужен resolver)
        // Process <xi:include> after parsing (needs resolver)
        bool processXInclude = false;

        // ---- Защитные лимиты (против «billion laughs» и глубокого ввода) ----
        // ---- Protective limits (against "billion laughs" and deep input) ----

        // Максимальная вложенность элементов
        // The maximum element nesting
        std::size_t maxDepth = 1024;

        // Максимальное число раскрытий сущностей за документ
        // The maximum number of entity expansions per document
        std::size_t maxEntityExpansions = 10000;

        // Максимальная вложенность раскрытия сущностей друг в друга
        // The maximum nesting of entity expansions into each other
        std::size_t maxEntityDepth = 16;

        // Максимальный суммарный объём текста, полученного раскрытием сущностей (байт)
        // The maximum total size of text produced by entity expansion (bytes)
        std::size_t maxExpandedSize = 10 * 1024 * 1024;

        // ---- Кодировка и ресурсы ----
        // ---- Encoding and resources ----

        // Принудительная кодировка входа (пусто — автоопределение по BOM и декларации)
        // A forced input encoding (empty — auto-detection by the BOM and the declaration)
        std::string forcedEncoding;

        // Базовый URI документа для относительных ссылок (parseFile задаёт его сам)
        // The document base URI for relative references (parseFile sets it itself)
        std::string baseUri;

        // Загрузчик внешних ресурсов; nullptr — внешние ресурсы не загружаются
        // The external resource loader; nullptr — external resources are not loaded
        ResourceResolver* resolver = nullptr;

        // Получатель ошибок и предупреждений; nullptr — ошибки валидности бросаются сразу
        // The receiver of errors and warnings; nullptr — validity errors are thrown at once
        ErrorHandler* errorHandler = nullptr;

        // XML Schema для проверки после разбора (nullptr — без проверки)
        // An XML Schema for validation after parsing (nullptr — no validation)
        std::shared_ptr<const xsd::Schema> schema;
    };
}

#endif
