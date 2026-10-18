'use strict';

/*
 * Временная память одного вызова. Строки и байты из JavaScript копируются
 * в память Wasm (malloc), служебные ячейки выделяются под выходные
 * значения; после вызова всё освобождается разом (release в finally).
 * Так вызов не оставляет утечек даже при исключении.
 *
 * Важно: после любого malloc память Wasm может вырасти, и старое
 * представление HEAPU8 станет недействительным, — поэтому оно каждый раз
 * берётся заново у модуля, а не хранится.
 *
 * The temporary memory of one call. Strings and bytes from JavaScript are copied
 * into the Wasm memory (malloc), service slots are allocated for output
 * values; after the call everything is freed at once (release in finally).
 * This way a call leaves no leaks even with an exception.
 *
 * Important: after any malloc the Wasm memory may grow, and the old
 * HEAPU8 view becomes invalid — that is why it is taken from the module anew
 * every time instead of being stored.
 */
class CallScope
{
    constructor(core)
    {
        this.core = core;
        this.pointers = [];
        this.slot = 0;
    }

    // Строка UTF-8 с завершающим нулём / A UTF-8 string with a terminating zero
    putString(text)
    {
        const size = this.core.lengthBytesUTF8(text) + 1;
        const pointer = this.allocate(size);

        this.core.stringToUTF8(text, pointer, size);
        return pointer;
    }

    // Копия байтов; пустой массив всё равно получает 1 байт, чтобы указатель не был NULL
    // A copy of the bytes; an empty array still gets 1 byte so that the pointer is not NULL
    putBytes(bytes)
    {
        const pointer = this.allocate(Math.max(bytes.length, 1));

        this.core.HEAPU8.set(bytes, pointer);
        return pointer;
    }

    // Выделение с проверкой: нехватка памяти Wasm — понятная ошибка, а не запись по NULL
    // An allocation with a check: running out of Wasm memory is a clear error, not a write to NULL
    allocate(size)
    {
        const pointer = this.core._malloc(size);

        if (pointer === 0)
            throw new RangeError('Не хватает памяти WebAssembly / Out of WebAssembly memory');
        this.pointers.push(pointer);
        return pointer;
    }

    // Освобождение всего, что выделил вызов / Frees everything the call allocated
    release()
    {
        for (const pointer of this.pointers)
            this.core._free(pointer);
        this.pointers.length = 0;
    }
}

module.exports = { CallScope };
