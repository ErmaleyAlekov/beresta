#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"

#include <memory>

namespace beresta::capi
{
    /*
    ** Перенос документа в динамическую память. Дескриптор — указатель
    ** Node* (а не Document*), приведённый к void*: так документ можно
    ** передавать и в функции beresta_node_* (единое правило приведения).
    **
    ** Moves a document into dynamic memory. The handle is a Node* pointer
    ** (not a Document*) cast to void*: so the document can also be passed
    ** to the beresta_node_* functions (a single cast rule).
    */
    beresta_document releaseDocument(Document&& document)
    {
        std::unique_ptr<Document> owned = std::make_unique<Document>(std::move(document));
        Node* node = owned.release();

        return static_cast<void*>(node);
    }
}

/*
** Новый документ. Создаём во временном объекте: если имя корня
** недопустимо, исключение вылетит до выделения дескриптора — утечки нет.
**
** A new document. It is created in a temporary object: if the root name
** is invalid, the exception is thrown before the handle is allocated — no leak.
*/
extern "C" BERESTA_API int beresta_document_create(const char* root_name, beresta_document* out)
{
    BERESTA_CAPI_TRY
        beresta::Document document;

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        if (root_name != nullptr)
            document.setRoot(std::make_unique<beresta::Element>(root_name));
        *out = beresta::capi::releaseDocument(std::move(document));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

/*
** Освобождение: деструктор Document освобождает всё дерево (unique_ptr
** вниз по дереву). Проверка типа защищает от двойного освобождения, если
** по ошибке передан дескриптор обычного узла, принадлежащего документу.
**
** Freeing: the Document destructor frees the whole tree (unique_ptr
** down the tree). The type check protects against a double free if
** a handle of an ordinary node owned by a document is passed by mistake.
*/
extern "C" BERESTA_API void beresta_document_free(beresta_document document)
{
    if (document == nullptr)
        return;
    BERESTA_CAPI_TRY
        beresta::Document* owned = &beresta::capi::toDocument(document);

        delete owned;
    BERESTA_CAPI_CATCH_VOID
}

// Глубокая копия через виртуальный clone() (паттерн Prototype)
// A deep copy through the virtual clone() (the Prototype pattern)
extern "C" BERESTA_API int beresta_document_clone(beresta_document document, beresta_document* out)
{
    BERESTA_CAPI_TRY
        const beresta::Document& source = beresta::capi::toDocument(document);

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        *out = beresta::capi::releaseDocument(beresta::Document(source));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Корневой элемент: NULL без ошибки, если корня нет
// The root element: NULL without an error if there is no root
extern "C" BERESTA_API beresta_node beresta_document_root(beresta_document document)
{
    BERESTA_CAPI_TRY
        beresta::Node* root = beresta::capi::toDocument(document).getRoot();

        return root;
    BERESTA_CAPI_CATCH(nullptr)
}
