#include "xinclude/internal/xinclude_processor.hpp"
#include "xinclude/xinclude.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta::xinclude::detail
{
    // Процессор использует настройки разбора: резолвер, лимиты, базовый URI
    XIncludeProcessor::XIncludeProcessor(const ParserOptions& options)
        : _options(options), _stack(), _count(0)
    {
    }

    /*
    ** Сначала собираются все включения верхнего уровня, затем каждое
    ** заменяется. Вложенные в загруженное содержимое включения
    ** обрабатываются рекурсивно уже с базовым URI загруженного ресурса.
    */
    std::size_t XIncludeProcessor::process(ContainerNode& root, const std::string& baseUri)
    {
        std::size_t before = _count;

        for (Element* include : collectIncludes(root))
            processInclude(*include, baseUri);
        return _count - before;
    }

    // Загрузка (или запасной вариант при ошибке) и замена элемента xi:include
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
    void XIncludeProcessor::replaceWith(Element& include, std::vector<std::unique_ptr<Node>> nodes)
    {
        ContainerNode* parent = include.getParent();
        std::size_t index = include.getIndexInParent();

        if (parent == nullptr)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION, "xi:include не может быть автономным узлом");
        parent->removeChild(index);
        for (std::unique_ptr<Node>& node : nodes)
            parent->insertChild(index++, std::move(node));
    }
}
