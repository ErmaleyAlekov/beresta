#ifndef ELEMENT_COUNTER_HPP
#define ELEMENT_COUNTER_HPP

#include "beresta.hpp"

#include <cstddef>
#include <string>

namespace demo
{
    /*
    ** Обработчик SAX для демонстрации: считает элементы и суммирует
    ** числа из элементов <price>, не храня документ в памяти.
    **
    ** A SAX handler for the demo: counts elements and sums the numbers
    ** from <price> elements without keeping the document in memory.
    */
    class ElementCounter : public beresta::ContentHandler
    {
    public:
        void startElement(const beresta::SaxElement& element) override;
        void endElement(const beresta::SaxElement& element) override;
        void characters(std::string_view text) override;

        std::size_t getElementCount() const noexcept;
        double getTotalPrice() const noexcept;

    private:
        std::size_t _elements = 0;
        bool _insidePrice = false;
        std::string _priceText;
        double _total = 0.0;
    };
}

#endif
