$ErrorActionPreference = 'Stop'

$siteRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot 'site'))
$listener = [System.Net.HttpListener]::new()
$port = 8765

while ($port -lt 8865) {
    try {
        $listener.Prefixes.Clear()
        $listener.Prefixes.Add("http://127.0.0.1:$port/")
        $listener.Start()
        break
    }
    catch [System.Net.HttpListenerException] {
        $port++
    }
}

if (-not $listener.IsListening) {
    throw 'Could not find an available local port for the documentation server.'
}

$mimeTypes = @{
    '.css' = 'text/css; charset=utf-8'
    '.html' = 'text/html; charset=utf-8'
    '.ico' = 'image/x-icon'
    '.js' = 'text/javascript; charset=utf-8'
    '.json' = 'application/json; charset=utf-8'
    '.png' = 'image/png'
    '.svg' = 'image/svg+xml'
    '.woff' = 'font/woff'
    '.woff2' = 'font/woff2'
}

$url = "http://127.0.0.1:$port/index.html"
Write-Host "CProcessing documentation is available at $url"
Write-Host 'Close this window to stop the local documentation server.'
Start-Process $url

try {
    while ($listener.IsListening) {
        $context = $listener.GetContext()
        $relativePath = [Uri]::UnescapeDataString($context.Request.Url.AbsolutePath.TrimStart('/'))
        if ([string]::IsNullOrWhiteSpace($relativePath)) { $relativePath = 'index.html' }
        $candidate = [System.IO.Path]::GetFullPath((Join-Path $siteRoot $relativePath))

        if (-not $candidate.StartsWith($siteRoot, [System.StringComparison]::OrdinalIgnoreCase) -or
            -not [System.IO.File]::Exists($candidate)) {
            $context.Response.StatusCode = 404
            $body = [System.Text.Encoding]::UTF8.GetBytes('Not found')
        }
        else {
            $extension = [System.IO.Path]::GetExtension($candidate).ToLowerInvariant()
            $context.Response.ContentType = if ($mimeTypes.ContainsKey($extension)) {
                $mimeTypes[$extension]
            } else {
                'application/octet-stream'
            }
            $body = [System.IO.File]::ReadAllBytes($candidate)
        }

        $context.Response.ContentLength64 = $body.Length
        $context.Response.OutputStream.Write($body, 0, $body.Length)
        $context.Response.OutputStream.Close()
    }
}
finally {
    $listener.Stop()
    $listener.Close()
}
