#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"

#include <set>
#include <string>
#include <utility>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** Атрибут id у элементов XSD имеет тип xs:ID: значение — NCName,
    ** уникальное в пределах ОДНОГО документа схемы. Поэтому проверка идёт
    ** отдельным обходом документа, а не при сборке компонентов: модель
    ** именованной группы собирается заново в каждом месте ссылки, и
    ** её id нельзя считать повтором. Обход — явным стеком с глубиной каждого
    ** элемента: документ глубже MAX_DOCUMENT_DEPTH отвергается сразу, иначе
    ** обход стоил бы O(n · глубина). Содержимое xs:appinfo / xs:documentation
    ** чужое и не проверяется.
    **
    ** The id attribute of XSD elements has the xs:ID type: the value is an NCName
    ** unique within ONE schema document. So the check is a separate traversal
    ** of the document, not part of building the components: the model of a named
    ** group is rebuilt at every reference place, and its id must not be counted
    ** as a repeat. The traversal uses an explicit stack with the depth of each
    ** element: a document deeper than MAX_DOCUMENT_DEPTH is rejected at once, otherwise
    ** the traversal would cost O(n · depth). The content of xs:appinfo /
    ** xs:documentation is foreign and not checked.
    */
    void SchemaBuilder::checkDocumentIds(const Element& root) const
    {
        std::set<std::string> seen;
        std::vector<std::pair<const Element*, std::size_t>> pending{{&root, 1}};

        while (!pending.empty())
        {
            const Element& current = *pending.back().first;
            std::size_t depth = pending.back().second;
            pending.pop_back();
            checkIdAttribute(current, seen);
            if (isSchemaElement(current, "appinfo") || isSchemaElement(current, "documentation"))
                continue;
            for (const Node* node = current.getFirstChild(); node != nullptr; node = node->getNextSibling())
            {
                if (node->getType() != NodeType::ELEMENT)
                    continue;
                const Element* child = static_cast<const Element*>(node);
                if (depth >= MAX_DOCUMENT_DEPTH)
                    fail(*child, i18n::format(i18n::Message::SCHEMA_DOCUMENT_TOO_DEEP,
                        {std::to_string(MAX_DOCUMENT_DEPTH)}));
                if (versionAccepted(*child) && child->getNamespaceURI().value_or("") == XSD_NAMESPACE_URI)
                    pending.emplace_back(child, depth + 1);
            }
        }
    }

    // id (если есть) — NCName и не повторяется среди уже встреченных в документе
    // id (if any) is an NCName and does not repeat among the ones already met in the document
    void SchemaBuilder::checkIdAttribute(const Element& element, std::set<std::string>& seen) const
    {
        std::optional<std::string> id = element.getAttribute("id");

        if (!id.has_value())
            return;
        std::string value = normalizeWhiteSpace(*id, WhiteSpace::COLLAPSE);
        checkNCName(element, "id", value);
        if (!seen.insert(value).second)
            fail(element, i18n::format(i18n::Message::SCHEMA_DUPLICATE_ID, {value}));
    }

    /*
    ** Порядок детей компонента XSD: xs:annotation — не больше одной и
    ** только первой (у всех элементов, кроме xs:schema / xs:redefine /
    ** xs:override, которые сюда не попадают). Остальные дети проверяет
    ** checkGroupChild. Исключённые vc:minVersion / vc:maxVersion
    ** элементы и элементы чужих пространств имён не учитываются.
    **
    ** The order of the children of an XSD component: xs:annotation — at most one and
    ** only as the first (for all elements except xs:schema / xs:redefine /
    ** xs:override, which do not get here). The other children are checked by
    ** checkGroupChild. Elements excluded by vc:minVersion / vc:maxVersion
    ** and elements of foreign namespaces are not counted.
    */
    void SchemaBuilder::checkChildOrder(const Element& element) const
    {
        bool content = false;
        std::size_t annotations = 0;
        std::size_t models = 0;

        for (const Node* node = element.getFirstChild(); node != nullptr; node = node->getNextSibling())
        {
            if (node->getType() != NodeType::ELEMENT)
                continue;
            const Element& child = static_cast<const Element&>(*node);
            if (child.getNamespaceURI().value_or("") != XSD_NAMESPACE_URI || !versionAccepted(child))
                continue;
            if (child.getLocalName() != "annotation")
            {
                content = true;
                checkGroupChild(element, child, models);
            }
            else if (content || ++annotations > 1)
                fail(child, i18n::format(i18n::Message::SCHEMA_ANNOTATION_POSITION,
                    {std::string(element.getLocalName())}));
        }
    }

    /*
    ** Два правила схемы для схем про xs:group:
    **  - внутри xs:group — кроме аннотации, не больше одной модели
    **    sequence / choice / all и ничего другого;
    **  - ссылка xs:group стоит только в модели (sequence / choice / all)
    **    или прямо в типе (complexType, restriction, extension); глобальные
    **    определения xs:group сюда не попадают — их родитель xs:schema
    **    или xs:redefine не проверяется.
    ** models — счётчик моделей, уже встреченных у этого родителя.
    **
    ** Two rules of the schema for schemas about xs:group:
    **  - inside xs:group — besides the annotation, at most one model
    **    sequence / choice / all and nothing else;
    **  - an xs:group reference stands only in a model (sequence / choice / all)
    **    or right in a type (complexType, restriction, extension); global
    **    xs:group definitions do not get here — their parent xs:schema
    **    or xs:redefine is not checked.
    ** models — the counter of the models already met at this parent.
    */
    void SchemaBuilder::checkGroupChild(const Element& parent, const Element& child, std::size_t& models) const
    {
        static const std::set<std::string_view> MODELS{"sequence", "choice", "all"};
        static const std::set<std::string_view> GROUP_PLACES{"sequence", "choice", "all",
            "complexType", "restriction", "extension"};
        std::string_view parentKind = parent.getLocalName();
        std::string_view kind = child.getLocalName();
        bool misplaced = false;

        if (parentKind == "group" && (MODELS.count(kind) == 0 || ++models > 1))
            misplaced = true;
        if (kind == "group" && GROUP_PLACES.count(parentKind) == 0)
            misplaced = true;
        if (misplaced)
            fail(child, i18n::format(i18n::Message::SCHEMA_CHILD_NOT_ALLOWED,
                {std::string(kind), std::string(parentKind)}));
    }

    // NCName — XML-имя (с символами Юникода) без двоеточия
    // NCName — an XML name (with Unicode characters) without a colon
    void SchemaBuilder::checkNCName(const Element& where, std::string_view attribute,
        const std::string& value) const
    {
        if (!utils::isValidName(value) || value.find(':') != std::string::npos)
            fail(where, i18n::format(i18n::Message::SCHEMA_NOT_NCNAME, {std::string(attribute), value}));
    }
}
