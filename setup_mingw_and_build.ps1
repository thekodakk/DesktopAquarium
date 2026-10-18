# PowerShell Automatic MinGW Installer and Build Script
Set-Location -Path $PSScriptRoot

Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "   MinGW (g++) Automatic Installer & Builder" -ForegroundColor Cyan
Write-Host "===================================================" -ForegroundColor Cyan
Write-Host ""

$toolsDir = Join-Path -Path $PSScriptRoot -ChildPath "tools"
$zipPath = Join-Path -Path $toolsDir -ChildPath "compiler.zip"

# Function to test g++
function Test-Gpp {
    try {
        $cmd = Get-Command g++ -ErrorAction Stop
        return $true
    } catch {
        return $false
    }
}

# Check if g++ is already available in toolsDir
if (-not (Test-Gpp) -and (Test-Path $toolsDir)) {
    $existingGpp = Get-ChildItem -Path $toolsDir -Recurse -Filter "g++.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($existingGpp) {
        $binDir = $existingGpp.DirectoryName
        $env:PATH = "$binDir;$env:PATH"
    }
}

if (-not (Test-Gpp)) {
    # Clean up broken previous tools folder if needed
    if (Test-Path $toolsDir) {
        Write-Host "[INFO] Eski tools klasörü temizleniyor..." -ForegroundColor Yellow
        Remove-Item -Path $toolsDir -Recurse -Force -ErrorAction SilentlyContinue
        if (Test-Path $toolsDir) {
            cmd /c rmdir /s /q "$toolsDir" 2>$null
        }
    }
    New-Item -ItemType Directory -Path $toolsDir -Force | Out-Null

    Write-Host "[INFO] Güncel w64devkit (MinGW) sürümü aranıyor..." -ForegroundColor Yellow
    
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 -bor [Net.SecurityProtocolType]::Tls13

    $downloadUrl = ""

    # Try GitHub API to get exact latest w64devkit zip URL
    try {
        $headers = @{ "User-Agent" = "PowerShell-MinGW-Installer" }
        $release = Invoke-RestMethod -Uri "https://api.github.com/repos/skeeto/w64devkit/releases/latest" -Headers $headers -UseBasicParsing
        $asset = $release.assets | Where-Object { $_.name -like "*.zip" } | Select-Object -First 1
        if ($asset -and $asset.browser_download_url) {
            $downloadUrl = $asset.browser_download_url
            Write-Host "[INFO] GitHub API üzerinden çalışan indirme adresi alındı: $downloadUrl" -ForegroundColor Green
        }
    } catch {
        Write-Host "[UYARI] GitHub API erişilemedi, bilinen sabit sürümler deneniyor..." -ForegroundColor Yellow
    }

    # Fallback URLs if API failed
    if (-not $downloadUrl) {
        $fallbackUrls = @(
            "https://github.com/skeeto/w64devkit/releases/download/v1.23.0/w64devkit-1.23.0.zip",
            "https://github.com/skeeto/w64devkit/releases/download/v1.22.0/w64devkit-1.22.0.zip",
            "https://github.com/skeeto/w64devkit/releases/download/v1.21.0/w64devkit-1.21.0.zip"
        )
        foreach ($url in $fallbackUrls) {
            try {
                $req = [System.Net.WebRequest]::Create($url)
                $req.Method = "HEAD"
                $res = $req.GetResponse()
                if ($res.StatusCode -eq 200) {
                    $downloadUrl = $url
                    $res.Close()
                    break
                }
                $res.Close()
            } catch {}
        }
    }

    if (-not $downloadUrl) {
        $downloadUrl = "https://github.com/skeeto/w64devkit/releases/download/v1.23.0/w64devkit-1.23.0.zip"
    }

    Write-Host "[1/3] $downloadUrl indiriliyor..." -ForegroundColor Cyan
    
    # Download with progress
    try {
        Invoke-WebRequest -Uri $downloadUrl -OutFile "$zipPath" -UseBasicParsing
    } catch {
        Write-Host "[BILGI] curl.exe ile alternatif indirme..." -ForegroundColor Yellow
        curl.exe -L $downloadUrl -o "$zipPath"
    }

    if (Test-Path "$zipPath") {
        $size = (Get-Item "$zipPath").Length
        Write-Host "[INFO] İndirilen dosya boyutu: $([math]::Round($size/1MB, 2)) MB" -ForegroundColor Green

        if ($size -lt 100000) {
            Write-Host "[HATA] İndirilen dosya çok küçük veya bozuk ($size bayt)!" -ForegroundColor Red
            exit 1
        }

        Write-Host "[2/3] Dosyalar ayıklanıyor (tar -xf kullanılıyor)..." -ForegroundColor Cyan
        tar -xf "$zipPath" -C "$toolsDir"
        Remove-Item -Path "$zipPath" -Force -ErrorAction SilentlyContinue
    } else {
        Write-Host "[HATA] ZIP dosyası indirilemedi!" -ForegroundColor Red
        exit 1
    }

    # Find bin directory containing g++.exe recursively
    $gppFile = Get-ChildItem -Path $toolsDir -Recurse -Filter "g++.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($gppFile) {
        $binDir = $gppFile.DirectoryName
        $env:PATH = "$binDir;$env:PATH"
        Write-Host "[3/3] MinGW PATH değişkenine eklendi: $binDir" -ForegroundColor Green

        # Add to Permanent User Environment Variable PATH
        try {
            $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
            if ($userPath -notlike "*$binDir*") {
                [Environment]::SetEnvironmentVariable("Path", "$binDir;$userPath", "User")
                Write-Host "[INFO] MinGW kalıcı Kullanıcı PATH değişkenine eklendi." -ForegroundColor Green
            }
        } catch {}
    } else {
        Write-Host "[HATA] Ayıklanan klasörde g++.exe bulunamadı!" -ForegroundColor Red
        exit 1
    }
}

# Verify g++
if (Test-Gpp) {
    $ver = & g++ --version | Select-Object -First 1
    Write-Host "`n[BAŞARILI DERLEYİCİ] $ver" -ForegroundColor Green
    Write-Host "[INFO] DesktopAquarium.exe derlemesi başlatılıyor..." -ForegroundColor Yellow

    # Compile Desktop Aquarium
    & g++ -O2 -std=c++17 main.cpp Aquarium.cpp Fish.cpp Renderer.cpp ContextMenu.cpp AssetManager.cpp -o DesktopAquarium.exe -lgdi32 -lgdiplus -luser32 -lshell32 -mwindows

    if ($LASTEXITCODE -eq 0 -and (Test-Path "DesktopAquarium.exe")) {
        Write-Host "`n===================================================" -ForegroundColor Green
        Write-Host " [TEBRİKLER] DesktopAquarium.exe başarıyla derlendi!" -ForegroundColor Green
        Write-Host " Masaüstü Akvaryum uygulaması başlatılıyor..." -ForegroundColor Green
        Write-Host "===================================================" -ForegroundColor Green
        
        Start-Process -FilePath ".\DesktopAquarium.exe" -WorkingDirectory $PSScriptRoot
    } else {
        Write-Host "`n[HATA] Derleme sırasında hata oluştu!" -ForegroundColor Red
    }
} else {
    Write-Host "[HATA] g++ çalıştırılamadı!" -ForegroundColor Red
}
