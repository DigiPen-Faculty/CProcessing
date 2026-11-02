"""Copy a GitHub wiki into MkDocs and normalize GitHub-specific links."""

from __future__ import annotations

import argparse
import re
import shutil
from pathlib import Path


MARKDOWN_LINK = re.compile(r"(?P<prefix>!?\[[^\]]*\]\()(?P<target>[^)]+)(?P<suffix>\))")


def normalize_target(target: str, page_names: set[str]) -> str:
    """Convert GitHub wiki page links and fragments to portable MkDocs links."""
    if target.startswith(("http://", "https://", "mailto:", "#")):
        if target.startswith("#"):
            return "#" + target[1:].lower()
        return target

    path, marker, fragment = target.partition("#")
    page = path.rstrip("/")
    if page in page_names:
        path = f"{page}.md"
    if marker:
        return f"{path}#{fragment.lower()}"
    return path


def prepare(source: Path, destination: Path) -> None:
    if not source.is_dir():
        raise SystemExit(f"Wiki source does not exist: {source}")

    if destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True)

    pages = sorted(source.glob("*.md"))
    if not pages:
        raise SystemExit(f"No Markdown pages found in {source}")
    page_names = {page.stem for page in pages}

    for source_page in pages:
        text = source_page.read_text(encoding="utf-8")
        text = MARKDOWN_LINK.sub(
            lambda match: (
                match.group("prefix")
                + normalize_target(match.group("target"), page_names)
                + match.group("suffix")
            ),
            text,
        )
        output_name = "index.md" if source_page.name == "Home.md" else source_page.name
        (destination / output_name).write_text(text, encoding="utf-8", newline="\n")

    for item in source.iterdir():
        if item.name == ".git" or item.suffix.lower() == ".md":
            continue
        target = destination / item.name
        if item.is_dir():
            shutil.copytree(item, target)
        else:
            shutil.copy2(item, target)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    prepare(args.source.resolve(), args.destination.resolve())


if __name__ == "__main__":
    main()
