"""Copy a GitHub wiki into MkDocs and normalize GitHub-specific links.

With --config, also write an MkDocs config: the given one, with its nav taken
from the wiki's _Sidebar.md, so the offline documentation is organized the
way the wiki's sidebar is.
"""

from __future__ import annotations

import argparse
import re
import shutil
from pathlib import Path

import yaml


MARKDOWN_LINK = re.compile(r"(?P<prefix>!?\[[^\]]*\]\()(?P<target>[^)]+)(?P<suffix>\))")


def page_file(page: str) -> str:
    """The MkDocs file for a wiki page: Home becomes the site's index"""
    return "index.md" if page == "Home" else f"{page}.md"


def normalize_target(target: str, page_names: set[str]) -> str:
    """Convert GitHub wiki page links and fragments to portable MkDocs links."""
    if target.startswith(("http://", "https://", "mailto:", "#")):
        if target.startswith("#"):
            return "#" + target[1:].lower()
        return target

    path, marker, fragment = target.partition("#")
    page = path.rstrip("/")
    if page in page_names:
        path = page_file(page)
    if marker:
        return f"{path}#{fragment.lower()}"
    return path


def sidebar_nav(sidebar: Path) -> list:
    """The MkDocs nav for a sidebar of bold section names and bulleted links:

        **[Home](Home)**            a page at the top level
        **Getting started**         a section
        * [Your first program](Getting-Started)
    """
    nav: list = []
    section = None
    for line in sidebar.read_text(encoding="utf-8").splitlines():
        heading = re.match(r"^\*\*(.+?)\*\*\s*$", line.strip())
        item = re.match(r"^\s*[*-]\s+\[(.+?)\]\(([^)#\s]+)[^)]*\)", line)
        if heading:
            link = re.match(r"^\[(.+?)\]\(([^)#\s]+)[^)]*\)$", heading.group(1))
            if link:
                nav.append({link.group(1): page_file(link.group(2))})
                section = None
            else:
                section = []
                nav.append({heading.group(1): section})
        elif item and not item.group(2).startswith(("http://", "https://")):
            entry = {item.group(1): page_file(item.group(2))}
            (nav if section is None else section).append(entry)
    return nav


def write_config(base: Path, output: Path, docs_dir: Path, site_dir: Path, nav: list | None) -> None:
    config = yaml.safe_load(base.read_text(encoding="utf-8"))
    config["docs_dir"] = str(docs_dir)
    config["site_dir"] = str(site_dir)
    if nav:
        config["nav"] = nav
    output.write_text(yaml.safe_dump(config, sort_keys=False, allow_unicode=True), encoding="utf-8")


def prepare(source: Path, destination: Path) -> None:
    if not source.is_dir():
        raise SystemExit(f"Wiki source does not exist: {source}")

    if destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True)

    # _Sidebar.md and _Footer.md are parts of every wiki page, not pages
    pages = sorted(p for p in source.glob("*.md") if not p.name.startswith("_"))
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
        # GitHub shows the page's name as its title (hyphens as spaces), so
        # wiki pages don't start with one; give MkDocs the same title
        if not text.lstrip().startswith("# "):
            title = "CProcessing" if source_page.stem == "Home" else source_page.stem.replace("-", " ")
            text = f"# {title}\n\n{text}"
        (destination / page_file(source_page.stem)).write_text(text, encoding="utf-8", newline="\n")

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
    parser.add_argument("--config", nargs=3, type=Path, metavar=("BASE", "OUTPUT", "SITE_DIR"),
                        help="write OUTPUT: the BASE MkDocs config, building the wiki into SITE_DIR "
                             "with the nav from _Sidebar.md")
    args = parser.parse_args()
    source, destination = args.source.resolve(), args.destination.resolve()
    prepare(source, destination)
    if args.config:
        base, output, site_dir = (p.resolve() for p in args.config)
        sidebar = source / "_Sidebar.md"
        write_config(base, output, destination, site_dir, sidebar_nav(sidebar) if sidebar.exists() else None)


if __name__ == "__main__":
    main()
