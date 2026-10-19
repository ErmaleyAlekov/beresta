#include "xsts/case_runner.hpp"

#include "beresta.hpp"

#include <exception>
#include <optional>
#include <utility>

namespace xsts
{
    namespace
    {
        // Откуда экземпляр: его файл, документы схемы группы и корень набора
        // Where the instance comes from: its file, the group schema documents and the suite root
        struct InstanceSource
        {
            const std::filesystem::path& file;
            const std::vector<std::filesystem::path>& groupDocuments;
            const std::filesystem::path& suiteRoot;
        };

        /*
        ** Настройки разбора экземпляра. Текст из одних пробелов сохраняется:
        ** для простого содержимого он значим (фасеты length, pattern, enumeration).
        ** Внешний DTD загружается — некоторые экземпляры объявляют в нём сущности.
        **
        ** The instance parse settings. Whitespace-only text is kept:
        ** it matters for simple content (the length, pattern, enumeration facets).
        ** The external DTD is loaded — some instances declare entities in it.
        */
        beresta::ParserOptions instanceOptions(beresta::ResourceResolver& resolver)
        {
            beresta::ParserOptions options;

            options.preserveWhitespaceText = true;
            options.coalesceCData = true;
            options.loadExternalDtd = true;
            options.resolver = &resolver;
            return options;
        }

        // Считает ошибки проверки (предупреждения не в счёт) и запоминает первую
        // Counts the validation errors (warnings do not count) and remembers the first one
        void countErrors(const std::vector<beresta::Diagnostic>& diagnostics, InstanceAttempt& attempt)
        {
            for (const beresta::Diagnostic& diagnostic : diagnostics)
            {
                if (diagnostic.severity == beresta::ErrorSeverity::WARNING)
                    continue;
                if (attempt.errors == 0)
                    attempt.message = diagnostic.message;
                ++attempt.errors;
            }
        }

        /*
        ** Схема для проверки: у группы без schemaTest — из подсказок (не
        ** загрузилась — провал экземпляра); у остальных — схема группы,
        ** дополненная подсказками для новых пространств имён, если такие
        ** есть и дополненная схема загрузилась (иначе — схема группы).
        **
        ** The schema for validation: for a group without a schemaTest — from the hints (did not
        ** load — a failure of the instance); for the others — the group schema
        ** extended by hints for new namespaces, if there are any and the extended
        ** schema loaded (otherwise — the group schema).
        */
        void validateParsed(const beresta::Document& document, const InstanceSource& source,
            std::shared_ptr<const beresta::xsd::Schema> schema, InstanceAttempt& attempt)
        {
            SchemaAttempt hinted;

            if (schema == nullptr)
                hinted = loadHintedSchema(document, source.file, source.suiteRoot);
            else
                hinted = extendWithHints(source.groupDocuments, document, source.file, source.suiteRoot);
            if (schema == nullptr && hinted.schema == nullptr)
            {
                attempt.schemaFailed = true;
                attempt.unexpected = hinted.unexpected;
                attempt.message = "схема по xsi:schemaLocation не загрузилась: " + hinted.message;
                return;
            }
            if (hinted.schema != nullptr)
                schema = hinted.schema;
            countErrors(beresta::xsd::SchemaValidator(schema).validate(document), attempt);
        }
    }

    /*
    ** Любое исключение библиотеки при разборе — «экземпляр не принят»
    ** (не well-formed). Всё прочее — непредусмотренный сбой.
    **
    ** Any library exception during parsing means "the instance is not accepted"
    ** (not well-formed). Everything else is an unexpected failure.
    */
    InstanceAttempt validateInstance(const std::filesystem::path& file,
        std::shared_ptr<const beresta::xsd::Schema> schema,
        const std::vector<std::filesystem::path>& groupDocuments, const std::filesystem::path& suiteRoot)
    {
        beresta::FileResourceResolver resolver(suiteRoot);
        InstanceSource source{file, groupDocuments, suiteRoot};
        InstanceAttempt attempt;
        std::optional<beresta::Document> document;

        try
        {
            document = beresta::XmlParser(instanceOptions(resolver)).parseFile(file);
            validateParsed(*document, source, std::move(schema), attempt);
        }
        catch (const beresta::DomException& problem)
        {
            attempt.rejected = !document.has_value();
            attempt.schemaFailed = document.has_value();
            attempt.message = problem.what();
        }
        catch (const std::exception& problem)
        {
            attempt.unexpected = true;
            attempt.message = problem.what();
        }
        return attempt;
    }
}
