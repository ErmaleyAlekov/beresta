#include "support/schema_helpers.hpp"

namespace support
{
    // Обработчик SAX, которому события не нужны: проверка идёт в валидаторе-декораторе
    // A SAX handler that does not need the events: the validation happens in the decorator validator
    class SilentHandler : public beresta::ContentHandler
    {
    };

    /*
    ** Три прохода над одним документом. Схема компилируется один раз;
    ** ошибки разбора и схемы собираются CollectingErrorHandler. Для
    ** проверки дерева документ разбирается с позициями узлов, чтобы
    ** строки и столбцы можно было сравнить с потоковыми режимами.
    **
    ** Three passes over one document. The schema is compiled once;
    ** parse and schema errors are collected by CollectingErrorHandler. For
    ** the tree check the document is parsed with node positions so that
    ** the lines and columns can be compared with the streaming modes.
    */
    SchemaRuns runSchemaModes(const std::string& xsd, const std::string& xml, beresta::ParserOptions options)
    {
        SchemaRuns runs;
        SilentHandler silent;
        beresta::CollectingErrorHandler saxErrors;
        beresta::CollectingErrorHandler parseErrors;
        beresta::ParserOptions plain = options;

        options.schema = beresta::xsd::Schema::fromString(xsd);
        options.errorHandler = &saxErrors;
        beresta::SaxParser(options).parse(xml, silent);
        runs.sax = saxErrors.getDiagnostics();
        options.errorHandler = &parseErrors;
        beresta::XmlParser(options).parse(xml);
        runs.parsed = parseErrors.getDiagnostics();
        plain.trackLocations = true;
        runs.tree = beresta::xsd::SchemaValidator(options.schema).validate(beresta::XmlParser(plain).parse(xml));
        return runs;
    }

    /*
    ** Тексты равны с точностью до приставки «строка N, столбец M: », которую
    ** ParseException добавляет к сообщениям из ErrorHandler (у списка
    ** SchemaValidator::validate её нет).
    **
    ** The texts are equal up to the "line N, column M: " prefix that
    ** ParseException adds to messages from an ErrorHandler (the list of
    ** SchemaValidator::validate does not have it).
    */
    static bool sameText(const std::string& left, const std::string& right)
    {
        if (left == right)
            return true;
        if (left.size() < right.size())
            return sameText(right, left);
        return left.size() > right.size() + 2
            && left.compare(left.size() - right.size() - 2, std::string::npos, ": " + right) == 0;
    }

    // Сравнение по тексту и позиции (смещение в сообщениях схемы не заполняется)
    // A comparison by the text and the position (the offset is not filled in schema messages)
    bool sameDiagnostics(const std::vector<beresta::Diagnostic>& left,
        const std::vector<beresta::Diagnostic>& right)
    {
        if (left.size() != right.size())
            return false;
        for (std::size_t index = 0; index < left.size(); ++index)
        {
            const beresta::Diagnostic& a = left[index];
            const beresta::Diagnostic& b = right[index];
            if (!sameText(a.message, b.message) || a.position.line != b.position.line
                || a.position.column != b.position.column)
                return false;
        }
        return true;
    }

    // "строка:столбец текст" через перевод строки
    // "line:column text" separated by line feeds
    std::string describeDiagnostics(const std::vector<beresta::Diagnostic>& diagnostics)
    {
        std::string text;

        for (const beresta::Diagnostic& diagnostic : diagnostics)
            text += std::to_string(diagnostic.position.line) + ":" + std::to_string(diagnostic.position.column)
                + " " + diagnostic.message + "\n";
        return text;
    }
}
