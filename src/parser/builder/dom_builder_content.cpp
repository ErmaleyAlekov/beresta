#include "parser/dom_builder.hpp"
#include "core/cdata_section.hpp"
#include "core/element.hpp"
#include "core/text.hpp"

namespace beresta
{
    // Новый элемент с атрибутами в исходном порядке становится текущим контейнером
    void DomBuilder::startElement(const SaxElement& element)
    {
        auto node = std::make_unique<Element>(std::string(element.name));

        for (const SaxAttribute& attribute : element.attributes)
            node->setAttribute(attribute.name, std::string(attribute.value));
        Element* raw = node.get();
        attach(std::move(node));
        _stack.push_back(raw);
    }

    // Закрытие элемента — возврат к родительскому контейнеру
    void DomBuilder::endElement(const SaxElement& element)
    {
        (void)element;
        if (!_stack.empty())
            _stack.pop_back();
    }

    // Значимый текст
    void DomBuilder::characters(std::string_view text)
    {
        appendText(text);
    }

    // CDATA сохраняется отдельным узлом или (по настройке) сливается с текстом
    void DomBuilder::cdata(std::string_view text)
    {
        if (_coalesceCData)
        {
            appendText(text);
            return;
        }
        attach(std::make_unique<CDataSection>(std::string(text)));
    }

    /*
    ** Соседние куски текста (например, текст до и после раскрытой сущности)
    ** объединяются в один узел — как после normalize() в DOM.
    */
    void DomBuilder::appendText(std::string_view text)
    {
        Node* last = current().getLastChild();

        if (last != nullptr && last->getType() == NodeType::TEXT)
        {
            Text& previous = static_cast<Text&>(*last);
            previous.setValue(previous.getValue() + std::string(text));
            return;
        }
        attach(std::make_unique<Text>(std::string(text)));
    }
}
