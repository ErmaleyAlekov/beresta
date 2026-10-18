#ifndef XSD_TREE_EVENT_SOURCE_HPP
#define XSD_TREE_EVENT_SOURCE_HPP

#include "core/document.hpp"
#include "sax/content_handler.hpp"
#include "sax/locator.hpp"

#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** TreeEventSource — готовое дерево как поток событий SAX (паттерн
    ** Adapter): обход в прямом порядке без рекурсии, на каждый узел —
    ** событие startElement / characters / cdata / comment / PI / endElement.
    ** Так проверка готового дерева идёт тем же потоковым валидатором, что и
    ** проверка при разборе, — логика проверки одна. Позиции событий — это
    ** позиции узлов (Node::getSourceLocation). Пространства имён не
    ** вычисляются: их по объявлениям xmlns разрешает сам валидатор.
    **
    ** TreeEventSource — a ready tree as a stream of SAX events (the Adapter
    ** pattern): a traversal in document order without recursion, an event for each node —
    ** startElement / characters / cdata / comment / PI / endElement.
    ** This way validating a ready tree goes through the same streaming validator as
    ** validation during parsing — the validation logic is one. The event positions are
    ** the node positions (Node::getSourceLocation). Namespaces are not
    ** computed: the validator itself resolves them by the xmlns declarations.
    */
    class TreeEventSource : public Locator
    {
    public:
        TreeEventSource();

        // События всего документа: startDocument, поддерево корня, endDocument
        // The events of the whole document: startDocument, the root subtree, endDocument
        void run(const Document& document, ContentHandler& handler);

        SourceLocation getLocation() const override;

    private:
        // ---- Узлы (tree_event_nodes.cpp) ----
        // ---- Nodes (tree_event_nodes.cpp) ----
        bool enter(const Node& node, ContentHandler& handler);
        const Node* leave(const Node& node, const Node& root, ContentHandler& handler);
        void startElement(const Element& element, ContentHandler& handler);
        void endElement(const Element& element, ContentHandler& handler);

        const Node* _current;
        std::vector<SaxAttribute> _attributes;
    };
}

#endif
