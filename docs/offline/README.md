# Offline documentation package

This directory builds the GitHub wiki into a searchable, self-contained website
and a PDF for students who do not have internet access.

## Build on Windows

Requirements: Git and Python 3.11 or newer.

```powershell
./docs/offline/build.cmd
```

The finished files are written to `docs/offline/output/`:

- `CProcessing-Offline-Documentation.zip` contains the searchable website and
  the PDF.
- `CProcessing-Documentation.pdf` contains the complete documentation in one
  printable file.

The build clones the current public wiki. To build from an existing local wiki
clone instead:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./docs/offline/build.ps1 -WikiSource ../CProcessing-wiki
```

Use `-SkipPdf` when Chromium cannot be installed or only the website is needed.

## Automated builds

The `offline-docs.yml` GitHub Actions workflow builds both files every week,
when a release is published, and on demand. Publishing a release attaches them
to it, so students download them from the
[Releases page](https://github.com/DigiPen-Faculty/CProcessing/releases)
with the templates.

Because a GitHub wiki is a separate Git repository, edits to the wiki don't
trigger workflows in the main repository. To refresh the files on the latest
release after an important wiki edit, run the workflow by hand (Actions >
Build offline documentation > Run workflow) with that release's tag.

The same workflow checks the wiki against the API with `tools/wiki-audit.py`,
which also compiles every example.

## Student use

Students extract the ZIP and double-click `Open CProcessing Docs.html`, on
Windows, macOS or Linux. The site, search included, works straight from the
disk (Material for MkDocs' `offline` plugin), so no web server or internet
connection is needed.
