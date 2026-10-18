#include "xsd/internal/validation_run.hpp"
#include "i18n/i18n.hpp"
#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Содержимое составного типа по его виду: пустое, только текст простого
    ** типа, только элементы или смешанное. Для элементов — модель и дети.
    **
    ** The content of a complex type by its kind: empty, only text of a simple
    ** type, only elements or mixed. For elements — the model and the children.
    */
    void ValidationRun::validateContent(const Element& element, const ComplexType& type,
        const ElementDecl* declaration, std::size_t path)
    {
        std::string buffer;

        _children.elements.clear();
        _children.uris.clear();
        for (const Node* child = element.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::ELEMENT)
                _children.elements.push_back(static_cast<const Element*>(child));
        }
        if (type.content == ContentType::SIMPLE_CONTENT)
        {
            if (!_children.elements.empty())
                report(element, path, i18n::format(i18n::Message::XSD_SIMPLE_CONTENT_CHILDREN));
            else
                checkValue(element, *type.simpleContent, textOf(element, buffer), declaration, path);
            return;
        }
        if (type.content != ContentType::MIXED_CONTENT && hasSignificantText(element))
            report(element, path, i18n::format(i18n::Message::XSD_TEXT_NOT_ALLOWED));
        for (const Element* child : _children.elements)
            _children.uris.push_back(child->getNamespaceURI().value_or(""));
        checkModel(element, type, path);
        validateChildren(type, path);
    }

    // Последовательность дочерних элементов сверяется с моделью общим алгоритмом
    // The sequence of child elements is matched against the model with the common algorithm
    void ValidationRun::checkModel(const Element& element, const ComplexType& type, std::size_t path)
    {
        const std::vector<const Element*>& children = _children.elements;

        if (!type.hasParticle)
        {
            if (!children.empty())
                report(element, path, i18n::format(i18n::Message::XSD_CHILDREN_NOT_ALLOWED));
            return;
        }
        _keys.clear();
        for (std::size_t index = 0; index < children.size(); ++index)
            _keys.push_back(content::ChildKey{children[index]->getName(), children[index]->getLocalName(),
                _children.uris[index]});
        content::MatchResult result = content::matchContent(type.particle, _keys);
        if (result.matched)
            return;
        std::string model = content::describeParticle(type.particle);
        if (result.failIndex < children.size())
            reportAt(*children[result.failIndex], childPath(pathText(path), *children[result.failIndex]),
                i18n::format(i18n::Message::XSD_CHILD_POSITION, {model}));
        else
            report(element, path, i18n::format(i18n::Message::XSD_CHILDREN_MISSING, {model}));
    }

    /*
    ** Дети проверяются по объявлениям из модели. Элементы, допущенные
    ** xs:any, проверяются по глобальным объявлениям согласно processContents.
    **
    ** The children are validated by the declarations from the model. Elements admitted by
    ** xs:any are validated by the global declarations according to processContents.
    */
    void ValidationRun::validateChildren(const ComplexType& type, std::size_t path)
    {
        std::size_t first = addChildSteps(path);

        _tasks.clear();
        for (std::size_t index = 0; index < _children.elements.size(); ++index)
        {
            const Element* child = _children.elements[index];
            const std::string& namespaceUri = _children.uris[index];
            const ElementDecl* declaration = findDeclaration(type, *child, namespaceUri);
            if (declaration == nullptr && type.hasElementWildcard
                && type.elementWildcard != ProcessContents::SKIP_CHECK)
                declaration = _schema.findElement(namespaceUri, child->getLocalName());
            if (declaration != nullptr)
                _tasks.push_back(PendingElement{child, declaration->type, declaration, first + index});
            else if (type.hasElementWildcard && type.elementWildcard == ProcessContents::STRICT_CHECK)
                report(*child, first + index, i18n::format(i18n::Message::XSD_WILDCARD_UNDECLARED));
        }
        enqueueChildren(_tasks);
    }

    // Объявление ребёнка из модели типа по паре (пространство имён, локальное имя)
    // The declaration of a child from the type model by the pair (namespace, local name)
    const ElementDecl* ValidationRun::findDeclaration(const ComplexType& type, const Element& child,
        const std::string& namespaceUri) const
    {
        for (const ElementDecl* declaration : type.elements)
        {
            if (declaration->name == child.getLocalName() && declaration->namespaceUri == namespaceUri)
                return declaration;
        }
        return nullptr;
    }
}
