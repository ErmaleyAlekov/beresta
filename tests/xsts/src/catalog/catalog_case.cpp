#include "xsts/catalog.hpp"
#include "xsts/xsts_error.hpp"

#include "beresta.hpp"

#include <optional>
#include <string>

namespace xsts
{
    namespace
    {
        // <expected validity="valid|invalid"/>; другое значение — ошибка метаданных
        // <expected validity="valid|invalid"/>; another value is a metadata error
        Expectation readExpectation(const beresta::Element& test)
        {
            for (const beresta::Element* expected : detail::childElements(test, "expected"))
            {
                std::string validity = detail::attributeOrEmpty(*expected, "validity");

                if (validity == "valid")
                    return Expectation::VALID;
                if (validity == "invalid")
                    return Expectation::INVALID;
            }
            throw XstsError("у случая " + detail::attributeOrEmpty(test, "name")
                + " нет <expected validity=\"valid|invalid\"/>");
        }

        // Статус из <current status="...">; без элемента случай считается принятым
        // The status from <current status="...">; without the element the case counts as accepted
        std::string readStatus(const beresta::Element& test)
        {
            for (const beresta::Element* current : detail::childElements(test, "current"))
                return detail::attributeOrEmpty(*current, "status");
            return "accepted";
        }

        /*
        ** Документы случая (schemaDocument или instanceDocument): xlink:href
        ** разрешается от папки файла метаданных и нормализуется,
        ** чтобы URI схемы в резолвере был каноническим.
        **
        ** The case documents (schemaDocument or instanceDocument): xlink:href
        ** is resolved from the metadata file folder and normalized
        ** so that the schema URI in the resolver is canonical.
        */
        std::vector<std::filesystem::path> readDocuments(const beresta::Element& test, CaseKind kind,
            const detail::SetScope& scope)
        {
            const char* element = "instanceDocument";
            std::vector<std::filesystem::path> documents;

            if (kind == CaseKind::SCHEMA)
                element = "schemaDocument";
            for (const beresta::Element* document : detail::childElements(test, element))
            {
                std::optional<std::string> href = document->getAttributeNS(XLINK_NAMESPACE, "href");

                if (href.has_value())
                    documents.push_back((scope.metaDir / std::filesystem::u8path(*href)).lexically_normal());
            }
            if (documents.empty())
                throw XstsError("у случая " + detail::attributeOrEmpty(test, "name") + " нет документов");
            return documents;
        }
    }

    namespace detail
    {
        // Все поля случая; id уникален в наборе: "набор/группа/имя"
        // All the case fields; the id is unique in the suite: "set/group/name"
        TestCase makeCase(const beresta::Element& test, CaseKind kind, const SetScope& scope,
            const std::string& groupName)
        {
            TestCase testCase;

            testCase.name = attributeOrEmpty(test, "name");
            testCase.group = groupName;
            testCase.testSet = scope.testSet;
            testCase.description = scope.description;
            testCase.id = scope.testSet + "/" + groupName + "/" + testCase.name;
            testCase.kind = kind;
            testCase.expected = readExpectation(test);
            testCase.status = readStatus(test);
            testCase.version = attributeOrEmpty(test, "version");
            testCase.documents = readDocuments(test, kind, scope);
            testCase.location = testCase.documents.front().lexically_relative(scope.root).generic_u8string();
            testCase.directory = dataDirectory(testCase.documents.front(), scope.root);
            testCase.features = detectFeatures(testCase.documents);
            return testCase;
        }
    }
}
