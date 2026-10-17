#include "capi/internal/capi_support.hpp"
#include "core/character_node.hpp"
#include "core/processing_instruction.hpp"

namespace
{
    /*
    ** Замена текста по типу узла. У элемента setTextContent заменяет всех
    ** детей одним текстовым узлом (как textContent в DOM). У символьных
    ** узлов меняется значение — с проверкой допустимости (например,
    ** "--" в комментарии будет отвергнуто ядром).
    */
    void setNodeText(beresta::Node& node, std::string text)
    {
        beresta::Element* element = dynamic_cast<beresta::Element*>(&node);
        beresta::CharacterNode* character = dynamic_cast<beresta::CharacterNode*>(&node);
        beresta::ProcessingInstruction* instruction = dynamic_cast<beresta::ProcessingInstruction*>(&node);

        if (element != nullptr)
            element->setTextContent(std::move(text));
        else if (character != nullptr)
            character->setValue(std::move(text));
        else if (instruction != nullptr)
            instruction->setData(std::move(text));
        else
            throw beresta::capi::InvalidArgumentException("у узла этого типа нельзя задать текст");
    }
}

extern "C" BERESTA_API int beresta_node_set_text(beresta_node node, const char* text)
{
    BERESTA_CAPI_TRY
        setNodeText(beresta::capi::toNode(node), std::string(beresta::capi::requireText(text, "text")));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Удаление: detachChild возвращает unique_ptr, который уничтожает узел
** и всё поддерево при выходе из области видимости. Документ удалить
** нельзя — его освобождают через beresta_document_free.
*/
extern "C" BERESTA_API int beresta_node_remove(beresta_node node)
{
    BERESTA_CAPI_TRY
        beresta::Node& target = beresta::capi::toNode(node);
        beresta::ContainerNode* parent = target.getParent();

        if (parent == nullptr)
            throw beresta::capi::InvalidArgumentException("узел без родителя (документ) удалить нельзя");
        std::unique_ptr<beresta::Node> removed = parent->detachChild(target);
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Копирование поддерева (паттерн Prototype): clone() создаёт полностью
** независимую копию, которая затем добавляется к parent — в том числе
** в другой документ. Исходный узел не меняется.
*/
extern "C" BERESTA_API int beresta_node_copy_to(beresta_node source, beresta_node parent, beresta_node* out)
{
    BERESTA_CAPI_TRY
        std::unique_ptr<beresta::Node> copy = beresta::capi::toNode(source).clone();
        beresta::Node* added = beresta::capi::appendNode(parent, std::move(copy));

        if (out != nullptr)
            *out = added;
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}
