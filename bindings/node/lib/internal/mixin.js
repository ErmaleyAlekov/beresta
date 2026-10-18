'use strict';

/*
 * Примеси (mixin): методы узла разнесены по небольшим классам по смыслу —
 * содержимое, навигация, дети, изменение дерева. Это принцип разделения
 * интерфейсов (ISP из SOLID): каждая часть отвечает за одну обязанность и
 * читается отдельно, а итоговый класс лишь собирает их. JavaScript
 * наследует только от одного класса, поэтому методы копируются в прототип
 * вместе с геттерами и сеттерами (дескрипторы свойств, а не значения).
 *
 * Mixins: the node methods are split into small classes by meaning —
 * content, navigation, children, tree changes. This is the interface
 * segregation principle (ISP from SOLID): each part has one responsibility
 * and reads separately, while the final class only combines them. JavaScript
 * inherits from one class only, so the methods are copied into the prototype
 * together with getters and setters (property descriptors, not values).
 */
function applyMixins(target, ...sources)
{
    for (const source of sources)
    {
        const descriptors = Object.getOwnPropertyDescriptors(source.prototype);

        delete descriptors.constructor;
        Object.defineProperties(target.prototype, descriptors);
    }
    return target;
}

module.exports = { applyMixins };
