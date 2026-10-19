#include "xsd/internal/assertion_recorder.hpp"
#include "core/text.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "xpath/internal/xpath_assertion.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    // Имя и все атрибуты (в том же порядке: номера атрибутов совпадают с номерами типов)
    // The name and all attributes (in the same order: the attribute numbers match the type numbers)
    std::unique_ptr<Element> copyElement(const SaxElement& element)
    {
        std::unique_ptr<Element> copy = std::make_unique<Element>(std::string(element.name));
        std::vector<Attribute> attributes;

        attributes.reserve(element.attributes.size());
        for (const SaxAttribute& attribute : element.attributes)
            attributes.push_back(Attribute{std::string(attribute.name), std::string(attribute.value)});
        copy->setAttributes(std::move(attributes));
        return copy;
    }

    /*
    ** Новое мини-дерево. Префиксы, объявленные у предков элемента, в копии
    ** не видны — они переносятся на корень, иначе имена с префиксами в
    ** мини-дереве потеряли бы свои пространства имён.
    **
    ** A new mini-tree. The prefixes declared at the ancestors of the element are
    ** not visible in the copy — they are moved onto the root, otherwise prefixed names in
    ** the mini-tree would lose their namespaces.
    */
    void AssertionRecorder::startCapture(std::unique_ptr<Element> root, std::size_t level, const ComplexType& type,
        const PrefixBindings& prefixes)
    {
        Element* current = root.get();

        for (const PrefixBinding& binding : prefixes.inScope())
        {
            if (!binding.namespaceUri.empty())
                root->declareNamespace(binding.prefix, binding.namespaceUri);
        }
        _captures.push_back(AssertionCapture{level, &type, std::move(root), current, AssertionTypes()});
    }

    /*
    ** Тип закрываемого элемента и его атрибутов. Пустой элемент с
    ** default / fixed получает текст значения по умолчанию — как в PSVI,
    ** утверждение видит значение, а не пустую строку.
    **
    ** The type of the element being closed and of its attributes. An empty element with
    ** default / fixed gets the text of the default value — as in the PSVI,
    ** an assertion sees the value, not an empty string.
    */
    void AssertionRecorder::annotate(AssertionCapture& capture, Element& element, const CaptureEnd& end)
    {
        const std::vector<Attribute>& attributes = element.getAttributes();

        if (!end.defaultText.empty() && element.getFirstChild() == nullptr)
            element.appendChild(std::make_unique<Text>(std::string(end.defaultText)));
        capture.types.annotate(&element, end.typing);
        for (std::size_t index = 0; index < attributes.size() && index < end.attributeTypes.size(); ++index)
        {
            if (end.attributeTypes[index] != nullptr)
                capture.types.annotate(&attributes[index], NodeTyping{end.attributeTypes[index], false});
        }
    }

    /*
    ** Все утверждения типа (унаследованные и свои) над мини-деревом. Ложное
    ** утверждение и утверждение, упавшее с ошибкой вычисления XPath, —
    ** нарушения (XSD 1.1: ошибка вычисления не равна выполнению).
    **
    ** All the assertions of the type (inherited and own) over the mini-tree. A false
    ** assertion and an assertion failed with an XPath evaluation error are
    ** violations (XSD 1.1: an evaluation error does not count as satisfaction).
    */
    static void evaluateAssertions(const AssertionCapture& capture, std::vector<std::string>& violations)
    {
        for (const std::shared_ptr<const Assertion>& assertion : capture.type->assertions)
        {
            const std::string& test = assertion->test.getSource();
            try
            {
                if (!xpath::detail::ExpressionAccess::evaluateAssertion(assertion->test, *capture.root,
                    assertion->context, capture.types, nullptr))
                    violations.push_back(i18n::format(i18n::Message::XSD_ASSERTION_FAILED, {test}));
            }
            catch (const DomException& error)
            {
                violations.push_back(i18n::format(i18n::Message::XSD_ASSERTION_ERROR, {test, error.getMessage()}));
            }
        }
    }

    /*
    ** Закрытие мини-дерева: проверка, затем дерево (без копирования) и его
    ** типы переходят во внешнее мини-дерево, если оно есть, иначе удаляются.
    **
    ** Closing a mini-tree: the check, then the tree (without copying) and its
    ** types move into the outer mini-tree if there is one, otherwise they are deleted.
    */
    void AssertionRecorder::closeCapture(std::vector<std::string>& violations)
    {
        evaluateAssertions(_captures.back(), violations);
        std::unique_ptr<Element> root = std::move(_captures.back().root);
        AssertionTypes types = std::move(_captures.back().types);
        _captures.pop_back();
        if (_captures.empty())
            return;
        AssertionCapture& outer = _captures.back();
        outer.current->appendChild(std::unique_ptr<Node>(std::move(root)));
        outer.types.absorb(types);
    }
}
