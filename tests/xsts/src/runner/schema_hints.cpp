#include "xsts/case_runner.hpp"

#include "beresta.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace xsts
{
    namespace
    {
        // Подсказка: пространство имён (пусто — noNamespaceSchemaLocation) и адрес схемы
        // A hint: the namespace (empty — noNamespaceSchemaLocation) and the schema address
        using Hint = std::pair<std::string, std::string>;

        /*
        ** Подсказки xsi:schemaLocation (пары «пространство адрес») и
        ** xsi:noNamespaceSchemaLocation со всех элементов экземпляра.
        ** Обход без рекурсии — явным стеком.
        **
        ** The xsi:schemaLocation hints ("namespace address" pairs) and
        ** xsi:noNamespaceSchemaLocation from all elements of the instance.
        ** The traversal is without recursion — with an explicit stack.
        */
        std::vector<Hint> collectHints(const beresta::Element& root)
        {
            const char* xsi = "http://www.w3.org/2001/XMLSchema-instance";
            std::vector<const beresta::Element*> pending{&root};
            std::vector<Hint> hints;

            while (!pending.empty())
            {
                const beresta::Element* element = pending.back();
                pending.pop_back();
                std::istringstream pairs(element->getAttributeNS(xsi, "schemaLocation").value_or(""));
                std::string space;
                std::string location;
                while (pairs >> space >> location)
                    hints.emplace_back(space, location);
                if (element->hasAttributeNS(xsi, "noNamespaceSchemaLocation"))
                    hints.emplace_back("", *element->getAttributeNS(xsi, "noNamespaceSchemaLocation"));
                for (std::size_t index = 0; index < element->getChildCount(); ++index)
                {
                    const beresta::Node* child = element->getChild(index);
                    if (child->getType() == beresta::NodeType::ELEMENT)
                        pending.push_back(static_cast<const beresta::Element*>(child));
                }
            }
            return hints;
        }

        /*
        ** Документы из подсказок, кроме пространств имён, для которых схема
        ** уже есть (known), и сетевых адресов: прогонщик работает без сети.
        ** Так поступают и Xerces, и System.Xml: схема, заданная явно,
        ** важнее подсказки в экземпляре.
        **
        ** The documents from the hints, except the namespaces for which there is already
        ** a schema (known), and network addresses: the runner works without a network.
        ** Xerces and System.Xml do the same: an explicitly given schema
        ** takes precedence over a hint in the instance.
        */
        std::vector<std::filesystem::path> hintedDocuments(const beresta::Document& instance,
            const std::filesystem::path& file, const std::vector<std::string>& known)
        {
            std::vector<std::filesystem::path> documents;

            for (const Hint& hint : collectHints(*instance.getRoot()))
            {
                std::filesystem::path path = (file.parent_path() / std::filesystem::u8path(hint.second)).lexically_normal();
                bool skipped = hint.second.find("://") != std::string::npos
                    || std::find(known.begin(), known.end(), hint.first) != known.end();

                if (!skipped && std::find(documents.begin(), documents.end(), path) == documents.end())
                    documents.push_back(path);
            }
            return documents;
        }

        // targetNamespace корневых документов схемы группы
        // The targetNamespace of the root documents of the group schema
        std::vector<std::string> targetNamespaces(const std::vector<std::filesystem::path>& documents)
        {
            std::vector<std::string> namespaces;

            for (const std::filesystem::path& document : documents)
            {
                beresta::Document parsed = beresta::XmlParser().parseFile(document);
                namespaces.push_back(parsed.getRoot()->getAttribute("targetNamespace").value_or(""));
            }
            return namespaces;
        }
    }

    /*
    ** Группы без schemaTest задают схему только подсказками в экземпляре.
    ** Без подсказок экземпляр проверяется по пустой схеме: валидным он
    ** может быть лишь благодаря xsi:type со встроенным типом.
    **
    ** Groups without a schemaTest give the schema only by hints in the instance.
    ** Without hints the instance is validated against an empty schema: it can be
    ** valid only thanks to xsi:type with a built-in type.
    */
    SchemaAttempt loadHintedSchema(const beresta::Document& instance,
        const std::filesystem::path& file, const std::filesystem::path& suiteRoot)
    {
        std::vector<std::filesystem::path> documents = hintedDocuments(instance, file, {});

        if (!documents.empty())
            return loadSchema(documents, suiteRoot);
        SchemaAttempt attempt;
        attempt.schema = beresta::xsd::Schema::fromString(
            "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'/>");
        return attempt;
    }

    /*
    ** Схема группы плюс документы из подсказок для пространств имён, которых
    ** в ней нет (экземпляр может ссылаться на тип из второй схемы через
    ** xsi:type). Нет таких подсказок — пустой результат: годится схема группы.
    ** Подсказки, которые и так ведут к документам группы, отсекаются без
    ** разбора — у большинства экземпляров набора лишних подсказок нет.
    **
    ** The group schema plus the documents from the hints for namespaces that are
    ** not in it (an instance may refer to a type from a second schema through
    ** xsi:type). No such hints — an empty result: the group schema will do.
    ** Hints that lead to the group documents anyway are cut off without
    ** parsing — most instances of the suite have no extra hints.
    */
    SchemaAttempt extendWithHints(const std::vector<std::filesystem::path>& groupDocuments,
        const beresta::Document& instance, const std::filesystem::path& file,
        const std::filesystem::path& suiteRoot)
    {
        std::vector<std::filesystem::path> extra;

        for (const std::filesystem::path& document : hintedDocuments(instance, file, {}))
        {
            if (std::find(groupDocuments.begin(), groupDocuments.end(), document) == groupDocuments.end())
                extra.push_back(document);
        }
        if (extra.empty() || groupDocuments.empty())
            return SchemaAttempt();
        std::vector<std::filesystem::path> combined = groupDocuments;
        for (const std::filesystem::path& document : hintedDocuments(instance, file, targetNamespaces(groupDocuments)))
        {
            if (std::find(combined.begin(), combined.end(), document) == combined.end())
                combined.push_back(document);
        }
        if (combined.size() == groupDocuments.size())
            return SchemaAttempt();
        return loadSchema(combined, suiteRoot);
    }
}
