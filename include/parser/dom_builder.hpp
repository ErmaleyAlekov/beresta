#ifndef DOM_BUILDER_HPP
#define DOM_BUILDER_HPP

#include "core/container_node.hpp"
#include "core/document.hpp"
#include "parser/parser_options.hpp"
#include "sax/content_handler.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace beresta
{
    /*
    ** DomBuilder — обработчик SAX-событий, строящий дерево (паттерн Builder).
    ** Парсер один, а результатов два: поток событий для SAX и готовый
    ** Document для DOM. Соседние куски текста сливаются в один узел Text.
    ** Открытый класс: его можно использовать с SaxParser напрямую или
    ** унаследовать, чтобы строить дерево и попутно собирать статистику.
    **
    ** DomBuilder — a SAX event handler that builds a tree (the Builder pattern).
    ** There is one parser but two results: an event stream for SAX and a ready
    ** Document for DOM. Adjacent pieces of text are merged into one Text node.
    ** An open class: it can be used with SaxParser directly or
    ** subclassed to build a tree and collect statistics along the way.
    */
    class DomBuilder : public ContentHandler
    {
    public:
        explicit DomBuilder(const ParserOptions& options);

        // Забирает построенный документ (после этого строитель пуст)
        // Takes the built document (after that the builder is empty)
        Document takeDocument();

        void setDocumentLocator(const Locator& locator) override;
        void xmlDeclaration(const XmlDeclaration& declaration) override;
        void documentType(const DocumentType& documentType) override;
        void startElement(const SaxElement& element) override;
        void endElement(const SaxElement& element) override;
        void characters(std::string_view text) override;
        void cdata(std::string_view text) override;
        void comment(std::string_view text) override;
        void processingInstruction(std::string_view target, std::string_view data) override;
        void dtdProcessingInstruction(std::string_view target, std::string_view data) override;

    private:
        // Контейнер, в который сейчас добавляются узлы
        // The container nodes are being added to now
        ContainerNode& current();

        // Добавляет узел (с позицией в исходнике, если её просили записывать)
        // Adds a node (with the source position, if recording it was requested)
        Node* attach(std::unique_ptr<Node>&& node);

        // Дописывает текст в последний Text-узел или создаёт новый
        // Appends text to the last Text node or creates a new one
        void appendText(std::string_view text);

        Document _document;
        std::vector<ContainerNode*> _stack;
        const Locator* _locator;
        bool _trackLocations;
        bool _discardComments;
        bool _coalesceCData;
    };
}

#endif
