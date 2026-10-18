#include "i18n/i18n.hpp"

#include <array>

namespace beresta::i18n
{
    /*
    ** Реестр языков. Чтобы добавить язык, допишите сюда его каталог —
    ** больше ничего менять не нужно: выбор при сборке, смена во время
    ** работы, Си-интерфейс и обёртки подхватят его автоматически.
    ** Английский обязан быть в реестре: это запасной язык для переводов,
    ** которых ещё нет в других каталогах.
    **
    ** The language registry. To add a language, append its catalog here —
    ** nothing else has to change: build-time selection, run-time switching,
    ** the C interface and the bindings pick it up automatically. English must
    ** be in the registry: it is the fallback for translations other catalogs
    ** do not have yet.
    */
    const std::vector<const Catalog*>& catalogs()
    {
        static const std::vector<const Catalog*> registry = {
            &RUSSIAN_CATALOG,
            &ENGLISH_CATALOG
        };

        return registry;
    }

    // Таблица «идентификатор -> шаблон» одного каталога для поиска за O(1)
    // An "identifier -> template" table of one catalog for O(1) lookup
    using MessageIndex = std::array<const char*, static_cast<std::size_t>(Message::COUNT)>;

    /*
    ** Индекс каталога строится один раз при первом обращении. Пустые места
    ** (перевода нет) заполняются английским текстом, а если нет и его —
    ** самим именем-номером, чтобы сообщение никогда не было пустым.
    **
    ** A catalog index is built once on first use. Empty slots (no
    ** translation) are filled with the English text, and if even that is
    ** missing, with a placeholder, so a message is never empty.
    */
    static MessageIndex buildIndex(const Catalog& catalog)
    {
        MessageIndex index{};

        for (std::size_t entry = 0; entry < ENGLISH_CATALOG.size; ++entry)
            index[static_cast<std::size_t>(ENGLISH_CATALOG.entries[entry].id)] = ENGLISH_CATALOG.entries[entry].text;
        for (std::size_t entry = 0; entry < catalog.size; ++entry)
            index[static_cast<std::size_t>(catalog.entries[entry].id)] = catalog.entries[entry].text;
        for (const char*& text : index)
        {
            if (text == nullptr)
                text = "(message without text)";
        }
        return index;
    }

    // Индексы всех каталогов реестра в том же порядке
    // Indexes of all registry catalogs in the same order
    static std::vector<MessageIndex> buildAllIndexes()
    {
        std::vector<MessageIndex> built;

        for (const Catalog* registered : catalogs())
            built.push_back(buildIndex(*registered));
        return built;
    }

    /*
    ** Шаблон сообщения. Индексы всех каталогов строятся вместе при первом
    ** вызове (статическая переменная функции — потокобезопасная инициализация
    ** C++11), дальше поиск — одно обращение к массиву.
    **
    ** The message template. Indexes of all catalogs are built together on the
    ** first call (a function-local static — thread-safe C++11 initialization);
    ** after that a lookup is a single array access.
    */
    const char* lookup(const Catalog& catalog, Message id)
    {
        static const std::vector<MessageIndex> indexes = buildAllIndexes();
        const std::vector<const Catalog*>& registry = catalogs();

        for (std::size_t position = 0; position < registry.size(); ++position)
        {
            if (registry[position] == &catalog)
                return indexes[position][static_cast<std::size_t>(id)];
        }
        // Каталог не из реестра: отвечает английский (он в реестре обязательно)
        // A catalog outside the registry: English answers (it is always registered)
        return lookup(ENGLISH_CATALOG, id);
    }

    // Каталог по коду: линейный поиск — языков единицы, а не тысячи
    // A catalog by code: a linear search — there are a few languages, not thousands
    const Catalog* findCatalog(std::string_view code) noexcept
    {
        for (const Catalog* catalog : catalogs())
        {
            if (code == catalog->code)
                return catalog;
        }
        return nullptr;
    }
}
