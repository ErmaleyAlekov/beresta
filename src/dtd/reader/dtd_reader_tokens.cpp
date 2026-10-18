#include "dtd/dtd_reader.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::dtd
{
    // Подсмотреть символ (исчерпанные сущности снимаются, чтобы не видеть '\0' на их границе)
    // Peeks at a character (exhausted entities are popped so as not to see '\0' at their boundary)
    char DtdReader::peek()
    {
        popFinished();
        return current().peek();
    }

    // Проверка префикса в текущем сканере
    // Checks a prefix in the current scanner
    bool DtdReader::startsWith(std::string_view fragment)
    {
        popFinished();
        return current().startsWith(fragment);
    }

    // Сдвиг в текущем сканере
    // Moves forward in the current scanner
    void DtdReader::advance(std::size_t count)
    {
        current().advance(count);
    }

    // Обязательный фрагмент
    // A mandatory fragment
    void DtdReader::expect(std::string_view fragment)
    {
        popFinished();
        current().expect(fragment);
    }

    // Имя копируется: исходный сканер может быть снят со стека раньше, чем имя понадобится
    // The name is copied: the source scanner may be popped before the name is needed
    std::string DtdReader::readName()
    {
        popFinished();
        return std::string(current().readName());
    }
}
