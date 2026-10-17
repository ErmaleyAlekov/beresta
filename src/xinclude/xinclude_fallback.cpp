#include "xinclude/internal/xinclude_processor.hpp"
#include "xinclude/xinclude.hpp"
#include "core/element.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xinclude::detail
{
    // Обход в порядке документа; поддеревья самих xi:include не просматриваются
    std::vector<Element*> collectIncludes(ContainerNode& root)
    {
        std::vector<Element*> found;

        for (Element* element : root.getElementsByTagNameNS(XINCLUDE_NAMESPACE_URI, "include"))
        {
            bool nested = false;
            for (const Node* parent = element->getParent(); parent != nullptr; parent = parent->getParent())
            {
                if (parent->getType() == NodeType::ELEMENT
                    && isXIncludeElement(*static_cast<const Element*>(parent), "include"))
                    nested = true;
            }
            if (!nested)
                found.push_back(element);
        }
        return found;
    }

    // Элемент XInclude по пространству имён и локальному имени
    bool isXIncludeElement(const Element& element, std::string_view localName)
    {
        return element.getLocalName() == localName
            && element.getNamespaceURI().value_or("") == XINCLUDE_NAMESPACE_URI;
    }

    /*
    ** <xi:fallback>: при ошибке загрузки xi:include заменяется копией его
    ** содержимого. Включения внутри fallback тоже обрабатываются.
    ** Нет fallback — ошибка загрузки распространяется дальше.
    */
    std::vector<std::unique_ptr<Node>> XIncludeProcessor::fallbackNodes(const Element& include,
        const std::string& baseUri)
    {
        const Element* fallback = nullptr;
        std::vector<std::unique_ptr<Node>> nodes;

        for (const Node* child = include.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::ELEMENT
                && isXIncludeElement(*static_cast<const Element*>(child), "fallback"))
                fallback = static_cast<const Element*>(child);
        }
        if (fallback == nullptr)
            throw DomException(DomErrorCode::RESOURCE_ERROR, "xi:include не выполнен, а xi:fallback отсутствует");
        Element holder = *fallback;
        process(holder, baseUri);
        while (holder.getChildCount() > 0)
            nodes.push_back(holder.removeChild(0));
        return nodes;
    }
}

namespace beresta::xinclude
{
    // Публичная точка входа: обработка всего документа от его базового URI
    std::size_t processXIncludes(Document& document, const ParserOptions& options)
    {
        detail::XIncludeProcessor processor(options);

        return processor.process(document, options.baseUri);
    }
}
