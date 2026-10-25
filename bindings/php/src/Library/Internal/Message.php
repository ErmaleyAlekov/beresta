<?php

declare(strict_types=1);

namespace Beresta\Internal;

/**
 * Номера сообщений обёрток в каталоге библиотеки (beresta_message).
 *
 * Тексты не пишутся в коде PHP: Language::message() берёт их у библиотеки
 * на текущем языке, поэтому новый язык каталога сразу доступен и здесь.
 *
 * The numbers of the binding messages in the library catalog (beresta_message).
 *
 * The texts are not written in the PHP code: Language::message() takes them from
 * the library in the current language, so a new catalog language is available
 * here at once.
 *
 * @internal
 */
enum Message: int
{
    case DocumentClosed = 1;
    case XPathClosed = 2;
    case SchemaClosed = 3;
    case NativeCallFailed = 4;
    case UnknownNodeType = 5;
    case InvalidArgument = 6;
    case NoFileSystem = 7;
}
