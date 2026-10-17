#include "core/document.hpp"
#include "dtd/dtd_model.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    Element* Document::getElementById(std::string_view id)
    {
        const Document& self = *this;

        return const_cast<Element*>(self.getElementById(id));
    }

    // Является ли атрибут идентификатором: xml:id или объявленный в DTD как ID
    static bool isIdentifier(const dtd::DtdModel* model, const Element& element,
        const Attribute& attribute)
    {
        if (attribute.name == "xml:id")
            return true;
        return model != nullptr && model->isIdAttribute(element.getName(), attribute.name);
    }

    /*
    ** Обход всех элементов документа в порядке следования; первый элемент
    ** с ID-атрибутом, равным id, и есть результат. ID уникальны по
    ** стандарту, поэтому «первый» означает «единственный».
    */
    const Element* Document::getElementById(std::string_view id) const
    {
        const DocumentType* doctype = getDocumentType();
        const dtd::DtdModel* model = nullptr;

        if (doctype != nullptr)
            model = doctype->getModel().get();
        for (const Element* element : getElementsByTagName("*"))
        {
            for (const Attribute& attribute : element->getAttributes())
            {
                if (attribute.value == id && isIdentifier(model, *element, attribute))
                    return element;
            }
        }
        return nullptr;
    }
}
