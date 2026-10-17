#include "xsd/internal/validation_run.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xsd::detail
{
    // Есть ли среди детей значимый текст (непробельный TEXT или любая CDATA)
    static bool hasSignificantText(const Element& element)
    {
        for (const Node* child = element.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::CDATA_SECTION)
                return true;
            if (child->getType() == NodeType::TEXT && !utils::isWhitespaceOnly(child->getTextContent()))
                return true;
        }
        return false;
    }

    /*
    ** Содержимое составного типа по его виду: пустое, только текст простого
    ** типа, только элементы или смешанное. Для элементов — модель и дети.
    */
    void ValidationRun::validateContent(const Element& element, const ComplexType& type,
        const ElementDecl* declaration, const std::string& path)
    {
        std::vector<const Element*> children;
        bool text = hasSignificantText(element);

        for (const Node* child = element.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::ELEMENT)
                children.push_back(static_cast<const Element*>(child));
        }
        if (type.content == ContentType::SIMPLE_CONTENT)
        {
            if (!children.empty())
                report(element, path, "тип с простым содержимым не допускает дочерних элементов");
            else
                checkValue(element, *type.simpleContent, element.getTextContent(), declaration, path);
            return;
        }
        if (text && type.content != ContentType::MIXED_CONTENT)
            report(element, path, "текст недопустим: содержимое типа — только элементы");
        checkModel(element, type, children, path);
        validateChildren(type, children, path);
    }

    // Последовательность дочерних элементов сверяется с моделью общим алгоритмом
    void ValidationRun::checkModel(const Element& element, const ComplexType& type,
        const std::vector<const Element*>& children, const std::string& path)
    {
        std::vector<std::string> uris;
        std::vector<content::ChildKey> keys;

        if (!type.hasParticle)
        {
            if (!children.empty())
                report(element, path, "дочерние элементы недопустимы: тип не задаёт модель содержимого");
            return;
        }
        uris.reserve(children.size());
        for (const Element* child : children)
            uris.push_back(child->getNamespaceURI().value_or(""));
        for (std::size_t index = 0; index < children.size(); ++index)
            keys.push_back(content::ChildKey{children[index]->getName(), children[index]->getLocalName(), uris[index]});
        content::MatchResult result = content::matchContent(type.particle, keys);
        if (result.matched)
            return;
        std::string model = content::describeParticle(type.particle);
        if (result.failIndex < children.size())
            report(*children[result.failIndex], childPath(path, *children[result.failIndex]),
                "элемент недопустим в этой позиции; ожидается модель " + model);
        else
            report(element, path, "не хватает обязательных дочерних элементов; модель " + model);
    }

    /*
    ** Дети проверяются по объявлениям из модели. Элементы, допущенные
    ** xs:any, проверяются по глобальным объявлениям согласно processContents.
    */
    void ValidationRun::validateChildren(const ComplexType& type, const std::vector<const Element*>& children,
        const std::string& path)
    {
        for (const Element* child : children)
        {
            std::string childLocation = childPath(path, *child);
            const ElementDecl* declaration = findDeclaration(type, *child);
            if (declaration == nullptr && type.hasElementWildcard
                && type.elementWildcard != ProcessContents::SKIP_CHECK)
                declaration = _schema.findElement(child->getNamespaceURI().value_or(""), child->getLocalName());
            if (declaration != nullptr)
                validateElement(*child, declaration->type, declaration, childLocation);
            else if (type.hasElementWildcard && type.elementWildcard == ProcessContents::STRICT_CHECK)
                report(*child, childLocation, "нет глобального объявления для элемента из xs:any");
        }
    }

    // Объявление ребёнка из модели типа по паре (пространство имён, локальное имя)
    const ElementDecl* ValidationRun::findDeclaration(const ComplexType& type, const Element& child) const
    {
        std::string namespaceUri = child.getNamespaceURI().value_or("");

        for (const ElementDecl* declaration : type.elements)
        {
            if (declaration->name == child.getLocalName() && declaration->namespaceUri == namespaceUri)
                return declaration;
        }
        return nullptr;
    }
}
