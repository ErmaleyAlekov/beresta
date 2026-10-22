#ifndef FAILING_LIST_ERROR_HPP
#define FAILING_LIST_ERROR_HPP

#include "xsts/xsts_error.hpp"

#include <string>

namespace failing
{
    /*
    ** Ошибка окружения отладчика провалов: нет или испорчен список
    ** failing_cases.tsv, неверные аргументы, случая нет в наборе.
    ** Наследует xsts::XstsError: main ловит один тип для всех сбоев
    ** инфраструктуры (своих и прогонщика tests/xsts) и отличает их
    ** от вердиктов по случаям.
    **
    ** An environment error of the failure debugger: the failing_cases.tsv
    ** list is missing or broken, invalid arguments, a case is absent from the suite.
    ** It inherits xsts::XstsError: main catches one type for all infrastructure
    ** failures (its own and the tests/xsts runner's) and tells them apart
    ** from the case verdicts.
    */
    class ListError : public xsts::XstsError
    {
    public:
        explicit ListError(const std::string& message);
    };
}

#endif
