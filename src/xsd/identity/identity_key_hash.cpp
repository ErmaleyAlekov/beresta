#include "xsd/internal/identity_common.hpp"
#include "utils/seeded_hash.hpp"

namespace beresta::xsd::detail
{
    // Порядок полей входит в хеш составного ключа.
    // Field order contributes to the compound key hash.
    std::size_t IdentityKeyHash::operator()(const std::vector<std::string>& fields) const noexcept
    {
        std::size_t hash = fields.size();
        for (const std::string& field : fields)
            hash ^= utils::SeededHash()(field) + 0x9e3779b9U + (hash << 6) + (hash >> 2);
        return hash;
    }
}
