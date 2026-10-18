using System;

namespace Beresta
{
    /// <summary>Флаги разбора (BERESTA_PARSE_*), объединяются через |.
    /// Parsing flags (BERESTA_PARSE_*), combined with |.</summary>
    [Flags]
    public enum ParseFlags : uint
    {
        /// <summary>Настройки по умолчанию: без DTD-проверки, внешние ресурсы запрещены.
        /// Default settings: no DTD validation, external resources are forbidden.</summary>
        Default = 0,
        /// <summary>Хранить пробельный текст между тегами.
        /// Keep whitespace-only text between tags.</summary>
        PreserveWhitespace = 1u << 0,
        /// <summary>Не создавать узлы комментариев.
        /// Do not create comment nodes.</summary>
        DiscardComments = 1u << 1,
        /// <summary>CDATA превращать в обычный текст.
        /// Turn CDATA into ordinary text.</summary>
        CoalesceCData = 1u << 2,
        /// <summary>Проверять документ по DTD.
        /// Validate the document against its DTD.</summary>
        ValidateDtd = 1u << 3,
        /// <summary>Отключить обработку пространств имён.
        /// Disable namespace processing.</summary>
        NoNamespaces = 1u << 4,
        /// <summary>Выполнять xi:include.
        /// Process xi:include.</summary>
        XInclude = 1u << 5,
        /// <summary>Разрешить внешние DTD, сущности и XInclude из каталога файла.
        /// Allow external DTDs, entities and XInclude from the file's directory.</summary>
        FileResources = 1u << 6,
    }

    /// <summary>Флаги вывода (BERESTA_WRITE_*).
    /// Output flags (BERESTA_WRITE_*).</summary>
    [Flags]
    public enum WriteFlags : uint
    {
        /// <summary>Компактно, без единого лишнего пробела.
        /// Compact, without a single extra space.</summary>
        Default = 0,
        /// <summary>Отступы и переводы строк.
        /// Indentation and line breaks.</summary>
        Pretty = 1u << 0,
        /// <summary>Всегда писать XML-декларацию.
        /// Always write the XML declaration.</summary>
        Declaration = 1u << 1,
    }

    /// <summary>Тип узла (значения совпадают с beresta_node_kind).
    /// Node type (the values match beresta_node_kind).</summary>
    public enum NodeType
    {
        /// <summary>Документ.
        /// A document.</summary>
        Document = 0,
        /// <summary>Объявление DOCTYPE.
        /// A DOCTYPE declaration.</summary>
        DocumentType = 1,
        /// <summary>Элемент.
        /// An element.</summary>
        Element = 2,
        /// <summary>Текст.
        /// Text.</summary>
        Text = 3,
        /// <summary>Секция CDATA.
        /// A CDATA section.</summary>
        CDataSection = 4,
        /// <summary>Комментарий.
        /// A comment.</summary>
        Comment = 5,
        /// <summary>Инструкция обработки.
        /// A processing instruction.</summary>
        ProcessingInstruction = 6,
    }
}
