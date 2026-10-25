<?php

declare(strict_types=1);

/*
 * Автозагрузчик без Composer: для запуска тестов и примеров прямо из
 * репозитория (require_once 'bindings/php/autoload.php'). Повторяет карту
 * PSR-4 из composer.json: публичные классы лежат в пространстве имён Beresta,
 * но разложены по папкам по смыслу (Dom, Parsing, XPath, Schema, Errors,
 * Library), а внутренние — в src/Native и подпапках Internal. Класс ищется
 * в каждой папке по очереди, первый найденный файл побеждает.
 *
 * An autoloader without Composer: for running the tests and examples straight
 * from the repository (require_once 'bindings/php/autoload.php'). It repeats the
 * PSR-4 map from composer.json: the public classes live in the Beresta namespace
 * but are spread over folders by meaning (Dom, Parsing, XPath, Schema, Errors,
 * Library), and the internal ones are in src/Native and the Internal subfolders.
 * A class is looked up in every folder in turn, the first file found wins.
 */

namespace Beresta\Autoload;

/**
 * Загрузка класса Beresta\... из одной из папок src/.
 * Loads a Beresta\... class from one of the src/ folders.
 */
function load(string $class): void
{
    // Чужие классы пропускаем: ими занимаются другие автозагрузчики
    // Foreign classes are skipped: other autoloaders take care of them
    $prefix = 'Beresta\\';
    if (!str_starts_with($class, $prefix)) {
        return;
    }
    $relative = str_replace('\\', '/', substr($class, strlen($prefix))) . '.php';
    $folders = ['Dom', 'Parsing', 'XPath', 'Schema', 'Errors', 'Library', ''];
    foreach ($folders as $folder) {
        $path = __DIR__ . '/src/' . $folder . '/' . $relative;
        if (is_file($path)) {
            require $path;
            return;
        }
    }
}

spl_autoload_register(__NAMESPACE__ . '\\load');
