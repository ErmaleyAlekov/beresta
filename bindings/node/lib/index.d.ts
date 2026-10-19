/*
 * Типы TypeScript для «Берёсты» (Node.js). Пакет написан на JavaScript,
 * а этот файл даёт редактору автодополнение, а компилятору — строгую
 * проверку: дескрипторы, флаги и результаты XPath имеют точные типы.
 *
 * TypeScript types for Beresta (Node.js). The package is written in JavaScript,
 * and this file gives the editor autocompletion and the compiler strict
 * checking: handles, flags and XPath results have exact types.
 */

/** Нативный дескриптор: адрес объекта в памяти библиотеки. / A native handle: an object address in the library memory. */
export type Handle = bigint;

/** Значение NodeType. / A NodeType value. */
export type NodeTypeValue = 0 | 1 | 2 | 3 | 4 | 5 | 6;

/** Флаги разбора (объединяются через |). / Parsing flags (combined with |). */
export declare const ParseFlags: {
    readonly DEFAULT: 0;
    readonly PRESERVE_WHITESPACE: 1;
    readonly DISCARD_COMMENTS: 2;
    readonly COALESCE_CDATA: 4;
    readonly VALIDATE_DTD: 8;
    readonly NO_NAMESPACES: 16;
    readonly XINCLUDE: 32;
    readonly FILE_RESOURCES: 64;
    readonly TRACK_LOCATIONS: 128;
};

/** Флаги вывода; по умолчанию — ни одного лишнего пробела. / Output flags; by default — not a single extra space. */
export declare const WriteFlags: {
    readonly DEFAULT: 0;
    readonly PRETTY: 1;
    readonly DECLARATION: 2;
};

/** Тип узла (порядок совпадает с beresta_node_kind). / The node type (the order matches beresta_node_kind). */
export declare const NodeType: {
    readonly DOCUMENT: 0;
    readonly DOCUMENT_TYPE: 1;
    readonly ELEMENT: 2;
    readonly TEXT: 3;
    readonly CDATA_SECTION: 4;
    readonly COMMENT: 5;
    readonly PROCESSING_INSTRUCTION: 6;
};

/** Вход разбора: текст (UTF-8) или байты в любой поддерживаемой кодировке.
 *  The parse input: text (UTF-8) or bytes in any supported encoding. */
export type XmlInput = string | Uint8Array | ArrayBuffer;

/** Настройки разбора. / Parse options. */
export interface ParseOptions {
    /** Флаги ParseFlags. / ParseFlags flags. */
    flags?: number;
    /** Проверка по XML Schema во время разбора. / Validation against an XML Schema during parsing. */
    schema?: Schema | null;
}

/** Настройки записи в файл. / File writing options. */
export interface SaveOptions {
    /** Флаги WriteFlags. / WriteFlags flags. */
    flags?: number;
    /** Кодировка результата, например "windows-1251" (по умолчанию UTF-8). / The output encoding, for example "windows-1251" (UTF-8 by default). */
    encoding?: string | null;
}

/** Ошибка «Берёсты»: code одинаков во всех языках, message — на текущем.
 *  A Beresta error: code is the same in all languages, message is in the current one. */
export declare class BerestaError extends Error {
    /** Число beresta_status. / The beresta_status number. */
    readonly status: number;
    /** Имя кода: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME... / The code name: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME... */
    readonly code: string;
    constructor(status: number, code: string, message: string);
}

/** Обращение к закрытому документу, выражению или схеме. / An access to a closed document, expression or schema. */
export declare class BerestaClosedError extends Error {
    readonly code: 'OBJECT_CLOSED';
    constructor(message: string);
}

/** Нативный аддон не найден или не загрузился. / The native addon is not found or failed to load. */
export declare class BerestaLoadError extends Error {
    readonly code: 'ADDON_NOT_LOADED';
    constructor(message: string);
}

/** Узел дерева документа. / A node of the document tree. */
export declare class XmlNode implements Iterable<XmlNode> {
    protected constructor(document: XmlDocument | null, handle: Handle);
    /** Нативный дескриптор; BerestaClosedError, если документ закрыт. / The native handle; BerestaClosedError if the document is closed. */
    readonly handle: Handle;
    /** Документ-владелец. / The owner document. */
    readonly document: XmlDocument;
    /** Тип узла. / The node type. */
    readonly type: NodeTypeValue;
    /** Имя по соглашению DOM: тег, цель PI, "#text"... / The name by the DOM convention: a tag, a PI target, "#text"... */
    readonly name: string;
    /** Текстовое содержимое; запись у элемента заменяет всех детей. / The text content; writing it on an element replaces all children. */
    text: string;
    readonly parent: XmlNode | null;
    readonly firstChild: XmlNode | null;
    readonly lastChild: XmlNode | null;
    readonly nextSibling: XmlNode | null;
    readonly previousSibling: XmlNode | null;
    readonly childCount: number;
    /** Снимок списка детей. / A snapshot of the child list. */
    readonly children: XmlNode[];
    /** Ребёнок по индексу; отрицательный — с конца. / A child by index; a negative one counts from the end. */
    child(index: number): XmlNode | null;
    [Symbol.iterator](): Iterator<XmlNode>;
    /** Указывают ли обёртки на один узел. / Whether the wrappers point to the same node. */
    isSameNode(other: unknown): boolean;
    /** Сериализация с потомками. / Serialization with the descendants. */
    toXml(flags?: number): string;
    appendElement(name: string): XmlElement;
    appendText(text: string): XmlNode;
    appendComment(text: string): XmlNode;
    appendCData(text: string): XmlNode;
    appendProcessingInstruction(target: string, data?: string | null): XmlNode;
    /** Удаление с потомками; обёртка становится недействительной. / Removal with the descendants; the wrapper becomes invalid. */
    remove(): void;
    /** Глубокая копия в конец детей parent. / A deep copy appended to the children of parent. */
    copyTo<T extends XmlNode>(parent: XmlNode): T;
    /** Перенос без копирования в конец детей parent (можно в другой документ); после него
     *  document — документ parent. Корень и сам документ переносить нельзя.
     *  A move without copying to the end of the children of parent (another document is allowed);
     *  after it document is the document of parent. The root and the document itself cannot be moved. */
    moveTo(parent: XmlNode): void;
}

/** Элемент XML. / An XML element. */
export declare class XmlElement extends XmlNode {
    readonly type: 2;
    readonly localName: string;
    readonly namespaceUri: string | null;
    /** Снимок атрибутов в порядке документа. / A snapshot of the attributes in document order. */
    readonly attributes: Record<string, string>;
    getAttribute(name: string): string | null;
    setAttribute(name: string, value: string | number | boolean): this;
    hasAttribute(name: string): boolean;
    removeAttribute(name: string): boolean;
}

/** XML-документ — владелец дерева; закрывается close() или `using`. / An XML document — the tree owner; closed with close() or `using`. */
export declare class XmlDocument extends XmlNode implements Disposable {
    protected constructor(handle: Handle);
    readonly type: 0;
    readonly closed: boolean;
    readonly root: XmlElement | null;
    elementById(id: string): XmlElement | null;
    /** Независимая копия; её тоже нужно закрыть. / An independent copy; it must be closed too. */
    clone(): XmlDocument;
    save(path: string, options?: SaveOptions): void;
    close(): void;
    [Symbol.dispose](): void;
}

/** Контекст выражения XPath. / An XPath expression context. */
export interface XPathOptions {
    /** Префиксы для имён p:name. / Prefixes for p:name names. */
    namespaces?: Record<string, string>;
    /** Переменные $name. / The $name variables. */
    variables?: Record<string, string | number>;
}

/** Скомпилированное выражение XPath 1.0. / A compiled XPath 1.0 expression. */
export declare class XPath implements Disposable {
    constructor(expression: string, options?: XPathOptions);
    readonly handle: Handle;
    readonly closed: boolean;
    bindNamespace(prefix: string, uri: string): this;
    set(name: string, value: string | number | boolean): this;
    /** Узлы результата (атрибуты — через selectValues). / The result nodes (attributes — through selectValues). */
    select(context: XmlNode): XmlNode[];
    selectValues(context: XmlNode): string[];
    string(context: XmlNode): string;
    number(context: XmlNode): number;
    boolean(context: XmlNode): boolean;
    close(): void;
    [Symbol.dispose](): void;
}

/** Одно нарушение схемы. / One schema violation. */
export declare class Diagnostic {
    private constructor();
    /** Текст с путём к элементу. / The text with the element path. */
    readonly message: string;
    /** Строка в исходнике (0 — неизвестна). / The line in the source (0 — unknown). */
    readonly line: number;
    readonly column: number;
}

/** Загруженная XML Schema. / A loaded XML Schema. */
export declare class Schema implements Disposable {
    private constructor(handle: Handle);
    static fromString(input: XmlInput): Schema;
    static fromFile(path: string): Schema;
    readonly handle: Handle;
    readonly closed: boolean;
    validate(document: XmlDocument): Diagnostic[];
    /** Потоковая проверка без дерева: память O(глубины), синтаксическая ошибка — исключение.
     *  Streaming validation without a tree: O(depth) memory, a syntax error is an exception. */
    validateBytes(input: XmlInput, options?: { flags?: number }): Diagnostic[];
    /** Потоковая проверка файла (файл отображается в память). / Streaming validation of a file (the file is mapped into memory). */
    validateFile(path: string, options?: { flags?: number }): Diagnostic[];
    close(): void;
    [Symbol.dispose](): void;
}

/** Разбор XML. / Parses XML. */
export declare function parse(input: XmlInput, options?: ParseOptions): XmlDocument;
/** Разбор файла. / Parses a file. */
export declare function parseFile(path: string, options?: ParseOptions): XmlDocument;
/** Новый документ (null — без корня). / A new document (null — without a root). */
export declare function createDocument(rootName?: string | null): XmlDocument;
/** Версия нативной библиотеки. / The native library version. */
export declare function version(): string;
/** Платформа аддона, например "linux-x86_64". / The addon platform, for example "linux-x86_64". */
export declare function platformName(): string;
/** Смена языка сообщений для всего процесса. / Switches the message language for the whole process. */
export declare function setLanguage(code: string): void;
/** Код текущего языка. / The current language code. */
export declare function language(): string;
/** Все языки библиотеки. / All languages of the library. */
export declare function languages(): string[];
