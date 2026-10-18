#include "xinclude/internal/xinclude_processor.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::xinclude::detail
{
    // Номер шага XPointer (с единицы); некорректный — 0
    // The number of an XPointer step (from one); an invalid one is 0
    static std::size_t parseStep(std::string_view text) noexcept
    {
        std::size_t value = 0;

        if (text.empty() || text.size() > 9)
            return 0;
        for (char symbol : text)
        {
            if (symbol < '0' || symbol > '9')
                return 0;
            value = value * 10 + static_cast<std::size_t>(symbol - '0');
        }
        return value;
    }

    // Дочерний элемент с номером position (среди элементов) или nullptr
    // The child element with number position (among elements) or nullptr
    static ContainerNode* childElementAt(ContainerNode& parent, std::size_t position)
    {
        for (Node* child = parent.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() != NodeType::ELEMENT)
                continue;
            if (--position == 0)
                return static_cast<Element*>(child);
        }
        return nullptr;
    }

    /*
    ** XPointer: element(/1/3) — путь по номерам дочерних элементов от
    ** документа, element(id/2) — от элемента с ID; без схемы — сам ID.
    ** Выбранный элемент извлекается из загруженного документа.
    **
    ** XPointer: element(/1/3) — a path by the numbers of child elements from
    ** the document, element(id/2) — from the element with the ID; without a scheme — the ID itself.
    ** The selected element is extracted from the loaded document.
    */
    std::unique_ptr<Node> XIncludeProcessor::selectPointer(Document& document, std::string_view pointer) const
    {
        std::string_view path = pointer;
        ContainerNode* cursor = &document;

        if (pointer.substr(0, 8) == "element(" && pointer.back() == ')')
            path = pointer.substr(8, pointer.size() - 9);
        std::size_t slash = path.find('/');
        if (slash != 0)
            cursor = document.getElementById(path.substr(0, slash));
        while (cursor != nullptr && slash != std::string_view::npos)
        {
            std::size_t next = path.find('/', slash + 1);
            std::size_t step = parseStep(path.substr(slash + 1, next - slash - 1));
            ContainerNode* parent = cursor;
            cursor = nullptr;
            if (step > 0)
                cursor = childElementAt(*parent, step);
            slash = next;
        }
        if (cursor == nullptr || cursor == &document)
            throw DomException(DomErrorCode::RESOURCE_ERROR, i18n::format(i18n::Message::XPOINTER_NOT_FOUND, {pointer}));
        return cursor->getParent()->detachChild(*cursor);
    }
}
