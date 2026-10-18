#include "conformance/catalog.hpp"
#include "conformance/conformance_error.hpp"

#include "beresta.hpp"

#include <optional>
#include <string>

namespace conformance
{
    namespace
    {
        // Значение атрибута TYPE -> категория случая
        // The TYPE attribute value -> the case category
        CaseType parseCaseType(const std::string& text, const std::string& id)
        {
            if (text == "valid")
                return CaseType::VALID;
            if (text == "invalid")
                return CaseType::INVALID;
            if (text == "not-wf")
                return CaseType::NOT_WF;
            if (text == "error")
                return CaseType::ERROR_CASE;
            throw ConformanceError("случай " + id + ": неизвестный TYPE \"" + text + "\"");
        }

        /*
        ** Относительный URI из каталога -> абсолютный нормализованный путь.
        ** Сначала — от xml:base предков; если файла там нет, то от каталога
        ** файла-подкаталога, где объявлен этот случай (база элемента внешней
        ** сущности по XML Base). Так находятся случаи hst-*: xml:base у них
        ** в каталоге 2013 года указывает не на ту папку.
        **
        ** A relative URI from the catalog -> an absolute normalized path.
        ** First — from the xml:base of the ancestors; if the file is not there, then from the directory
        ** of the subcatalog file where this case is declared (the base of an element of an external
        ** entity by XML Base). This is how the hst-* cases are found: their xml:base
        ** in the 2013 catalog points to the wrong folder.
        */
        std::filesystem::path resolvePath(const detail::WalkScope& scope, const std::string& uri,
            const std::string& id)
        {
            if (uri.empty())
                return std::filesystem::path();
            std::filesystem::path primary = (scope.base / std::filesystem::u8path(uri)).lexically_normal();

            if (std::filesystem::exists(primary))
                return primary;
            for (const detail::EntityFile& file : *scope.entityFiles)
            {
                bool declaresCase = file.text.find("\"" + id + "\"") != std::string::npos;

                if (declaresCase)
                    return (file.directory / std::filesystem::u8path(uri)).lexically_normal();
            }
            return primary;
        }
    }

    namespace detail
    {
        // Отсутствующий атрибут для каталога равносилен пустому значению
        // A missing attribute is equivalent to an empty value for the catalog
        std::string attributeOrEmpty(const beresta::Element& element, const char* name)
        {
            std::optional<std::string> value = element.getAttribute(name);

            if (value)
                return *value;
            return std::string();
        }

        // Все поля случая берутся из атрибутов <TEST>; пути — относительно xml:base
        // All case fields are taken from the <TEST> attributes; paths are relative to xml:base
        TestCase makeTestCase(const beresta::Element& test, const WalkScope& scope)
        {
            TestCase testCase;

            testCase.id = attributeOrEmpty(test, "ID");
            testCase.type = parseCaseType(attributeOrEmpty(test, "TYPE"), testCase.id);
            testCase.file = resolvePath(scope, attributeOrEmpty(test, "URI"), testCase.id);
            testCase.output = resolvePath(scope, attributeOrEmpty(test, "OUTPUT"), testCase.id);
            testCase.location = testCase.file.lexically_relative(scope.root).generic_u8string();
            testCase.suite = scope.suite;
            testCase.entities = attributeOrEmpty(test, "ENTITIES");
            testCase.recommendation = attributeOrEmpty(test, "RECOMMENDATION");
            testCase.edition = attributeOrEmpty(test, "EDITION");
            testCase.version = attributeOrEmpty(test, "VERSION");
            testCase.sections = attributeOrEmpty(test, "SECTIONS");
            // NAMESPACE="no": документ не обязан соблюдать Namespaces in XML
            // NAMESPACE="no": the document does not have to follow Namespaces in XML
            testCase.namespaceAware = attributeOrEmpty(test, "NAMESPACE") != "no";
            return testCase;
        }
    }
}
