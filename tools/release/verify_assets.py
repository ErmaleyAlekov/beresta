"""Проверка файлов зеркала / Verify staged release assets before publishing."""

import hashlib
import re
import sys
from pathlib import Path


def file_digest(path):
    digest = hashlib.sha256()
    with path.open("rb") as asset:
        while block := asset.read(1024 * 1024):
            digest.update(block)
    return digest.hexdigest()


def manifest_entries(manifest):
    result = {}
    for entry in manifest.read_text(encoding="utf-8").splitlines():
        record = re.fullmatch(r"([0-9a-f]{64})  ([A-Za-z0-9_][A-Za-z0-9_.+-]*)", entry)
        if not record or record[2] in result:
            sys.exit("Некорректный манифест / Invalid SHA256SUMS.txt")
        result[record[2]] = record[1]
    return result


def verify(urls, folder):
    manifest = folder / "SHA256SUMS.txt"
    assets = {}
    for entry in urls.read_text(encoding="utf-8").splitlines():
        url, *metadata = entry.split()
        assets[url.rsplit("/", 1)[-1]] = metadata
    checksums = {}
    if manifest.exists():
        checksums = manifest_entries(manifest)
        if set(checksums) != assets.keys() - {manifest.name}:
            sys.exit("Состав релиза отличается / Release and manifest files differ")
    for name, metadata in assets.items():
        actual = file_digest(folder / name)
        if metadata not in ([], ["-"]) and metadata != ["sha256:" + actual]:
            sys.exit(f"Ошибка SHA-256 GitHub / GitHub digest mismatch: {name}")
        if name != manifest.name and (name in checksums or metadata in ([], ["-"])):
            if checksums.get(name) != actual:
                sys.exit(f"Ошибка SHA-256 / SHA-256 verification failed: {name}")


if __name__ == "__main__":
    verify(Path(sys.argv[1]), Path(sys.argv[2]))
