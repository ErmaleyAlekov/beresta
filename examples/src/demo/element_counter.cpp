#include "demo/element_counter.hpp"

namespace demo
{
    // Каждый открывающий тег — плюс один элемент; внутри <price> копим текст
    // Each start tag adds one element; inside <price> the text is accumulated
    void ElementCounter::startElement(const beresta::SaxElement& element)
    {
        ++_elements;
        _insidePrice = element.localName == "price";
        _priceText.clear();
    }

    // Закрытие <price>: текст переводится в число через XPath-правила преобразования
    // Closing </price>: the text is converted to a number by the XPath conversion rules
    void ElementCounter::endElement(const beresta::SaxElement& element)
    {
        if (element.localName == "price" && _insidePrice)
            _total += beresta::xpath::stringToNumber(_priceText);
        _insidePrice = false;
    }

    // Текст может прийти несколькими кусками — собираем целиком
    // The text may arrive in several pieces — it is collected whole
    void ElementCounter::characters(std::string_view text)
    {
        if (_insidePrice)
            _priceText.append(text);
    }

    // Сколько элементов встретилось
    // How many elements were encountered
    std::size_t ElementCounter::getElementCount() const noexcept
    {
        return _elements;
    }

    // Сумма цен
    // The sum of the prices
    double ElementCounter::getTotalPrice() const noexcept
    {
        return _total;
    }
}
