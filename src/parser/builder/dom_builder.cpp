#include "parser/dom_builder.hpp"

#include <utility>

namespace beresta
{
    // Настройки построения дерева берутся из параметров парсера
    // The tree building settings are taken from the parser parameters
    DomBuilder::DomBuilder(const ParserOptions& options)
        : _document(), _stack(), _locator(nullptr), _trackLocations(options.trackLocations),
          _discardComments(options.discardComments), _coalesceCData(options.coalesceCData)
    {
    }

    // Документ отдаётся перемещением — без копирования дерева
    // The document is handed over by move — without copying the tree
    Document DomBuilder::takeDocument()
    {
        _stack.clear();
        return std::move(_document);
    }

    // Источник позиций для узлов
    // The position source for the nodes
    void DomBuilder::setDocumentLocator(const Locator& locator)
    {
        _locator = &locator;
    }

    // Вершина стека открытых элементов или сам документ
    // The top of the stack of open elements or the document itself
    ContainerNode& DomBuilder::current()
    {
        if (_stack.empty())
            return _document;
        return *_stack.back();
    }

    // Позиция записывается до вставки, чтобы узел сразу был полностью готов
    // The position is recorded before insertion so that the node is complete at once
    Node* DomBuilder::attach(std::unique_ptr<Node>&& node)
    {
        if (_trackLocations && _locator != nullptr)
            node->setSourceLocation(_locator->getLocation());
        return current().appendChild(std::move(node));
    }
}
