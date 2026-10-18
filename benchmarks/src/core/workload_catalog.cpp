#include "bench/workload.hpp"

#include <string>

namespace bench
{
    // Категории товаров по кругу / Product categories in a circle
    static const char* const CATEGORIES[] = {"tea", "coffee", "cocoa", "herbs"};

    /*
    ** Один товар. Числа получаются из номера простой арифметикой (без
    ** генератора случайных чисел), поэтому данные одинаковы на любой машине.
    ** В описании есть сущности &amp; и &lt; — их раскрытие тоже часть работы.
    **
    ** One product. The numbers come from the index by simple arithmetic (without
    ** a random number generator), so the data is the same on any machine.
    ** The description has the &amp; and &lt; entities — expanding them is part of the work too.
    */
    static void appendItem(std::string& xml, std::size_t index)
    {
        std::string number = std::to_string(index);
        std::size_t price = (index * 7919) % 50000;

        xml += "<item id=\"i" + number + "\" category=\"" + CATEGORIES[index % 4] + "\" price=\"";
        xml += std::to_string(price / 100) + "." + std::to_string(10 + price % 90);
        xml += "\" stock=\"" + std::to_string((index * 31) % 1000) + "\">";
        xml += "<name>Товар №" + number + "</name>";
        xml += "<description>Описание товара " + number + ": состав &amp; свойства, цена &lt; ";
        xml += std::to_string(price / 50 + 1) + " руб.</description>";
        xml += "<tags><tag>t" + std::to_string(index % 97) + "</tag><tag>new</tag></tags></item>";
    }

    // Каталог из items товаров одной строкой (без отступов) / A catalog of items products on one line (without indentation)
    Workload makeCatalog(std::size_t items)
    {
        Workload workload;

        workload.name = "catalog";
        workload.xml.reserve(items * 300);
        workload.xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><catalog>";
        for (std::size_t index = 0; index < items; ++index)
            appendItem(workload.xml, index);
        workload.xml += "</catalog>";
        workload.xsd = catalogSchema();
        workload.query = "//item[@category = 'tea' and @price > 250]/name";
        return workload;
    }

    /*
    ** Схема каталога: последовательность элементов, атрибуты с типами
    ** decimal, nonNegativeInteger и ID, перечисление категорий, NCName у тегов.
    **
    ** The catalog schema: a sequence of elements, attributes with the decimal,
    ** nonNegativeInteger and ID types, an enumeration of categories, NCName for tags.
    */
    std::string catalogSchema()
    {
        return "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:element name=\"catalog\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"item\" type=\"Item\" maxOccurs=\"unbounded\"/>"
            "</xs:sequence></xs:complexType></xs:element>"
            "<xs:simpleType name=\"Category\"><xs:restriction base=\"xs:string\">"
            "<xs:enumeration value=\"tea\"/><xs:enumeration value=\"coffee\"/>"
            "<xs:enumeration value=\"cocoa\"/><xs:enumeration value=\"herbs\"/>"
            "</xs:restriction></xs:simpleType>"
            "<xs:complexType name=\"Item\"><xs:sequence>"
            "<xs:element name=\"name\" type=\"xs:string\"/><xs:element name=\"description\" type=\"xs:string\"/>"
            "<xs:element name=\"tags\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"tag\" type=\"xs:NCName\" maxOccurs=\"unbounded\"/>"
            "</xs:sequence></xs:complexType></xs:element></xs:sequence>"
            "<xs:attribute name=\"id\" type=\"xs:ID\" use=\"required\"/>"
            "<xs:attribute name=\"category\" type=\"Category\" use=\"required\"/>"
            "<xs:attribute name=\"price\" type=\"xs:decimal\" use=\"required\"/>"
            "<xs:attribute name=\"stock\" type=\"xs:nonNegativeInteger\" use=\"required\"/>"
            "</xs:complexType></xs:schema>";
    }
}
