package bench;

/**
 * Библиотека XML под замером (паттерн Strategy): разбор байтов в дерево,
 * обход дерева, запрос XPath и разбор с проверкой по схеме. Документ,
 * который умеет закрываться (AutoCloseable), закрывает сама программа замеров.
 *
 * An XML library under measurement (the Strategy pattern): parsing bytes into a tree,
 * walking the tree, an XPath query and parsing with schema validation. A document
 * that can be closed (AutoCloseable) is closed by the benchmark program itself.
 */
public abstract class Engine
{
    private final String name;

    /** Имя для таблицы результатов. / The name for the results table. */
    protected Engine(String name)
    {
        this.name = name;
    }

    /** Имя библиотеки. / The library name. */
    public final String name()
    {
        return name;
    }

    /** Разбор в дерево. / Parsing into a tree. */
    public abstract Object parse(byte[] xml) throws Exception;

    /** Обход всего дерева; результат — число элементов. / Walking the whole tree; the result is the element count. */
    public abstract int walk(Object document);

    /** Запрос XPath; результат — число найденных узлов. / An XPath query; the result is the found node count. */
    public abstract int xpath(Object document) throws Exception;

    /** Разбор в дерево с проверкой по схеме; результат — число ошибок. / Parsing into a tree with schema validation; the result is the error count. */
    public abstract int parseValidated(byte[] xml) throws Exception;
}
