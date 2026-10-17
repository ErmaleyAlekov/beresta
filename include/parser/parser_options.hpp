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
    */
    struct ParserOptions
    {
        // ---- Текст и форматирование ----

        // Сохранять текстовые узлы из одних пробелов (отступы между тегами)
        bool preserveWhitespaceText = false;

        // Отбрасывать комментарии при построении DOM
        bool discardComments = false;

        // Превращать CDATA-секции в обычный текст (сливая с соседним текстом)
        bool coalesceCData = false;

        // Записывать строку и столбец каждого узла (Node::getSourceLocation)
        bool trackLocations = false;

        // ---- Пространства имён ----

        // Проверять правила Namespaces in XML (необъявленный префикс — ошибка)
        bool namespaceAware = true;

        // ---- DTD ----

        // Проверять документ по DTD (ошибки валидности — в errorHandler)
        bool validateDtd = false;

        // Загружать внешний DTD (DOCTYPE ... SYSTEM "file.dtd") через resolver
        bool loadExternalDtd = false;

        // Добавлять атрибуты со значениями по умолчанию из <!ATTLIST>
        bool applyDtdDefaults = true;

        // ---- XInclude ----

        // Обрабатывать <xi:include> после разбора (нужен resolver)
        bool processXInclude = false;

        // ---- Защитные лимиты (против «billion laughs» и глубокого ввода) ----

        // Максимальная вложенность элементов
        std::size_t maxDepth = 1024;

        // Максимальное число раскрытий сущностей за документ
        std::size_t maxEntityExpansions = 10000;

        // Максимальная вложенность раскрытия сущностей друг в друга
        std::size_t maxEntityDepth = 16;

        // Максимальный суммарный объём текста, полученного раскрытием сущностей (байт)
        std::size_t maxExpandedSize = 10 * 1024 * 1024;

        // ---- Кодировка и ресурсы ----

        // Принудительная кодировка входа (пусто — автоопределение по BOM и декларации)
        std::string forcedEncoding;

        // Базовый URI документа для относительных ссылок (parseFile задаёт его сам)
        std::string baseUri;

        // Загрузчик внешних ресурсов; nullptr — внешние ресурсы не загружаются
        ResourceResolver* resolver = nullptr;

        // Получатель ошибок и предупреждений; nullptr — ошибки валидности бросаются сразу
        ErrorHandler* errorHandler = nullptr;

        // XML Schema для проверки после разбора (nullptr — без проверки)
        std::shared_ptr<const xsd::Schema> schema;
    };
}

#endif
