#include "addon/errors.hpp"
#include "addon/results.hpp"

#include <new>

namespace beresta::node
{
    // Данные текущего окружения или nullptr, если init ещё не вызывали
    // The data of the current environment, or nullptr if init has not been called yet
    static AddonState* currentState(napi_env env) noexcept
    {
        void* data = nullptr;

        if (napi().getInstanceData(env, &data) != NAPI_OK)
            return nullptr;
        return static_cast<AddonState*>(data);
    }

    /*
    ** Освобождение данных при уничтожении окружения (выход из Node или
    ** завершение воркера): ссылка на класс отпускается, память — тоже.
    **
    ** Releases the data when the environment is destroyed (Node exits or
    ** a worker ends): the reference to the class is released, and so is the memory.
    */
    static void releaseState(napi_env env, void* data, void* hint)
    {
        AddonState* state = static_cast<AddonState*>(data);

        (void)hint;
        if (state->errorClass != nullptr)
            napi().deleteReference(env, state->errorClass);
        delete state;
    }

    /*
    ** init(BerestaError): JavaScript передаёт класс своей ошибки, и дальше
    ** аддон бросает именно его — вызывающий код ловит обычный instanceof,
    ** а не служебную ошибку Node. Повторный вызов заменяет класс.
    **
    ** init(BerestaError): JavaScript passes its error class, and from then on
    ** the addon throws exactly that — the calling code catches it with an
    ** ordinary instanceof, not a service Node error. A repeated call replaces the class.
    */
    napi_value initialize(napi_env env, napi_callback_info info) noexcept
    {
        std::size_t count = 1;
        napi_value constructor = nullptr;
        AddonState* state = currentState(env);

        if (napi().getCbInfo(env, info, &count, &constructor, nullptr, nullptr) != NAPI_OK || count < 1)
            return nullptr;
        if (state == nullptr)
        {
            state = new (std::nothrow) AddonState{nullptr};
            if (state == nullptr)
                return nullptr;
            if (napi().setInstanceData(env, state, releaseState, nullptr) != NAPI_OK)
            {
                delete state;
                return nullptr;
            }
        }
        if (state->errorClass != nullptr)
            napi().deleteReference(env, state->errorClass);
        state->errorClass = nullptr;
        napi().createReference(env, constructor, 1, &state->errorClass);
        return makeUndefined(env);
    }

    // Класс ошибки текущего окружения / The error class of the current environment
    napi_value errorClass(napi_env env) noexcept
    {
        AddonState* state = currentState(env);
        napi_value constructor = nullptr;

        if (state == nullptr || state->errorClass == nullptr)
            return nullptr;
        if (napi().getReferenceValue(env, state->errorClass, &constructor) != NAPI_OK)
            return nullptr;
        return constructor;
    }
}
