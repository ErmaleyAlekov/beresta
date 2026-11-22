// Пропускаем литералы и комментарии / Skip literals and comments.
const TOKENS = /\/\/[^\n]*|\/\*[\s\S]*?\*\/|"(?:\\[\s\S]|[^"\\])*"|'(?:\\[\s\S]|[^'\\])*'|\x60(?:\\[\s\S]|[^\x60\\])*\x60|\brequire\s*\(\s*(['"])([^'"]+)\1\s*\)/g;

export function* dependencies(source)
{
    for (const match of source.matchAll(TOKENS))
        if (match[2] !== undefined)
            yield match;
}
