#ifndef XPATH_VALUE_HPP
#define XPATH_VALUE_HPP

#include "core/attribute.hpp"
#include "core/element.hpp"
#include "core/node.hpp"

#include <string>
#include <vector>

namespace beresta::xpath
{
    /*
    ** Узел модели данных XPath. В DOM «Берёсты» атрибуты — не узлы,
    ** а свойства элемента, поэтому узел XPath — это либо обычный узел
    ** (attribute == nullptr), либо атрибут вместе с элементом-владельцем.
    */
    struct XPathItem
    {
        const Node* node = nullptr;
        const Attribute* attribute = nullptr;
        const Element* owner = nullptr;
    };

    // Тип значения выражения XPath 1.0
    enum class ValueType
    {
        NODE_SET,
        BOOLEAN,
        NUMBER,
        STRING
    };

    /*
    ** XPathValue — результат вычисления: множество узлов, логическое
    ** значение, число (double) или строка. Преобразования между типами
    ** выполняются строго по правилам функций boolean(), number(), string().
    */
    class XPathValue
    {
    public:
        static XPathValue fromNodeSet(std::vector<XPathItem> nodes);
        static XPathValue fromBoolean(bool value);
        static XPathValue fromNumber(double value);
        static XPathValue fromString(std::string value);

        ValueType getType() const noexcept;

        // Узлы в порядке документа (для не-множеств — пустой список)
        const std::vector<XPathItem>& getNodes() const noexcept;

        bool toBoolean() const;
        double toNumber() const;
        std::string toString() const;

    private:
        XPathValue() noexcept;

        ValueType _type;
        std::vector<XPathItem> _nodes;
        bool _boolean;
        double _number;
        std::string _string;
    };

    // Строковое значение узла (string-value из спецификации XPath)
    std::string stringValue(const XPathItem& item);

    // Число в строку по правилам XPath: целые без ".0", без экспоненты, NaN, Infinity
    std::string numberToString(double value);

    // Строка в число по правилам XPath: [-]цифры[.цифры] с пробелами вокруг, иначе NaN
    double stringToNumber(std::string_view text) noexcept;
}

#endif
