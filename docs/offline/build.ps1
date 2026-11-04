param(
    [string]$WikiSource,
    [switch]$SkipPdf
)

$ErrorActionPreference = 'Stop'
$offlineRoot = $PSScriptRoot
$workRoot = Join-Path $offlineRoot 'work'
$outputRoot = Join-Path $offlineRoot 'output'
$venvRoot = Join-Path $offlineRoot '.venv'
$python = Join-Path $venvRoot 'Scripts/python.exe'
$tempRoot = Join-Path $workRoot 'temp'

New-Item -ItemType Directory -Force -Path $workRoot, $outputRoot, $tempRoot | Out-Null
$env:TEMP = $tempRoot
$env:TMP = $tempRoot

if (-not (Test-Path $python)) {
    py -3 -m venv $venvRoot
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path $python)) {
        throw 'Python 3.11 or newer is required to build the offline documentation.'
    }
}
& $python -m pip install --disable-pip-version-check -r (Join-Path $offlineRoot 'requirements.txt')

$wikiClone = Join-Path $workRoot 'wiki-source'
if ($WikiSource) {
    $wikiClone = [System.IO.Path]::GetFullPath($WikiSource)
}
else {
    if (Test-Path $wikiClone) {
        git -C $wikiClone fetch --depth 1 origin
        git -C $wikiClone reset --hard origin/master
    }
    else {
        git clone --depth 1 https://github.com/DigiPen-Faculty/CProcessing.wiki.git $wikiClone
    }
}

$preparedWiki = Join-Path $workRoot 'wiki'
& $python (Join-Path $offlineRoot 'prepare_wiki.py') $wikiClone $preparedWiki
& $python -m mkdocs build --clean --config-file (Join-Path $offlineRoot 'mkdocs.yml')

$packageRoot = Join-Path $workRoot 'package'
if (Test-Path $packageRoot) { Remove-Item -Recurse -Force $packageRoot }
New-Item -ItemType Directory -Force -Path (Join-Path $packageRoot 'site') | Out-Null
Copy-Item (Join-Path $workRoot 'site\*') (Join-Path $packageRoot 'site') -Recurse -Force
Copy-Item (Join-Path $offlineRoot 'student\*') $packageRoot -Force

$pdfPath = Join-Path $outputRoot 'CProcessing-Documentation.pdf'
if (-not $SkipPdf) {
    & $python -m playwright install chromium
    & $python (Join-Path $offlineRoot 'make_pdf.py') (Join-Path $workRoot 'site') $pdfPath
    Copy-Item $pdfPath $packageRoot -Force
}

$zipPath = Join-Path $outputRoot 'CProcessing-Offline-Documentation.zip'
if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
Compress-Archive -Path (Join-Path $packageRoot '*') -DestinationPath $zipPath -CompressionLevel Optimal

Write-Host "Created $zipPath"
if (-not $SkipPdf) { Write-Host "Created $pdfPath" }
