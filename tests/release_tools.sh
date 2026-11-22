#!/usr/bin/env bash
# Автономная проверка инструментов выпуска / Offline release-tool regression checks.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT
mkdir -p "$work/repo/tools/release" "$work/bin" "$work/tmp" "$work/dist"
cp "$root/tools/release/release_notes.sh" "$work/repo/tools/release/"
cd "$work"
printf '## 2.2.1 — date\nwrong\n## 2.1 — date\nright\n' > repo/CHANGELOG.md
cp repo/CHANGELOG.md repo/CHANGELOG.en.md
if bash repo/tools/release/release_notes.sh 2.2 notes; then exit 1; fi
bash repo/tools/release/release_notes.sh 2.1 notes
test "$(grep -c right notes)" = 2

# Подменяем только внешние программы / Replace only the external programs.
cat > bin/curl <<'SH'
#!/usr/bin/env bash
set -eu
method=GET
while (($# > 1)); do
    case "$1" in
        -X) method="$2"; shift ;;
        -o) out="$2"; shift ;;
    esac
    shift
done
echo "$method $1" >> calls
case "$method $1" in
    'GET '*/tags/*) echo '{"id":1}' > "$out"; printf %s "$LOOKUP" ;;
    'POST '*/releases) echo '{"id":1}' > "$out"; printf 201 ;;
    'GET '*/assets) echo '[]' > "$out"; printf 200 ;;
    'POST '*'/assets?name='*) : > "$out"; printf %s "$UPLOAD" ;;
    *) exit 1 ;;
esac
SH
cat > bin/zip <<'SH'
#!/usr/bin/env bash
cp "$3" "$2"
SH
chmod +x bin/curl bin/zip
export PATH="$work/bin:$PATH" GV_API_TOKEN=dummy TMPDIR="$work/tmp" PYTHON="${PYTHON:-python3}"
gitverse="$root/tools/release/gitverse_release.sh"
if LOOKUP=502 bash "$gitverse" v1 notes; then exit 1; fi
test "$(wc -l < calls)" -eq 1
echo payload > dist/sample.nupkg
LOOKUP=404 UPLOAD=201 bash "$gitverse" v1 notes dist/sample.nupkg
grep -q 'POST .*/releases$' calls
grep -q 'name=sample.nupkg.zip$' calls
if LOOKUP=200 UPLOAD=500 bash "$gitverse" v1 notes dist/sample.nupkg; then exit 1; fi
test -z "$(ls -A tmp)"
test ! -e dist/sample.nupkg.zip

digest="$(sha256sum dist/sample.nupkg | cut -d' ' -f1)"
printf 'https://example.test/sample.nupkg sha256:%s\n' "$digest" > urls
"$PYTHON" "$root/tools/release/verify_assets.py" urls dist
printf '%s  sample.nupkg\n' "$digest" > dist/SHA256SUMS.txt
printf 'https://example.test/sample.nupkg\nhttps://example.test/SHA256SUMS.txt\n' > urls
"$PYTHON" "$root/tools/release/verify_assets.py" urls dist
printf '%s  missing.zip\n' "$digest" >> dist/SHA256SUMS.txt
if "$PYTHON" "$root/tools/release/verify_assets.py" urls dist; then exit 1; fi
printf '%064d  sample.nupkg\n' 0 > dist/SHA256SUMS.txt
printf 'https://example.test/sample.nupkg sha256:%s\nhttps://example.test/SHA256SUMS.txt\n' "$digest" > urls
if "$PYTHON" "$root/tools/release/verify_assets.py" urls dist; then exit 1; fi
printf '%s  sample.nupkg\n' "$digest" > dist/SHA256SUMS.txt
echo changed >> dist/sample.nupkg
if "$PYTHON" "$root/tools/release/verify_assets.py" urls dist; then exit 1; fi

# На выход попадают только проверенные файлы / Only verified files reach the destination.
cat > bin/curl <<'SH'
#!/usr/bin/env bash
while (($# > 1)); do
    if [[ "$1" = -o ]]; then out="$2"; shift; fi
    shift
done
case "$1" in
    https://api.github.com/*)
        printf '{"assets":[{"browser_download_url":"https://example.test/pkg.zip","digest":"sha256:%s"}]}' "$HASH" > "$out"
        printf 200 ;;
    https://example.test/pkg.zip) cp dist/sample.nupkg "$out" ;;
    *) exit 1 ;;
esac
SH
mirror="$root/tools/release/github_assets.sh"
if HASH="$digest" bash "$mirror" v1 mirror 0; then exit 1; fi
test ! -e mirror/pkg.zip
HASH="$(sha256sum dist/sample.nupkg | cut -d' ' -f1)" bash "$mirror" v1 mirror 0
cmp dist/sample.nupkg mirror/pkg.zip
test -z "$(ls -A tmp)"
