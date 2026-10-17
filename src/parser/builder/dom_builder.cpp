#include "parser/dom_builder.hpp"

#include <utility>

namespace beresta
{
    // Настройки построения дерева берутся из параметров парсера
    DomBuilder::DomBuilder(const ParserOptions& options)
        : _document(), _stack(), _locator(nullptr), _trackLocations(options.trackLocations),
          _discardComments(options.discardComments), _coalesceCData(options.coalesceCData)
    {
    }

    // Документ отдаётся перемещением — без копирования дерева
    Document DomBuilder::takeDocument()
    {
        _stack.clear();
        return std::move(_document);
    }

    // Источник позиций для узлов
    void DomBuilder::setDocumentLocator(const Locator& locator)
    {
        _locator = &locator;
    }

    // Вершина стека открытых элементов или сам документ
    ContainerNode& DomBuilder::current()
    {
        if (_stack.empty())
            return _document;
        return *_stack.back();
    }

    // Позиция записывается до вставки, чтобы узел сразу был полностью готов
    Node* DomBuilder::attach(std::unique_ptr<Node>&& node)
    {
        if (_trackLocations && _locator != nullptr)
            node->setSourceLocation(_locator->getLocation());
        return current().appendChild(std::move(node));
    }
}
