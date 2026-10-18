#ifndef UTILS_SEEDED_HASH_HPP
#define UTILS_SEEDED_HASH_HPP

#include <cstddef>
#include <string_view>

namespace beresta::utils
{
    /*
    ** Хеш строк с секретным ключом для хеш-таблиц, ключи которых приходят
    ** из входного документа (значения ID, имена элементов и атрибутов).
    **
    ** Зачем: std::hash<std::string> не имеет секрета, и злоумышленник может
    ** заранее подобрать тысячи строк с одним хешем. Таблица на таких ключах
    ** вырождается в список, вставка — в O(n), а разбор файла в пару мегабайт
    ** занимает минуты (атака hash flooding, CVE-2011-4885 и родственные).
    **
    ** Как: SipHash-1-3 — криптографически стойкая функция с 128-битным
    ** ключом (её же используют Rust и Python). Ключ случайный и создаётся
    ** один раз на процесс, поэтому подобрать коллизии заранее нельзя.
    ** Порядок обхода таких таблиц от запуска к запуску меняется — поэтому
    ** они используются только для поиска, а не для вывода.
    **
    ** A string hash with a secret key for hash tables whose keys come
    ** from the input document (ID values, element and attribute names).
    **
    ** Why: std::hash<std::string> has no secret, and an attacker can
    ** pick thousands of strings with one hash in advance. A table on such keys
    ** degenerates into a list, an insertion into O(n), and parsing a file of a couple of megabytes
    ** takes minutes (the hash flooding attack, CVE-2011-4885 and related ones).
    **
    ** How: SipHash-1-3 is a cryptographically strong function with a 128-bit
    ** key (Rust and Python use it too). The key is random and created
    ** once per process, so collisions cannot be picked in advance.
    ** The iteration order of such tables changes from run to run — so
    ** they are used only for lookup, not for output.
    */
    struct SeededHash
    {
        std::size_t operator()(std::string_view text) const noexcept;
    };

    /*
    ** Сколько ключей перебирать простым циклом, прежде чем переходить к
    ** хеш-таблице. У почти всех тегов атрибутов единицы, и перебор без
    ** выделения памяти для них быстрее. Но перебор на теге со 100 тысячами
    ** атрибутов — это миллиарды сравнений, то есть готовая атака «отказ в
    ** обслуживании»; выше порога повторы ищутся за O(1).
    **
    ** How many keys to try with a plain loop before switching to a
    ** hash table. Almost all tags have only a few attributes, and a loop without
    ** memory allocation is faster for them. But a loop over a tag with 100 thousand
    ** attributes means billions of comparisons, that is a ready denial-of-service
    ** attack; above the threshold repetitions are found in O(1).
    */
    constexpr std::size_t LINEAR_SEARCH_LIMIT = 16;
}

#endif
