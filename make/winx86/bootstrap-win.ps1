# based on http://poshcode.org/2461
Function downloadFileIfNotExists {
    Param (
        [Parameter(Mandatory=$True)] [System.Uri]$uri,
        [Parameter(Mandatory=$True)] [string]$path
    )

    if (!(Test-Path $path)) {
        $client = (New-Object System.Net.WebClient)
        $Global:downloadComplete = $false

        $eventDownloadComplete = Register-ObjectEvent $client DownloadFileCompleted -SourceIdentifier WebClient.DownloadFileComplete -Action { $Global:downloadComplete = $true }
        $eventDownloadProgress = Register-ObjectEvent $client DownloadProgressChanged -SourceIdentifier WebClient.DownloadProgressChanged -Action { $Global:DPCEventArgs = $EventArgs }

        Write-Progress -Activity "Downloading" -Status $uri.toString()
        $client.DownloadFileAsync($uri.toString(), $path)

        while (!($Global:downloadComplete)) {
            $pc = $Global:DPCEventArgs.ProgressPercentage
            if ($pc -ne $null) {
                Write-Progress -Activity "Downloading" -Status $uri.toString() -PercentComplete $pc
            }
        }

        Write-Progress -Activity "Downloading" -Status $uri.toString() -Complete

        Unregister-Event -SourceIdentifier WebClient.DownloadProgressChanged
        Unregister-Event -SourceIdentifier WebClient.DownloadFileComplete
        $client.Dispose()
        $Global:downloadComplete = $null
        $Global:DPCEventArgs = $null
    }
}

Function runProcessWait {
    Param (
        [Parameter(Mandatory=$True)] [string]$exe,
        [Parameter(Mandatory=$True)] [string[]]$argumentList
    )
    $proc = Start-Process -FilePath $exe -ArgumentList $argumentList -Wait -NoNewWindow -PassThru
    $proc.ExitCode
}

Function qtSdkInstall {
    Param (
        [Parameter(Mandatory=$True)] [string]$sdkInstaller,
        [Parameter(Mandatory=$True)] [string]$sdkPath
    )

    $installScript = Join-Path $Global:scriptDir "qt-install.qs"
    runProcessWait $sdkInstaller "--script",$installScript,"dronin_qt_path=$($sdkPath)"
}

Function qtSdkInstallMinGW {
    Param (
        [Parameter(Mandatory=$True)] [string]$sdkPath
    )

    $maintTool = Join-Path $sdkPath "MaintenanceTool.exe"
    $installScript = Join-Path $Global:scriptDir "qt-install.qs"
    runProcessWait $maintTool "--script",$installScript,"--manage-packages","--setTempRepository","http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/tools_mingw/","dronin_qt_components=qt.tools.win32_mingw530"
}

$environments = "msvc", "mingw"
$environment = "msvc"
if ($environments -contains $args[0]) {
    $environment = $args[0]
}

$invocation = (Get-Variable MyInvocation).Value
$Global:rootDir = Split-Path $invocation.MyCommand.Path | Join-Path -ChildPath "..\.." -Resolve
$Global:scriptDir = Join-Path $Global:rootDir "make\scripts"
$dlDir = Join-Path $Global:rootDir "downloads"
$toolsDir = Join-Path $Global:rootDir "tools"

if (!(Test-Path -PathType Container $dlDir)) {
    New-Item -ItemType Directory -Path $dlDir | Out-Null
}

if (!(Test-Path -PathType Container $toolsDir)) {
    New-Item -ItemType Directory -Path $toolsDir | Out-Null
}

$qtSdkInstallerUrl = [System.Uri]"http://download.qt.io/official_releases/qt/5.8/5.8.0/qt-opensource-windows-x86-msvc2015-5.8.0.exe"
if ($environment -eq "mingw") {
    $qtSdkInstallerUrl = [System.Uri]"http://download.qt.io/official_releases/qt/5.8/5.8.0/qt-opensource-windows-x86-mingw530-5.8.0.exe"
}

$qtSdkInstaller = Join-Path $dlDir $qtSdkInstallerUrl.Segments[-1]
$qtSdkPath = Join-Path $toolsDir "Qt$($qtSdkInstallerUrl.Segments[-2])"

# TODO: give choice to reinstall or abort
if (!(Test-Path $qtSdkPath)) {
    Echo "Downloading Qt..."
    downloadFileIfNotExists $qtSdkInstallerUrl $qtSdkInstaller

    Echo "Installing Qt..."
    if ((qtSdkInstall $qtSdkInstaller $qtSdkPath) -ne 0) {
        Echo "Qt installation FAILED!"
        Exit
    }
    if ($environment -ne "mingw") {
        Echo "Installing MinGW..."
        if ((qtSdkInstallMinGW $qtSdkPath) -ne 0) {
            Echo "MinGW installation FAILED!"
            Exit
        }
    }
}

# set environment variable in user profile that the bash profile will use to choose msvc or MinGW
[Environment]::SetEnvironmentVariable("DRONIN_ENV", $environment, "User")

# install build environment setup to bash profile
$bashScript = Join-Path $Global:rootDir "make\winx86\bash_profile"
$bashPath = Join-Path $env:userprofile ".bash_profile"
if (Test-Path($bashPath)) {
    if (Get-Content $bashPath | Select-String "# bashrc for dRonin development on Windows" -quiet) {
        # old-style dRonin bash script
        $i = 0
        while (Test-Path(Join-Path $env:userprofile "bash_profile.$($i).bak")) {
            $i++
        }
        Move-Item $bashPath (Join-Path $env:userprofile "bash_profile.$($i).bak")
        Echo "Bash profile backed up to $(Join-Path $env:userprofile "bash_profile.$($i).bak")"
    } elseif (!(Get-Content $bashPath | Select-String "# dRonin build environment setup" -quiet)) {
        # no dRonin previously setup
        Add-Content $bashPath "`n`n"
        Get-Content $bashScript | Add-Content $bashPath
        Echo "Appended dRonin environment setup to bash profile at $($bashPath)"
    }
}
if (!(Test-Path($bashPath))) {
    Copy-Item $bashScript $bashPath
    Echo "Installed new bash profile to $($bashPath)"
}
