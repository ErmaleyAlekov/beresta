#include "xsd/internal/assertion_recorder.hpp"
#include "core/text.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    // Запись идёт, пока открыт хотя бы один элемент с утверждениями
    // Recording goes on while at least one element with assertions is open
    bool AssertionRecorder::recording() const noexcept
    {
        return !_captures.empty();
    }

    /*
    ** Начало элемента: накопленный текст родителя закрывается узлом Text,
    ** затем элемент с утверждениями открывает своё мини-дерево, а любой
    ** другой становится ребёнком текущего элемента открытого мини-дерева.
    **
    ** An element start: the accumulated text of the parent is closed with a Text node,
    ** then an element with assertions opens its own mini-tree, and any
    ** other one becomes a child of the current element of the open mini-tree.
    */
    void AssertionRecorder::elementStarted(const SaxElement& element, std::size_t level, const ComplexType* asserted,
        const PrefixBindings& prefixes)
    {
        std::unique_ptr<Element> copy = copyElement(element);

        flushText();
        if (asserted != nullptr)
        {
            startCapture(std::move(copy), level, *asserted, prefixes);
            return;
        }
        AssertionCapture& capture = _captures.back();
        capture.current = static_cast<Element*>(capture.current->appendChild(std::unique_ptr<Node>(std::move(copy))));
    }

    // Куски текста копятся в одной строке: парсер может отдавать текст частями
    // Pieces of text accumulate in one string: the parser may deliver text in parts
    void AssertionRecorder::textAppended(std::string_view text)
    {
        _pendingText.append(text.data(), text.size());
    }

    /*
    ** Конец элемента: его текст, тип элемента и атрибутов; затем подъём к
    ** родителю — или, если это корень мини-дерева, проверка утверждений.
    **
    ** An element end: its text, the type of the element and of the attributes; then climbing to
    ** the parent — or, if it is the root of a mini-tree, the assertion check.
    */
    void AssertionRecorder::elementEnded(const CaptureEnd& end, std::vector<std::string>& violations)
    {
        AssertionCapture& capture = _captures.back();
        Element& element = *capture.current;

        flushText();
        annotate(capture, element, end);
        if (capture.level == end.level)
        {
            closeCapture(violations);
            return;
        }
        capture.current = static_cast<Element*>(element.getParent());
    }

    // Накопленный текст становится узлом Text текущего элемента
    // The accumulated text becomes a Text node of the current element
    void AssertionRecorder::flushText()
    {
        if (_pendingText.empty() || _captures.empty())
        {
            _pendingText.clear();
            return;
        }
        _captures.back().current->appendChild(std::make_unique<Text>(std::move(_pendingText)));
        _pendingText.clear();
    }
}
