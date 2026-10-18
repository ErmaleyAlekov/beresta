#include "dtd/dtd_reader.hpp"

namespace beresta::dtd
{
    // Ошибка в позиции текущего сканера
    // An error at the position of the current scanner
    void DtdReader::fail(const std::string& message)
    {
        current().fail(message);
    }

    // Общий контекст разбора
    // The common parsing context
    DtdContext& DtdReader::getContext() noexcept
    {
        return _context;
    }

    // Номер вершины стека; исчерпанный, но ещё не снятый кадр считается текущим
    // The number of the stack top; an exhausted but not yet popped frame counts as current
    std::size_t DtdReader::frameId() const noexcept
    {
        if (_frameInfo.empty())
            return 0;
        return _frameInfo.back().id;
    }

    // Кадр «открыт», пока он на стеке и в нём остался непрочитанный текст
    // A frame is "open" while it is on the stack and has unread text left
    bool DtdReader::isFrameOpen(std::size_t id) const noexcept
    {
        if (id == 0)
            return true;
        for (std::size_t index = 0; index < _frameInfo.size(); ++index)
        {
            if (_frameInfo[index].id == id)
                return !_frames[index].isAtEnd();
        }
        return false;
    }

    /*
    ** Внешний текст — внешнее подмножество или (на любой глубине стека)
    ** текст внешней PE. В нём разрешены %pe; внутри объявлений и
    ** условные секции (разделы 2.8 и 3.4; уточнено в errata E18, E60).
    **
    ** External text is the external subset or (at any stack depth)
    ** the text of an external PE. %pe; inside declarations and conditional
    ** sections are allowed in it (sections 2.8 and 3.4; clarified in errata E18, E60).
    */
    bool DtdReader::inExternalText() const noexcept
    {
        if (_context.externalSubset)
            return true;
        for (const FrameInfo& info : _frameInfo)
        {
            if (info.external)
                return true;
        }
        return false;
    }
}
