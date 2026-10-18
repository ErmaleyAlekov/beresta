'use strict';

/*
 * Каталог товаров для замеров в Node.js — тот же формат, что у программы
 * замеров на C++ (benchmarks/src/core/workload_catalog.cpp): данные
 * детерминированы, одинаковы при каждом запуске и на любой машине.
 *
 * A product catalog for the Node.js measurements — the same format as in the
 * C++ benchmark (benchmarks/src/core/workload_catalog.cpp): the data is
 * deterministic, the same on every run and on any machine.
 */

// Категории товаров по кругу / Product categories in a circle
const CATEGORIES = ['tea', 'coffee', 'cocoa', 'herbs'];

// Один товар с атрибутами, сущностями и вложенными элементами / One product with attributes, entities and nested elements
function appendItem(parts, index)
{
    const price = (index * 7919) % 50000;
    const priceText = Math.floor(price / 100) + '.' + (10 + price % 90);

    parts.push('<item id="i' + index + '" category="' + CATEGORIES[index % 4] + '" price="' + priceText
        + '" stock="' + ((index * 31) % 1000) + '"><name>Товар №' + index + '</name>'
        + '<description>Описание товара ' + index + ': состав &amp; свойства, цена &lt; '
        + (Math.floor(price / 50) + 1) + ' руб.</description>'
        + '<tags><tag>t' + (index % 97) + '</tag><tag>new</tag></tags></item>');
}

// Каталог из items товаров одной строкой / A catalog of items products on one line
function makeCatalog(items)
{
    const parts = ['<?xml version="1.0" encoding="UTF-8"?><catalog>'];

    for (let index = 0; index < items; ++index)
        appendItem(parts, index);
    parts.push('</catalog>');
    return parts.join('');
}

// Запрос для библиотек с XPath / The query for libraries with XPath
const QUERY = "//item[@category = 'tea' and @price > 250]/name";

/*
 * Схема каталога — та же, что catalogSchema() в программе замеров на C++:
 * последовательность, перечисление, ID, decimal и nonNegativeInteger.
 *
 * The catalog schema — the same as catalogSchema() in the C++ benchmark:
 * a sequence, an enumeration, ID, decimal and nonNegativeInteger.
 */
const CATALOG_XSD = '<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">'
    + '<xs:element name="catalog"><xs:complexType><xs:sequence>'
    + '<xs:element name="item" type="Item" maxOccurs="unbounded"/>'
    + '</xs:sequence></xs:complexType></xs:element>'
    + '<xs:simpleType name="Category"><xs:restriction base="xs:string">'
    + '<xs:enumeration value="tea"/><xs:enumeration value="coffee"/>'
    + '<xs:enumeration value="cocoa"/><xs:enumeration value="herbs"/>'
    + '</xs:restriction></xs:simpleType>'
    + '<xs:complexType name="Item"><xs:sequence>'
    + '<xs:element name="name" type="xs:string"/><xs:element name="description" type="xs:string"/>'
    + '<xs:element name="tags"><xs:complexType><xs:sequence>'
    + '<xs:element name="tag" type="xs:NCName" maxOccurs="unbounded"/>'
    + '</xs:sequence></xs:complexType></xs:element></xs:sequence>'
    + '<xs:attribute name="id" type="xs:ID" use="required"/>'
    + '<xs:attribute name="category" type="Category" use="required"/>'
    + '<xs:attribute name="price" type="xs:decimal" use="required"/>'
    + '<xs:attribute name="stock" type="xs:nonNegativeInteger" use="required"/>'
    + '</xs:complexType></xs:schema>';

module.exports = { makeCatalog, QUERY, CATALOG_XSD };
