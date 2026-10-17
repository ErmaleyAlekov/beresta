#include "dtd/dtd_reader.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::dtd
{
    // Подсмотреть символ (исчерпанные сущности снимаются, чтобы не видеть '\0' на их границе)
    char DtdReader::peek()
    {
        popFinished();
        return current().peek();
    }

    // Проверка префикса в текущем сканере
    bool DtdReader::startsWith(std::string_view fragment)
    {
        popFinished();
        return current().startsWith(fragment);
    }

    // Сдвиг в текущем сканере
    void DtdReader::advance(std::size_t count)
    {
        current().advance(count);
    }

    // Обязательный фрагмент
    void DtdReader::expect(std::string_view fragment)
    {
        popFinished();
        current().expect(fragment);
    }

    // Имя копируется: исходный сканер может быть снят со стека раньше, чем имя понадобится
    std::string DtdReader::readName()
    {
        popFinished();
        return std::string(current().readName());
    }
}
