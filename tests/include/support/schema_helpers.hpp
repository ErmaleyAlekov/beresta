#ifndef SCHEMA_HELPERS_HPP
#define SCHEMA_HELPERS_HPP

#include "beresta.hpp"

#include <string>
#include <vector>

namespace support
{
    /*
    ** Ошибки схемы в трёх режимах проверки одного документа: SAX без дерева
    ** (SaxParser + schema), разбор в DOM со схемой (XmlParser + schema) и
    ** проверка готового дерева (SchemaValidator::validate). Логика проверки
    ** одна, поэтому списки обязаны совпадать — тесты сверяют их целиком.
    **
    ** Schema errors in the three validation modes of one document: SAX without a tree
    ** (SaxParser + schema), parsing into a DOM with a schema (XmlParser + schema) and
    ** validation of a ready tree (SchemaValidator::validate). The validation logic
    ** is one, so the lists must be equal — the tests compare them entirely.
    */
    struct SchemaRuns
    {
        std::vector<beresta::Diagnostic> sax;
        std::vector<beresta::Diagnostic> parsed;
        std::vector<beresta::Diagnostic> tree;
    };

    // Все три прохода; options — настройки парсера (схема и обработчик ошибок ставятся здесь)
    // All three passes; options — the parser settings (the schema and the error handler are set here)
    SchemaRuns runSchemaModes(const std::string& xsd, const std::string& xml,
        beresta::ParserOptions options = beresta::ParserOptions());

    // Совпадают ли списки: тексты, строки и столбцы в том же порядке
    // Whether the lists are equal: the texts, lines and columns in the same order
    bool sameDiagnostics(const std::vector<beresta::Diagnostic>& left,
        const std::vector<beresta::Diagnostic>& right);

    // Список одной строкой для сообщения о несовпадении
    // The list as one string for a mismatch message
    std::string describeDiagnostics(const std::vector<beresta::Diagnostic>& diagnostics);

    // Ошибки, одинаковые во всех трёх режимах (иначе тест падает): список проверки дерева
    // The errors equal in all three modes (otherwise the test fails): the tree validation list
    std::vector<beresta::Diagnostic> agreedSchemaErrors(const std::string& xsd, const std::string& xml);
}

#endif
