#include "xsd/internal/id_registry.hpp"
#include "utils/seeded_hash.hpp"

#include <cstring>
#include <utility>

namespace beresta::xsd::detail
{
    // Начальный размер таблицы (степень двойки: номер ячейки — хеш по маске)
    // The initial table size (a power of two: the slot number is the hash under a mask)
    static const std::size_t INITIAL_SLOTS = 64;

    // Таблица создаётся при первом значении: у документов без ID она ничего не стоит
    // The table is created on the first value: for documents without IDs it costs nothing
    IdRegistry::IdRegistry()
        : _slots(), _bytes(), _count(0)
    {
    }

    // Новое значение дописывается в общий буфер; ячейка хранит его место и хеш
    // A new value is appended to the common buffer; the slot keeps its place and hash
    bool IdRegistry::insert(std::string_view value)
    {
        if ((_count + 1) * 2 > _slots.size())
            grow();
        std::size_t hash = utils::SeededHash()(value);
        std::size_t index = findSlot(value, hash);
        if (_slots[index].lengthPlusOne != 0)
            return false;
        _slots[index] = IdSlot{_bytes.size(), static_cast<std::uint32_t>(value.size() + 1),
            static_cast<std::uint32_t>(hash >> 32)};
        _bytes.append(value.data(), value.size());
        ++_count;
        return true;
    }

    // Поиск без изменений
    // A lookup without changes
    bool IdRegistry::contains(std::string_view value) const
    {
        if (_slots.empty())
            return false;
        return _slots[findSlot(value, utils::SeededHash()(value))].lengthPlusOne != 0;
    }

    /*
    ** Линейное пробирование: от ячейки hash по маске вперёд до совпадения
    ** или до пустой ячейки. Таблица заполнена не больше чем наполовину,
    ** поэтому пустая ячейка всегда найдётся, а цепочки коротки. Байты
    ** сравниваются, только если совпали старшие биты хеша и длина.
    **
    ** Linear probing: from the slot hash under the mask forward to a match
    ** or to an empty slot. The table is at most half full,
    ** so an empty slot is always found, and the chains are short. The bytes
    ** are compared only if the high bits of the hash and the length match.
    */
    std::size_t IdRegistry::findSlot(std::string_view value, std::size_t hash) const noexcept
    {
        std::size_t mask = _slots.size() - 1;
        std::size_t index = hash & mask;

        while (_slots[index].lengthPlusOne != 0)
        {
            const IdSlot& slot = _slots[index];
            if (slot.tag == static_cast<std::uint32_t>(hash >> 32) && slot.lengthPlusOne == value.size() + 1
                && std::memcmp(_bytes.data() + slot.offset, value.data(), value.size()) == 0)
                return index;
            index = (index + 1) & mask;
        }
        return index;
    }

    // Ячейки переносятся в таблицу вдвое больше; номер ячейки — по хешу байтов значения
    // The slots are moved into a table twice as large; the slot number comes from the hash of the value bytes
    void IdRegistry::grow()
    {
        std::size_t size = INITIAL_SLOTS;
        std::vector<IdSlot> old;

        if (!_slots.empty())
            size = _slots.size() * 2;
        old.swap(_slots);
        _slots.assign(size, IdSlot{0, 0, 0});
        for (const IdSlot& slot : old)
        {
            if (slot.lengthPlusOne == 0)
                continue;
            std::string_view value(_bytes.data() + slot.offset, slot.lengthPlusOne - 1);
            std::size_t index = utils::SeededHash()(value) & (size - 1);
            while (_slots[index].lengthPlusOne != 0)
                index = (index + 1) & (size - 1);
            _slots[index] = slot;
        }
    }
}
