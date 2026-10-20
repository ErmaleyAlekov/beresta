"""Сведения о документе PDF: словарь /Info, метаданные XMP и OutputIntent для PDF/A-1b.

Роспатент принимает депонируемые материалы в PDF/A (п. 29 Требований, приказ
Минэкономразвития № 211). Для отметки PDF/A-1b нужны: встроенные шрифты, метаданные XMP
(те же название и даты, что в /Info) и OutputIntent с профилем ICC. Профиль sRGB берётся
из системы; если его нет, файл остаётся обычным PDF без отметки PDF/A.

The PDF document information: the /Info dictionary, the XMP metadata and the OutputIntent for PDF/A-1b.
Rospatent accepts deposit materials in PDF/A (clause 29 of the Requirements, Order No. 211 of the
Ministry of Economic Development). The PDF/A-1b mark needs: embedded fonts, XMP metadata (the same
title and dates as in /Info) and an OutputIntent with an ICC profile. The sRGB profile is taken from
the system; if there is none, the file stays an ordinary PDF without the PDF/A mark.
"""

from __future__ import annotations

from datetime import datetime
from pathlib import Path
from xml.sax.saxutils import escape

from .pdf_writer import PdfWriter

# Где искать профиль sRGB на Windows, Linux и macOS / Where to look for the sRGB profile on Windows, Linux and macOS
ICC_CANDIDATES = (
    "C:/Windows/System32/spool/drivers/color/sRGB Color Space Profile.icm",
    "/usr/share/color/icc/colord/sRGB.icc",
    "/usr/share/color/icc/sRGB.icc",
    "/usr/share/color/icc/ghostscript/srgb.icc",
    "/System/Library/ColorSync/Profiles/sRGB Profile.icc",
)

PRODUCER = "Beresta tools/legal/make_listing.py"


def pdf_text(value: str) -> str:
    """Текстовая строка PDF в UTF-16BE с меткой порядка байтов (шестнадцатеричная форма).
    A PDF text string in UTF-16BE with the byte order mark (the hexadecimal form)."""
    return "<FEFF" + value.encode("utf-16-be").hex().upper() + ">"


def find_icc(explicit: str) -> Path:
    """Профиль ICC: явно указанный или первый найденный системный; None — профиля нет.
    The ICC profile: the explicitly given one or the first system one found; None means there is none."""
    candidates = list(ICC_CANDIDATES)
    if explicit:
        candidates = [explicit]
    for candidate in candidates:
        if Path(candidate).is_file():
            return Path(candidate)
    return None


def xmp_packet(title: str, moment: datetime, pdfa: bool) -> bytes:
    """Пакет XMP: название, даты, программа-создатель и (если pdfa) отметка PDF/A-1b.
    The XMP packet: the title, the dates, the producer and (if pdfa) the PDF/A-1b mark."""
    stamp = moment.strftime("%Y-%m-%dT%H:%M:%S+00:00")
    mark = ""
    if pdfa:
        mark = ('<rdf:Description rdf:about="" xmlns:pdfaid="http://www.aiim.org/pdfa/ns/id/">'
                "<pdfaid:part>1</pdfaid:part><pdfaid:conformance>B</pdfaid:conformance></rdf:Description>")
    body = (
        '<?xpacket begin="\ufeff" id="W5M0MpCehiHzreSzNTczkc9d"?>'
        '<x:xmpmeta xmlns:x="adobe:ns:meta/"><rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">'
        '<rdf:Description rdf:about="" xmlns:dc="http://purl.org/dc/elements/1.1/">'
        '<dc:title><rdf:Alt><rdf:li xml:lang="x-default">' + escape(title) + "</rdf:li></rdf:Alt></dc:title>"
        "<dc:format>application/pdf</dc:format></rdf:Description>"
        '<rdf:Description rdf:about="" xmlns:xmp="http://ns.adobe.com/xap/1.0/">'
        "<xmp:CreateDate>" + stamp + "</xmp:CreateDate><xmp:ModifyDate>" + stamp + "</xmp:ModifyDate>"
        "<xmp:CreatorTool>" + PRODUCER + "</xmp:CreatorTool></rdf:Description>"
        '<rdf:Description rdf:about="" xmlns:pdf="http://ns.adobe.com/pdf/1.3/">'
        "<pdf:Producer>" + PRODUCER + "</pdf:Producer></rdf:Description>"
        + mark + '</rdf:RDF></x:xmpmeta><?xpacket end="w"?>'
    )
    return body.encode("utf-8")


def write_metadata(writer: PdfWriter, title: str, moment: datetime, icc: Path) -> dict:
    """Объекты /Info, XMP и (при наличии профиля) OutputIntent; номера — в словаре.
    The /Info, XMP and (when a profile exists) OutputIntent objects; the numbers are in a dictionary."""
    stamp = moment.strftime("D:%Y%m%d%H%M%S+00'00'")
    numbers = {"intent": None}
    numbers["info"] = writer.add((
        "<< /Title %s /Creator %s /Producer %s /CreationDate (%s) /ModDate (%s) >>"
        % (pdf_text(title), pdf_text(PRODUCER), pdf_text(PRODUCER), stamp, stamp)).encode("ascii"))
    # Метаданные XMP не сжимаются: их должны читать программы, не знающие PDF
    # The XMP metadata is not compressed: programs that do not know PDF must be able to read it
    numbers["metadata"] = writer.add(writer.stream(
        "/Type /Metadata /Subtype /XML", xmp_packet(title, moment, icc is not None), compress=False))
    if icc is not None:
        profile = writer.add(writer.stream("/N 3", icc.read_bytes()))
        numbers["intent"] = writer.add((
            "<< /Type /OutputIntent /S /GTS_PDFA1 /OutputConditionIdentifier (sRGB IEC61966-2.1)"
            " /Info (sRGB IEC61966-2.1) /DestOutputProfile %d 0 R >>" % profile).encode("ascii"))
    return numbers
