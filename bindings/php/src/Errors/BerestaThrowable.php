<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Общий признак всех исключений пакета: catch (BerestaThrowable) ловит
 * и сбои библиотеки, и ошибки загрузки, и обращение к закрытому документу.
 *
 * The common marker of all package exceptions: catch (BerestaThrowable) catches
 * library failures, loading errors and access to a closed document alike.
 */
interface BerestaThrowable extends \Throwable
{
}
