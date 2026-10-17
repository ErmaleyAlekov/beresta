#include "capi/internal/capi_support.hpp"
#include "core/comment.hpp"
#include "core/text.hpp"

namespace beresta::capi
{
    /*
    ** Добавление узла в конец детей. У документа элемент становится
    ** корнем через setRoot — он сам проверит, что корня ещё нет.
    ** Остальные проверки (цикл, недопустимый тип ребёнка) делает ядро;
    ** при ошибке unique_ptr уничтожит узел — утечки нет.
    */
    Node* appendNode(void* parent, std::unique_ptr<Node> child)
    {
        ContainerNode& container = toContainer(parent);
        Document* document = dynamic_cast<Document*>(&container);

        if (document != nullptr && child->getType() == NodeType::ELEMENT && document->getRoot() == nullptr)
        {
            std::unique_ptr<Element> root(static_cast<Element*>(child.release()));

            return document->setRoot(std::move(root));
        }
        return container.appendChild(std::move(child));
    }
}

// Новый элемент: имя проверяется конструктором Element (INVALID_NAME)
extern "C" BERESTA_API beresta_node beresta_node_append_element(beresta_node parent, const char* name)
{
    BERESTA_CAPI_TRY
        std::string tag(beresta::capi::requireText(name, "name"));

        return beresta::capi::appendNode(parent, std::make_unique<beresta::Element>(std::move(tag)));
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API beresta_node beresta_node_append_text(beresta_node parent, const char* text)
{
    BERESTA_CAPI_TRY
        std::string value(beresta::capi::requireText(text, "text"));

        return beresta::capi::appendNode(parent, std::make_unique<beresta::Text>(std::move(value)));
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API beresta_node beresta_node_append_comment(beresta_node parent, const char* text)
{
    BERESTA_CAPI_TRY
        std::string value(beresta::capi::requireText(text, "text"));

        return beresta::capi::appendNode(parent, std::make_unique<beresta::Comment>(std::move(value)));
    BERESTA_CAPI_CATCH(nullptr)
}
