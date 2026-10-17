#include "capi/internal/capi_support.hpp"

#include <memory>

namespace beresta::capi
{
    /*
    ** Перенос документа в динамическую память. Дескриптор — указатель
    ** Node* (а не Document*), приведённый к void*: так документ можно
    ** передавать и в функции beresta_node_* (единое правило приведения).
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
*/
extern "C" BERESTA_API int beresta_document_create(const char* root_name, beresta_document* out)
{
    BERESTA_CAPI_TRY
        beresta::Document document;

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException("out: передан NULL");
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
extern "C" BERESTA_API int beresta_document_clone(beresta_document document, beresta_document* out)
{
    BERESTA_CAPI_TRY
        const beresta::Document& source = beresta::capi::toDocument(document);

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException("out: передан NULL");
        *out = beresta::capi::releaseDocument(beresta::Document(source));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Корневой элемент: NULL без ошибки, если корня нет
extern "C" BERESTA_API beresta_node beresta_document_root(beresta_document document)
{
    BERESTA_CAPI_TRY
        beresta::Node* root = beresta::capi::toDocument(document).getRoot();

        return root;
    BERESTA_CAPI_CATCH(nullptr)
}
