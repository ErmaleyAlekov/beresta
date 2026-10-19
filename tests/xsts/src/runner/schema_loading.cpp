#include "xsts/case_runner.hpp"

#include "beresta.hpp"

#include <exception>
#include <string>
#include <vector>

namespace xsts
{
    namespace
    {
        // Экранирует значение для атрибута в двойных кавычках
        // Escapes a value for an attribute in double quotes
        std::string escapeAttribute(const std::string& value)
        {
            std::string escaped;

            for (char symbol : value)
            {
                if (symbol == '&')
                    escaped += "&amp;";
                else if (symbol == '<')
                    escaped += "&lt;";
                else if (symbol == '"')
                    escaped += "&quot;";
                else
                    escaped += symbol;
            }
            return escaped;
        }

        /*
        ** Схема из нескольких документов собирается документом-«драйвером»:
        ** xs:import для документа с targetNamespace и xs:include для
        ** документа без него. Так ВСЕ документы загружаются через резолвер
        ** под своими URI, и документ, который другой документ набора
        ** подключает сам, загружается один раз (как требует спецификация),
        ** а не дважды с ошибкой «повторное определение».
        **
        ** A schema of several documents is assembled by a "driver" document:
        ** xs:import for a document with a targetNamespace and xs:include for
        ** a document without one. So ALL documents are loaded through the resolver
        ** under their own URIs, and a document that another document of the set
        ** includes itself is loaded once (as the specification requires),
        ** not twice with a "duplicate definition" error.
        */
        std::string driverText(const std::vector<std::filesystem::path>& documents)
        {
            std::string text = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>";

            for (const std::filesystem::path& document : documents)
            {
                beresta::Document parsed = beresta::XmlParser().parseFile(document);
                std::optional<std::string> target = parsed.getRoot()->getAttribute("targetNamespace");
                std::string location = escapeAttribute(document.generic_u8string());

                if (target.has_value() && !target->empty())
                    text += "<xs:import namespace=\"" + escapeAttribute(*target) + "\" schemaLocation=\""
                        + location + "\"/>";
                else
                    text += "<xs:include schemaLocation=\"" + location + "\"/>";
            }
            return text + "</xs:schema>";
        }

        /*
        ** Один документ — Schema::fromDocuments с его путём как базой;
        ** несколько — через драйвер. У драйвера свой (несуществующий) URI
        ** в той же папке: библиотека запоминает URI корневого документа,
        ** и с путём первого документа его импорт был бы пропущен как
        ** «уже загруженный».
        **
        ** One document — Schema::fromDocuments with its path as the base;
        ** several — through the driver. The driver has its own (non-existent) URI
        ** in the same folder: the library remembers the URI of the root document,
        ** and with the path of the first document its import would be skipped as
        ** "already loaded".
        */
        std::shared_ptr<const beresta::xsd::Schema> compile(const std::vector<std::filesystem::path>& documents,
            beresta::xsd::SchemaLoadOptions options)
        {
            options.baseUri = documents.front().u8string();
            if (documents.size() > 1)
            {
                options.baseUri = (documents.front().parent_path() / "xsts-driver.xsd").u8string();
                return beresta::xsd::Schema::fromString(driverText(documents), options);
            }
            beresta::ParserOptions parserOptions;
            parserOptions.trackLocations = true;
            beresta::Document parsed = beresta::XmlParser(parserOptions).parseFile(documents.front());
            return beresta::xsd::Schema::fromDocuments({&parsed}, options);
        }
    }

    /*
    ** Факты загрузки: схема, отказ библиотеки (DomException) или сбой.
    **
    ** The loading facts: the schema, a library rejection (DomException) or a failure.
    */
    SchemaAttempt loadSchema(const std::vector<std::filesystem::path>& documents,
        const std::filesystem::path& suiteRoot)
    {
        beresta::FileResourceResolver resolver(suiteRoot);
        beresta::xsd::SchemaLoadOptions options;
        SchemaAttempt attempt;

        options.resolver = &resolver;
        try
        {
            attempt.schema = compile(documents, options);
        }
        catch (const beresta::DomException& problem)
        {
            attempt.rejected = true;
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
