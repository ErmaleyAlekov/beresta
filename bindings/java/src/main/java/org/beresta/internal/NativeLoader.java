package org.beresta.internal;

import java.io.IOException;
import java.io.InputStream;
import java.lang.foreign.Arena;
import java.lang.foreign.SymbolLookup;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.Locale;

/**
 * Поиск и загрузка динамической библиотеки «Берёсты».
 *
 * <p>Порядок поиска:
 * <ol>
 *   <li>системное свойство {@code beresta.library.path} — явный путь к файлу;</li>
 *   <li>ресурс jar {@code /natives/<os>-<arch>/<файл>} — туда бинарник кладёт
 *       сборка (CMake {@code -DBERESTA_BUILD_JAVA=ON} или {@code make JAVA=1});
 *       он извлекается во временный каталог, потому что ОС умеет загружать
 *       библиотеки только из файла.</li>
 * </ol>
 *
 * <p>Имена ОС и архитектуры совпадают с CMake, Makefile и Python-обёрткой:
 * windows / linux / macos и x86_64 / aarch64 / x86.
 *
 * <p>Finding and loading the Beresta native library.
 * Lookup order:
 * <ol>
 *   <li>the system property {@code beresta.library.path} — an explicit file path;</li>
 *   <li>the jar resource {@code /natives/<os>-<arch>/<file>} — the build puts the binary
 *       there (CMake {@code -DBERESTA_BUILD_JAVA=ON} or {@code make JAVA=1});
 *       it is extracted into a temporary directory, because the OS can load
 *       libraries only from a file.</li>
 * </ol>
 * The OS and architecture names match CMake, the Makefile and the Python wrapper:
 * windows / linux / macos and x86_64 / aarch64 / x86.
 */
public final class NativeLoader
{
    private NativeLoader()
    {
    }

    /**
     * Имя платформы вида {@code windows-x86_64} — имя папки ресурсов.
     * <p>The platform name like {@code windows-x86_64} — the resource folder name.
     */
    public static String platformName()
    {
        String os = System.getProperty("os.name").toLowerCase(Locale.ROOT);
        String arch = System.getProperty("os.arch").toLowerCase(Locale.ROOT);
        String system = os.replace(' ', '_');

        if (os.startsWith("windows"))
            system = "windows";
        else if (os.startsWith("mac") || os.startsWith("darwin"))
            system = "macos";
        else if (os.startsWith("linux"))
            system = "linux";
        if (arch.equals("amd64") || arch.equals("x86_64") || arch.equals("x64"))
            arch = "x86_64";
        else if (arch.equals("aarch64") || arch.equals("arm64"))
            arch = "aarch64";
        else if (arch.equals("x86") || arch.equals("i386") || arch.equals("i686"))
            arch = "x86";
        return system + "-" + arch;
    }

    /**
     * Каноническое имя файла библиотеки для ОС (первая часть имени платформы).
     * <p>The canonical library file name for the OS (the first part of the platform name).
     */
    static String libraryFileName(String platform)
    {
        if (platform.startsWith("windows-"))
            return "beresta.dll";
        if (platform.startsWith("macos-"))
            return "libberesta.dylib";
        return "libberesta.so";
    }

    /**
     * Извлечение ресурса во временный файл. Каталог уникален для процесса:
     * два приложения с разными версиями библиотеки не мешают друг другу.
     *
     * <p>Extracts a resource into a temporary file. The directory is unique per process:
     * two applications with different library versions do not interfere.
     */
    static Path extract(String resource, String fileName) throws IOException
    {
        try (InputStream stream = NativeLoader.class.getResourceAsStream(resource))
        {
            if (stream == null)
                throw new UnsatisfiedLinkError("Берёста: в ресурсах нет " + resource
                    + ". Соберите библиотеку с флагом JAVA=1 (make) или -DBERESTA_BUILD_JAVA=ON (CMake)"
                    + " либо укажите путь: -Dberesta.library.path=... / Beresta: the resources have no "
                    + resource + ". Build the library with JAVA=1 (make) or -DBERESTA_BUILD_JAVA=ON (CMake)"
                    + " or set the path: -Dberesta.library.path=...");
            Path directory = Files.createTempDirectory("beresta-");
            Path file = directory.resolve(fileName);

            Files.copy(stream, file, StandardCopyOption.REPLACE_EXISTING);
            file.toFile().deleteOnExit();
            directory.toFile().deleteOnExit();
            return file;
        }
    }

    /**
     * Загрузка библиотеки. Arena.global(): библиотека остаётся загруженной
     * до конца работы JVM — дескрипторы функций живут в статических полях.
     *
     * <p>Loads the library. Arena.global(): the library stays loaded until the JVM
     * exits — function handles live in static fields.
     */
    public static SymbolLookup load()
    {
        String explicit = System.getProperty("beresta.library.path");

        try
        {
            Path file;
            if (explicit != null)
                file = Path.of(explicit);
            else
            {
                String platform = platformName();
                String name = libraryFileName(platform);
                file = extract("/natives/" + platform + "/" + name, name);
            }
            return SymbolLookup.libraryLookup(file, Arena.global());
        }
        catch (IOException error)
        {
            throw new UnsatisfiedLinkError("Берёста: не удалось извлечь библиотеку / Beresta: cannot extract the library: "
                + error.getMessage());
        }
    }
}
