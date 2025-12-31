function printTitle($title)
{
    Write-Host "########################################################################"
    Write-Host "####"
    Write-Host "#### $title"
    Write-Host "####"
    Write-Host "########################################################################"
}

function makeInstaller()
{
    printTitle("make installer...")
    $Env:WORKSPACE + "\_projects\installer" | Set-Location
    .\make_installer_all.bat
}

function copyInstaller_($product, $destProduct, $lang)
{
    $srcFilePath = $Env:WORKSPACE + "\_projects\installer\bin\" + $product +"\" + $lang +"\"
    if ($product -eq "ecm")
    {
        Write-Host "copyInstaller : ecm"
        $srcFilePath += "DestinySmartUpdaterInstall.exe"
    }
    else
    {
        Write-Host "copyInstaller : cloudium"
        $srcFilePath += "cloudiumSmartUpdaterInstall.exe"
    }
    $destPath = $Env:WORKSPACE + "\_distribute\" + $destProduct + "\solution\SmartUpdaterInstall\" + $lang + "\"
    Copy-Item -Path $srcFilePath -Destination $destPath -Force
}

function copyInstaller
{
    copyInstaller_ "ecm" "ecm" "KO"
    copyInstaller_ "ecm" "ecm" "EN"
    copyInstaller_ "ecm" "ecm" "ZH"
    copyInstaller_ "ecm" "ecm" "JA"
    copyInstaller_ "cloudium" "cloudium" "KO"
    copyInstaller_ "cloudium" "cloudium" "EN"
    copyInstaller_ "cloudium" "cloudium" "ZH"
    copyInstaller_ "cloudium" "cloudium" "JA"
}

function commit($commitMessage)
{
    $arguments = @(
        "solution\SmartUpdaterInstall\EN\*.exe",
        "solution\SmartUpdaterInstall\KO\*.exe",
        "solution\SmartUpdaterInstall\ZH\*.exe",
        "solution\SmartUpdaterInstall\JA\*.exe"
    )

    $arguments += "-m","`"$commitMessage`""

    svn.exe commit $arguments
    Write-Host $arguments
}

if ($Env:COMMIT_INSTALLER -ne $null -And $Env:COMMIT_INSTALLER -ne "")
{
    $commitInstallerList = $Env:COMMIT_INSTALLER.Split(",")
    $commitInstallerItem = ""
    foreach ($commitInstallerItem in $commitInstallerList)
    {
        if ($commitInstallerItem -eq "installer")
        {
            makeInstaller
            copyInstaller

            printTitle "commit installer..."

            $Env:WORKSPACE + "\_projects" | Set-Location
            Get-Location | Write-Host
            $commitMessage = $Env:BUILD_TAG + " agent installer (r" + $Env:SVN_REVISION_1 + ")"

            $Env:WORKSPACE + "\_distribute\ecm" | Set-Location
            Get-Location | Write-Host
            commit($commitMessage + " - ecm")

            $Env:WORKSPACE + "\_distribute\cloudium" | Set-Location
            Get-Location | Write-Host
            commit($commitMessage + " - cloudium")
        }
    }
}
