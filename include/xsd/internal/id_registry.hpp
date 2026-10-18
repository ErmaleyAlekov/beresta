#ifndef XSD_ID_REGISTRY_HPP
#define XSD_ID_REGISTRY_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** Ячейка таблицы (16 байт — четыре ячейки в строке кэша): место байтов
    ** значения в общем буфере, длина + 1 (0 — ячейка пуста) и старшие биты
    ** хеша, отсеивающие несовпадения без сравнения байтов.
    **
    ** A table slot (16 bytes — four slots per cache line): the place of the value
    ** bytes in the common buffer, the length + 1 (0 — the slot is empty) and the high bits
    ** of the hash that filter out mismatches without comparing bytes.
    */
    struct IdSlot
    {
        std::size_t offset;
        std::uint32_t lengthPlusOne;
        std::uint32_t tag;
    };

    /*
    ** IdRegistry — множество значений xs:ID документа. В выгрузке их сотни
    ** тысяч, и std::unordered_set<std::string> тратил время на выделение
    ** узла под каждое значение и на их освобождение в конце. Здесь
    ** открытая адресация с линейным пробированием: ячейки — один массив,
    ** байты значений — один общий буфер. Хеш — с секретным ключом
    ** (SeededHash): значения ID приходят из документа, и подобрать
    ** коллизии заранее нельзя (защита от hash flooding).
    **
    ** IdRegistry — the set of xs:ID values of a document. An export has hundreds of
    ** thousands of them, and std::unordered_set<std::string> spent time allocating a
    ** node for each value and freeing them at the end. Here it is
    ** open addressing with linear probing: the slots are one array, the
    ** value bytes are one common buffer. The hash has a secret key
    ** (SeededHash): ID values come from the document, and collisions cannot
    ** be picked in advance (protection against hash flooding).
    */
    class IdRegistry
    {
    public:
        IdRegistry();

        // Добавление; false — такое значение уже было
        // Adding; false — such a value was already there
        bool insert(std::string_view value);

        // Есть ли значение во множестве
        // Whether the value is in the set
        bool contains(std::string_view value) const;

    private:
        // Ячейка с этим значением или первая пустая на пути пробирования
        // The slot with this value or the first empty one on the probing path
        std::size_t findSlot(std::string_view value, std::size_t hash) const noexcept;

        // Удвоение таблицы при заполнении наполовину (номера ячеек — по хешам байтов значений)
        // Doubling the table when it is half full (the slot numbers come from the hashes of the value bytes)
        void grow();

        std::vector<IdSlot> _slots;
        std::string _bytes;
        std::size_t _count;
    };
}

#endif
