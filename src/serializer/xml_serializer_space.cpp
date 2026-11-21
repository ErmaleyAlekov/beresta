#include "serializer/xml_serializer.hpp"

namespace beresta
{
    /*
    ** xml:space наследуется (XML 1.0, раздел 2.10): решает ближайшее значение
    ** на самом элементе или выше. "preserve" — пробелы значимы, и красивый
    ** вывод выводит такой элемент дословно, иначе повторный разбор получил бы
    ** лишние текстовые узлы из отступов.
    **
    ** xml:space is inherited (XML 1.0, section 2.10): the nearest value
    ** on the element itself or above decides. "preserve" means spaces are significant, and the pretty
    ** output writes such an element verbatim, otherwise parsing again would get
    ** extra text nodes from the indentation.
    */
    bool preservesSpace(const Element& element, bool lookUp)
    {
        const Node* node = &element;

        while (node != nullptr && node->getType() == NodeType::ELEMENT)
        {
            std::optional<std::string> space = static_cast<const Element*>(node)->getAttribute("xml:space");
            if (space.has_value())
                return space.value() == "preserve";
            if (!lookUp)
                return false;
            node = node->getParent();
        }
        return false;
    }
}
