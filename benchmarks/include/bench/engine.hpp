#ifndef BERESTA_BENCH_ENGINE_HPP
#define BERESTA_BENCH_ENGINE_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bench
{
    /*
    ** Собственное исключение программы замеров: сбой самой
    ** инфраструктуры (нет памяти под данные, неверные аргументы,
    ** библиотека отвергла корректный документ). Отделено от исключений
    ** сравниваемых библиотек, чтобы ошибку окружения не спутать с результатом.
    **
    ** The benchmark's own exception: a failure of the infrastructure itself
    ** (no memory for the data, invalid arguments, a library rejected a valid
    ** document). It is separate from the exceptions of the compared libraries
    ** so that an environment error is not confused with a result.
    */
    class BenchError : public std::exception
    {
    public:
        explicit BenchError(std::string message);
        const char* what() const noexcept override;

    private:
        std::string _message;
    };

    /*
    ** XmlEngine — одна сравниваемая библиотека (паттерн Strategy). Все
    ** библиотеки получают одинаковые операции над одними и теми же данными:
    **   parse          — текст -> дерево DOM (дерево хранится в движке);
    **   traverse       — обход всего дерева с подсчётом элементов, атрибутов
    **                    и текстовых узлов (контрольная сумма сверяется между
    **                    библиотеками: так видно, что все построили одно дерево);
    **   serialize      — дерево -> текст без отступов; результат — число байтов;
    **   xpathCount     — число узлов результата выражения XPath 1.0;
    **   parseValidated — разбор с проверкой по XML Schema; результат — число ошибок;
    **   streamValidated — потоковая проверка по схеме без дерева (SAX); результат — число ошибок.
    ** Необязательные операции возвращают nullopt, если библиотека их не умеет.
    **
    ** XmlEngine — one compared library (the Strategy pattern). All libraries
    ** get the same operations on the same data:
    **   parse          — text -> a DOM tree (the tree is kept in the engine);
    **   traverse       — a walk over the whole tree counting elements, attributes
    **                    and text nodes (the checksum is compared between the
    **                    libraries: this shows that all of them built the same tree);
    **   serialize      — tree -> text without indentation; the result is the byte count;
    **   xpathCount     — the number of nodes in the result of an XPath 1.0 expression;
    **   parseValidated — parsing with XML Schema validation; the result is the error count;
    **   streamValidated — streaming schema validation without a tree (SAX); the result is the error count.
    ** Optional operations return nullopt if the library cannot do them.
    */
    class XmlEngine
    {
    public:
        virtual ~XmlEngine();

        // Название и версия библиотеки / The library name and version
        virtual std::string name() const = 0;

        // Разбор в DOM; предыдущее дерево освобождается / Parsing into DOM; the previous tree is freed
        virtual void parse(const std::string& xml) = 0;

        // Контрольная сумма обхода текущего дерева / The walk checksum of the current tree
        virtual std::size_t traverse() const = 0;

        // Освобождение текущего дерева / Freeing the current tree
        virtual void release() = 0;

        // Размер сериализованного текста / The size of the serialized text
        virtual std::optional<std::size_t> serialize() const;

        // Число узлов результата XPath / The number of nodes in an XPath result
        virtual std::optional<std::size_t> xpathCount(const std::string& query) const;

        // Разбор с проверкой по схеме; число ошибок валидности / Parsing with schema validation; the validity error count
        virtual std::optional<std::size_t> parseValidated(const std::string& xml, const std::string& xsd);

        // Проверка по схеме потоком событий, без дерева; число ошибок / Validation by an event stream, without a tree; the error count
        virtual std::optional<std::size_t> streamValidated(const std::string& xml, const std::string& xsd);
    };

    // Все библиотеки, с которыми собрана программа (registry.cpp) / All libraries the program is built with
    std::vector<std::unique_ptr<XmlEngine>> createEngines();
}

#endif
