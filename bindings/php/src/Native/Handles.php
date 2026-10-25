<?php

declare(strict_types=1);

namespace Beresta\Native;

use FFI;
use FFI\CData;

/**
 * Дескрипторы (void*) и выходные параметры Си-функций.
 *
 * Функции вида beresta_document_parse(..., beresta_document* out) пишут
 * результат по указателю. В PHP для этого создаётся ячейка void*
 * (Handles::slot), в функцию передаётся её адрес (Handles::out), а сама
 * ячейка после вызова и есть дескриптор: FFI передаёт её значение туда,
 * где ждут void*.
 *
 * Handles (void*) and output parameters of C functions.
 *
 * Functions like beresta_document_parse(..., beresta_document* out) write the
 * result through a pointer. In PHP a void* cell is created for that
 * (Handles::slot), its address is passed to the function (Handles::out), and the
 * cell itself is the handle after the call: FFI passes its value wherever a
 * void* is expected.
 */
final class Handles
{
    /**
     * Новая пустая ячейка void* для выходного параметра.
     * A new empty void* cell for an output parameter.
     */
    public static function slot(): CData
    {
        return Library::ffi()->new('void*');
    }

    /**
     * Адрес ячейки (void**) — то, что передаётся как out.
     * The address of the cell (void**) — what is passed as out.
     */
    public static function out(CData $slot): CData
    {
        return FFI::addr($slot);
    }

    /**
     * Ячейка double для beresta_xpath_number.
     * A double cell for beresta_xpath_number.
     */
    public static function number(): CData
    {
        return Library::ffi()->new('double');
    }

    /**
     * Адрес узла как целое: ключ карты обёрток и сравнение узлов.
     *
     * Прямое приведение void* к intptr_t (cast('intptr_t', $pointer)) в PHP
     * FFI даёт не значение указателя, а посторонний адрес — одинаковый для
     * разных узлов. Поэтому читается сама ячейка, где FFI хранит указатель:
     * её адрес (void**) приводится к intptr_t* и разыменовывается.
     *
     * The node address as an integer: the key of the wrapper map and node comparison.
     *
     * A direct cast of void* to intptr_t (cast('intptr_t', $pointer)) in PHP FFI
     * gives not the pointer value but an unrelated address — the same for different
     * nodes. So the cell where FFI keeps the pointer is read itself: its address
     * (void**) is cast to intptr_t* and dereferenced.
     */
    public static function address(CData $pointer): int
    {
        return (int) Library::ffi()->cast('intptr_t*', FFI::addr($pointer))[0];
    }
}
