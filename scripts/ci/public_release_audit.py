#!/usr/bin/env python3
"""Reject local data and credentials from files intended for public push."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import PurePosixPath


TEXT_CHUNKS = {b"tEXt", b"zTXt", b"iTXt", b"eXIf"}
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
TOKEN_PREFIX_PATTERN = "|".join(
    re.escape(prefix)
    for prefix in ("gh" + "p_", "github" + "_pat_", "h" + "f_", "s" + "k-")
)

PATTERNS = (
    ("local Unix home path", re.compile(r"/(?:home|Users)/[^/\s\"']+", re.IGNORECASE)),
    ("local Windows home path", re.compile(r"[A-Z]:\\Users\\[^\\\s\"']+", re.IGNORECASE)),
    ("private key", re.compile(r"-----BEGIN [A-Z ]*PRIVATE KEY-----")),
    (
        "access token",
        re.compile(r"(?:" + TOKEN_PREFIX_PATTERN + r")[A-Za-z0-9_]{16,}"),
    ),
    (
        "credential assignment",
        re.compile(
            r"(?:api[_-]?key|secret|password|access[_-]?token)\s*[:=]\s*"
            r"(?!\$\{\{|<)[^\s\"']{8,}",
            re.IGNORECASE,
        ),
    ),
    ("email address", re.compile(r"[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}")),
)
IP_RE = re.compile(r"\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b")


def git(*args: str) -> bytes:
    return subprocess.run(
        ["git", *args], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    ).stdout


def png_has_text_metadata(data: bytes) -> bool:
    if not data.startswith(PNG_SIGNATURE):
        return True
    offset = len(PNG_SIGNATURE)
    while offset + 12 <= len(data):
        length = int.from_bytes(data[offset : offset + 4], "big")
        chunk_type = data[offset + 4 : offset + 8]
        offset += 12 + length
        if offset > len(data):
            return True
        if chunk_type in TEXT_CHUNKS:
            return True
        if chunk_type == b"IEND":
            return offset == len(data)
    return True


def contains_publication_risk(data: bytes) -> str | None:
    if b"\0" in data:
        return "binary content is not allowed outside sanitized PNG reports"
    text = data.decode("utf-8", errors="replace")
    for name, pattern in PATTERNS:
        if pattern.search(text):
            return name
    for match in IP_RE.finditer(text):
        octets = [int(part) for part in match.group().split(".")]
        if all(part <= 255 for part in octets) and match.group() not in {
            "0.0.0.0",
            "127.0.0.1",
        }:
            return "IP address"
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--range", default=None)
    mode.add_argument("--staged", action="store_true")
    args = parser.parse_args()

    if args.staged:
        paths = git("diff", "--cached", "--name-only", "--diff-filter=ACMR").decode().splitlines()
        content_ref = ":{}"
    else:
        diff_range = args.range or "HEAD^..HEAD"
        paths = git("diff", "--name-only", "--diff-filter=ACMR", diff_range).decode().splitlines()
        content_ref = "HEAD:{}"

    failures: list[tuple[str, str]] = []
    for path in paths:
        data = git("show", content_ref.format(path))
        suffix = PurePosixPath(path).suffix.lower()
        if suffix == ".png":
            risk = "PNG metadata or structure" if png_has_text_metadata(data) else None
        else:
            risk = contains_publication_risk(data)
        if risk:
            failures.append((path, risk))

    if failures:
        print("public release audit failed:", file=sys.stderr)
        for path, risk in failures:
            print(f"- {path}: {risk}", file=sys.stderr)
        return 1
    print(f"public release audit passed for {len(paths)} changed file(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
