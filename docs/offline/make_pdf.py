"""Print the combined MkDocs print page to PDF with Chromium."""

from __future__ import annotations

import argparse
import contextlib
import http.server
import socketserver
import threading
from pathlib import Path

from playwright.sync_api import sync_playwright


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format: str, *args: object) -> None:
        pass


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("site", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    site = args.site.resolve()
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    handler = lambda *a, **kw: QuietHandler(*a, directory=site, **kw)
    with socketserver.TCPServer(("127.0.0.1", 0), handler) as server:
        port = server.server_address[1]
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        try:
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch()
                page = browser.new_page()
                page.goto(
                    f"http://127.0.0.1:{port}/print_page.html",
                    wait_until="networkidle",
                )
                page.pdf(
                    path=str(output),
                    format="Letter",
                    print_background=True,
                    margin={"top": "0.6in", "right": "0.55in", "bottom": "0.6in", "left": "0.55in"},
                    display_header_footer=False,
                )
                browser.close()
        finally:
            server.shutdown()
            thread.join()


if __name__ == "__main__":
    main()
