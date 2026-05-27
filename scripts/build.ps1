<#
.SYNOPSIS
    Local Docker-based devkitPro build wrapper for ReverseNX-RT (Ryazhenka).
#>
[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$Dist,
    [int]$Jobs = 0
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
Set-Location $repoRoot

function Write-Section($msg) {
    Write-Host ''
    Write-Host '============================================================' -ForegroundColor Cyan
    Write-Host " $msg" -ForegroundColor Cyan
    Write-Host '============================================================' -ForegroundColor Cyan
}

function Test-Docker {
    try {
        $null = & docker version --format '{{.Server.Version}}' 2>&1
        return $LASTEXITCODE -eq 0
    } catch { return $false }
}

Write-Section 'ReverseNX-RT (Ryazhenka) — local build'

if (-not (Test-Docker)) {
    Write-Host ''
    Write-Host 'Docker is required. Install Docker Desktop:' -ForegroundColor Yellow
    Write-Host '  https://www.docker.com/products/docker-desktop/'
    Write-Host 'Or build natively via msys2 + devkitPro pacman (see docs/RU/build.md).'
    exit 1
}

if ($Jobs -le 0) { $Jobs = [Environment]::ProcessorCount }

$appVersion = (Select-String -Path "$repoRoot\Overlay\Makefile" -Pattern '^APP_VERSION\s*:=\s*(\S+)').Matches.Groups[1].Value
$sha = (& git -C $repoRoot rev-parse --short HEAD).Trim()

$image = 'devkitpro/devkita64:latest'
Write-Host "Pulling/refreshing $image..." -ForegroundColor Green
& docker pull $image
if ($LASTEXITCODE -ne 0) { throw "docker pull failed" }

$makeCmd = @(
    'git config --global --add safe.directory /project',
    'dkp-pacman -S --noconfirm --needed switch-libpng switch-curl switch-zziplib switch-mbedtls switch-libjpeg-turbo'
)
if ($Clean) { $makeCmd += 'make clean || true' }
$makeCmd += "make -j$Jobs"

if ($Dist) {
    $name = "ReverseNX-RT-$appVersion-$sha"
    $makeCmd += "rm -rf dist/$name dist/$name.zip"
    $makeCmd += "mkdir -p dist/$name/switch/.overlays"
    $makeCmd += "mkdir -p dist/$name/config/ReverseNX-RT/lang"
    $makeCmd += "cp Overlay/ReverseNX-RT-ovl.ovl dist/$name/switch/.overlays/"
    $makeCmd += "cp lang/*.json dist/$name/config/ReverseNX-RT/lang/"
    $makeCmd += "cd dist && zip -r $name.zip $name"
}
$shellCmd = $makeCmd -join ' && '

Write-Section "Running build (jobs=$Jobs, dist=$Dist, clean=$Clean)"
Write-Host "Command: $shellCmd" -ForegroundColor DarkGray

& docker run --rm -v "${repoRoot}:/project" -w /project $image bash -c $shellCmd
if ($LASTEXITCODE -ne 0) {
    Write-Host 'Build failed.' -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Section 'Build complete'

if ($Dist) {
    $zips = Get-ChildItem "$repoRoot\dist" -Filter '*.zip' -ErrorAction SilentlyContinue
    foreach ($z in $zips) {
        $hash = (Get-FileHash $z.FullName -Algorithm SHA256).Hash
        $size = [math]::Round($z.Length / 1KB, 1)
        Write-Host ''
        Write-Host "  $($z.Name)" -ForegroundColor Green
        Write-Host "  Size:   $size KB"
        Write-Host "  SHA256: $hash"
    }
}
if (Test-Path "$repoRoot\Overlay\ReverseNX-RT-ovl.ovl") {
    $sig = [Text.Encoding]::ASCII.GetString([byte[]](Get-Content -Path "$repoRoot\Overlay\ReverseNX-RT-ovl.ovl" -AsByteStream -Tail 4))
    Write-Host ''
    Write-Host "  Overlay signature (last 4 bytes): $sig" -ForegroundColor $(if ($sig -eq 'RYZH') { 'Green' } else { 'Yellow' })
}
