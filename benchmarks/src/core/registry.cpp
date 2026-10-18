#include "bench/engine.hpp"
#include "bench/engines/beresta_engine.hpp"

/*
** Сторонние библиотеки подключаются, только если сборка их нашла
** (pkg-config в Makefile, pkg_check_modules в CMake): программа замеров
** собирается и без них — тогда она меряет одну «Берёсту».
**
** Third-party libraries are included only if the build found them
** (pkg-config in the Makefile, pkg_check_modules in CMake): the benchmark
** builds without them too — then it measures Beresta alone.
*/
#if defined(BENCH_WITH_LIBXML2)
    #include "bench/engines/libxml2_engine.hpp"
#endif
#if defined(BENCH_WITH_PUGIXML)
    #include "bench/engines/pugixml_engine.hpp"
#endif
#if defined(BENCH_WITH_XERCES)
    #include "bench/engines/xerces_engine.hpp"
#endif
#if defined(BENCH_WITH_TINYXML2)
    #include "bench/engines/tinyxml2_engine.hpp"
#endif
#if defined(BENCH_WITH_RAPIDXML)
    #include "bench/engines/rapidxml_engine.hpp"
#endif

namespace bench
{
    // Все доступные библиотеки; «Берёста» всегда первая / All available libraries; Beresta is always the first
    std::vector<std::unique_ptr<XmlEngine>> createEngines()
    {
        std::vector<std::unique_ptr<XmlEngine>> engines;

        engines.push_back(std::make_unique<BerestaEngine>());
#if defined(BENCH_WITH_LIBXML2)
        engines.push_back(std::make_unique<Libxml2Engine>());
#endif
#if defined(BENCH_WITH_XERCES)
        engines.push_back(std::make_unique<XercesEngine>());
#endif
#if defined(BENCH_WITH_PUGIXML)
        engines.push_back(std::make_unique<PugixmlEngine>());
#endif
#if defined(BENCH_WITH_TINYXML2)
        engines.push_back(std::make_unique<Tinyxml2Engine>());
#endif
#if defined(BENCH_WITH_RAPIDXML)
        engines.push_back(std::make_unique<RapidxmlEngine>());
#endif
        return engines;
    }
}
