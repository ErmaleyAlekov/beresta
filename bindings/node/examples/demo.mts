/*
 * Пример на TypeScript. Node.js 23.6+ запускает его без компилятора
 * (встроенное удаление типов): node examples/demo.mts
 * Типы берутся из lib/index.d.ts — редактор подсказывает методы, а
 * опечатка в имени или неверный тип аргумента видны ещё до запуска.
 *
 * A TypeScript example. Node.js 23.6+ runs it without a compiler
 * (built-in type stripping): node examples/demo.mts
 * The types come from lib/index.d.ts — the editor suggests methods, and a
 * typo in a name or a wrong argument type is visible before the run.
 */

import { parse, createDocument, XPath, WriteFlags, BerestaError } from '../lib/index.js';
import type { XmlDocument, XmlElement } from '../lib/index.js';

// Сумма цен всех товаров через XPath / The sum of all item prices through XPath
function totalPrice(document: XmlDocument): number
{
    using query = new XPath('sum(//item/@price)');

    return query.number(document);
}

// Новый заказ, построенный цепочкой вызовов / A new order built with a chain of calls
function buildOrder(): string
{
    using order = createDocument('order');
    const item: XmlElement = order.root!.appendElement('item').setAttribute('qty', 2);

    item.appendText('Мёд & орехи');
    return order.toXml(WriteFlags.PRETTY);
}

{
    using shop = parse("<shop><item price='120'>Чай</item><item price='80'>Кофе</item></shop>");

    console.log('Сумма / Total:', totalPrice(shop));
    console.log(buildOrder());
}

try
{
    parse('<a><b></a>');
}
catch (error)
{
    if (error instanceof BerestaError)
        console.log(error.code, error.message);
}
