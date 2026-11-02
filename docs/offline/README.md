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
  Windows launchers.
- `CProcessing-Documentation.pdf` contains the complete documentation in one
  printable file.

The build clones the current public wiki. To build from an existing local wiki
clone instead:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./docs/offline/build.ps1 -WikiSource ../CProcessing-wiki
```

Use `-SkipPdf` when Chromium cannot be installed or only the website is needed.

## Automated builds

The `offline-docs.yml` GitHub Actions workflow runs weekly and on demand. Its
artifact contains both deliverables. Because a GitHub wiki is a separate Git
repository, edits to the wiki do not trigger workflows in the main repository;
run the workflow manually for an immediate export after an important wiki edit.

## Student use

Students extract the ZIP and double-click `Open CProcessing Docs.cmd`. The
launcher starts a local-only web server and opens the documentation in the
default browser. Closing the PowerShell window stops the server.

The server is needed because browser security rules prevent the search index
from loading reliably when the HTML is opened directly with a `file:` URL.
