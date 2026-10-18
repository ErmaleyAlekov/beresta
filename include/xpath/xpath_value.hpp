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
    **
    ** A node of the XPath data model. In the Beresta DOM attributes are not nodes
    ** but element properties, so an XPath node is either an ordinary node
    ** (attribute == nullptr) or an attribute together with its owner element.
    */
    struct XPathItem
    {
        const Node* node = nullptr;
        const Attribute* attribute = nullptr;
        const Element* owner = nullptr;

        /*
        ** Узел пространства имён (ось namespace). Он тоже «виртуальный»:
        ** attribute указывает на объявляющий атрибут xmlns:p (у этого
        ** элемента или его предка), owner — элемент, к которому узел
        ** относится. Имя узла — префикс, строковое значение — URI.
        **
        ** A namespace node (the namespace axis). It is "virtual" too:
        ** attribute points to the declaring xmlns:p attribute (of this
        ** element or its ancestor), owner is the element the node belongs to.
        ** The node name is the prefix, the string value is the URI.
        */
        bool isNamespace = false;
    };

    // Тип значения выражения XPath 1.0
    // The value type of an XPath 1.0 expression
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
    **
    ** XPathValue — the evaluation result: a node set, a boolean,
    ** a number (double) or a string. Conversions between the types
    ** follow the rules of the boolean(), number(), string() functions strictly.
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
        // The nodes in document order (for non-sets — an empty list)
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
    // The string value of a node (string-value from the XPath specification)
    std::string stringValue(const XPathItem& item);

    // Число в строку по правилам XPath: целые без ".0", без экспоненты, NaN, Infinity
    // A number to a string by the XPath rules: integers without ".0", no exponent, NaN, Infinity
    std::string numberToString(double value);

    // Строка в число по правилам XPath: [-]цифры[.цифры] с пробелами вокруг, иначе NaN
    // A string to a number by the XPath rules: [-]digits[.digits] with spaces around, otherwise NaN
    double stringToNumber(std::string_view text) noexcept;
}

#endif
