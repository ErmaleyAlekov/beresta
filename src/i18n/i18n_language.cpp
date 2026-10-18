#include "i18n/i18n.hpp"

#include <atomic>

namespace beresta::i18n
{
    /*
    ** Текущий каталог. Атомарный указатель: смена языка в одном потоке и
    ** форматирование сообщений в других не создают гонки данных. Каталоги
    ** статические и неизменяемые, поэтому указатель никогда не «повиснет».
    ** Начальное значение — язык, выбранный при сборке (BERESTA_ERRORS_LANG),
    ** а если такого каталога нет — английский.
    **
    ** The current catalog. An atomic pointer: changing the language in one
    ** thread while other threads format messages causes no data race.
    ** Catalogs are static and immutable, so the pointer never dangles. The
    ** initial value is the language chosen at build time (BERESTA_ERRORS_LANG),
    ** or English if there is no such catalog.
    */
    static std::atomic<const Catalog*>& currentCatalog() noexcept
    {
        static std::atomic<const Catalog*> current{nullptr};

        if (current.load() == nullptr)
        {
            const Catalog* initial = findCatalog(BERESTA_I18N_STRINGIZE(BERESTA_ERRORS_LANG));
            if (initial == nullptr)
                initial = &ENGLISH_CATALOG;
            const Catalog* expected = nullptr;
            current.compare_exchange_strong(expected, initial);
        }
        return current;
    }

    // Смена языка: неизвестный код не меняет текущий язык
    // Changes the language: an unknown code leaves the current language unchanged
    bool setLanguage(std::string_view code)
    {
        const Catalog* catalog = findCatalog(code);

        if (catalog == nullptr)
            return false;
        currentCatalog().store(catalog);
        return true;
    }

    // Код текущего языка
    // The current language code
    std::string_view language() noexcept
    {
        return currentCatalog().load()->code;
    }

    // Коды всех каталогов реестра
    // Codes of all registry catalogs
    std::vector<std::string_view> availableLanguages()
    {
        std::vector<std::string_view> codes;

        for (const Catalog* catalog : catalogs())
            codes.push_back(catalog->code);
        return codes;
    }
}
