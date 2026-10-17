using System;

namespace Beresta
{
    /// <summary>Флаги разбора (BERESTA_PARSE_*), объединяются через |.</summary>
    [Flags]
    public enum ParseFlags : uint
    {
        /// <summary>Настройки по умолчанию: без DTD-проверки, внешние ресурсы запрещены.</summary>
        Default = 0,
        /// <summary>Хранить пробельный текст между тегами.</summary>
        PreserveWhitespace = 1u << 0,
        /// <summary>Не создавать узлы комментариев.</summary>
        DiscardComments = 1u << 1,
        /// <summary>CDATA превращать в обычный текст.</summary>
        CoalesceCData = 1u << 2,
        /// <summary>Проверять документ по DTD.</summary>
        ValidateDtd = 1u << 3,
        /// <summary>Отключить обработку пространств имён.</summary>
        NoNamespaces = 1u << 4,
        /// <summary>Выполнять xi:include.</summary>
        XInclude = 1u << 5,
        /// <summary>Разрешить внешние DTD, сущности и XInclude из каталога файла.</summary>
        FileResources = 1u << 6,
    }

    /// <summary>Флаги вывода (BERESTA_WRITE_*).</summary>
    [Flags]
    public enum WriteFlags : uint
    {
        /// <summary>Компактно, без единого лишнего пробела.</summary>
        Default = 0,
        /// <summary>Отступы и переводы строк.</summary>
        Pretty = 1u << 0,
        /// <summary>Всегда писать XML-декларацию.</summary>
        Declaration = 1u << 1,
    }

    /// <summary>Тип узла (значения совпадают с beresta_node_kind).</summary>
    public enum NodeType
    {
        /// <summary>Документ.</summary>
        Document = 0,
        /// <summary>Объявление DOCTYPE.</summary>
        DocumentType = 1,
        /// <summary>Элемент.</summary>
        Element = 2,
        /// <summary>Текст.</summary>
        Text = 3,
        /// <summary>Секция CDATA.</summary>
        CDataSection = 4,
        /// <summary>Комментарий.</summary>
        Comment = 5,
        /// <summary>Инструкция обработки.</summary>
        ProcessingInstruction = 6,
    }
}
