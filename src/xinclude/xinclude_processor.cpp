#include "xinclude/internal/xinclude_processor.hpp"
#include "i18n/i18n.hpp"
#include "xinclude/xinclude.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xinclude::detail
{
    // Процессор использует настройки разбора: резолвер, лимиты, базовый URI
    // The processor uses the parse settings: the resolver, the limits, the base URI
    XIncludeProcessor::XIncludeProcessor(const ParserOptions& options)
        : _options(options), _stack(), _count(0)
    {
    }

    /*
    ** Сначала собираются все включения верхнего уровня, затем каждое
    ** заменяется. Вложенные в загруженное содержимое включения
    ** обрабатываются рекурсивно уже с базовым URI загруженного ресурса.
    **
    ** First all top-level inclusions are collected, then each one
    ** is replaced. Inclusions nested in the loaded content are processed
    ** recursively, with the base URI of the loaded resource.
    */
    std::size_t XIncludeProcessor::process(ContainerNode& root, const std::string& baseUri)
    {
        std::size_t before = _count;

        for (Element* include : collectIncludes(root))
            processInclude(*include, baseUri);
        return _count - before;
    }

    // Загрузка (или запасной вариант при ошибке) и замена элемента xi:include
    // Loads (or uses the fallback on error) and replaces the xi:include element
    void XIncludeProcessor::processInclude(Element& include, const std::string& baseUri)
    {
        std::vector<std::unique_ptr<Node>> nodes;

        try
        {
            nodes = loadReplacement(include, baseUri);
        }
        catch (const DomException& error)
        {
            if (error.getCode() == DomErrorCode::LIMIT_EXCEEDED)
                throw;
            nodes = fallbackNodes(include, baseUri);
        }
        replaceWith(include, std::move(nodes));
        ++_count;
    }

    // Новые узлы встают на место xi:include в том же порядке; сам xi:include удаляется
    // The new nodes take the place of xi:include in the same order; xi:include itself is removed
    void XIncludeProcessor::replaceWith(Element& include, std::vector<std::unique_ptr<Node>> nodes)
    {
        ContainerNode* parent = include.getParent();
        std::size_t index = include.getIndexInParent();

        if (parent == nullptr)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION, i18n::format(i18n::Message::XINCLUDE_DETACHED));
        parent->removeChild(index);
        for (std::unique_ptr<Node>& node : nodes)
            parent->insertChild(index++, std::move(node));
    }
}
