#include "addon/adapters.hpp"
#include "addon/call_frame.hpp"
#include "addon/results.hpp"

namespace beresta::node
{
    // int f(handle, handle) -> статус: перенос узла к новому родителю
    // int f(handle, handle) -> a status: moving a node to a new parent
    napi_value adaptStatusOfHandles(napi_env env, napi_callback_info info) noexcept
    {
        CallFrame call(env, info);
        void* subject = nullptr;
        void* other = nullptr;

        if (!call.isValid() || !call.handle(0, subject) || !call.handle(1, other))
            return nullptr;
        StatusOfHandles function = reinterpret_cast<StatusOfHandles>(call.target());
        return makeStatus(env, function(subject, other));
    }
}
