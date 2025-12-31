function printTitle($title)
{
    Write-Host "########################################################################"
    Write-Host "####"
    Write-Host "#### $title"
    Write-Host "####"
    Write-Host "########################################################################"
}

function copyOfflineInstaller_($product, $destProduct, $lang)
{
    $srcFilePath = $Env:WORKSPACE + "\_projects\installer\bin\" + $product +"\" + $lang +"\"
    if ($product -eq "ecm")
    {
        Write-Host "copyInstaller : ecm"
        $srcFilePath += "DestinySmartUpdaterOfflineInstall.??.exe"
    }
    else
    {
        Write-Host "copyInstaller : cloudium"
        $srcFilePath += "cloudiumSmartUpdaterOfflineInstall.??.exe"
    }
    $destPath = $Env:WORKSPACE + "\_offline_installer\"
    Copy-Item -Path $srcFilePath -Destination $destPath -Force
}

function copyOfflineInstaller
{
    copyOfflineInstaller_ "ecm" "ecm" "KO"
    copyOfflineInstaller_ "ecm" "ecm" "EN"
    copyOfflineInstaller_ "ecm" "ecm" "ZH"
    copyOfflineInstaller_ "ecm" "ecm" "JA"
    copyOfflineInstaller_ "cloudium" "cloudium" "KO"
    copyOfflineInstaller_ "cloudium" "cloudium" "EN"
    copyOfflineInstaller_ "cloudium" "cloudium" "ZH"
    copyOfflineInstaller_ "cloudium" "cloudium" "JA"
}

function makeOfflineInstaller()
{
    printTitle("make offline-installer...")
    $Env:WORKSPACE + "\_projects\installer" | Set-Location
    .\make_offline_installer_all.bat
}

if ($Env:COMMIT_INSTALLER -ne $null -And $Env:COMMIT_INSTALLER -ne "")
{
    $commitInstallerList = $Env:COMMIT_INSTALLER.Split(",")
    $commitInstallerItem = ""
    foreach ($commitInstallerItem in $commitInstallerList)
    {
        if ($commitInstallerItem -eq "offline-installer")
        {
            $makeOfflineInstallerBat = $Env:WORKSPACE + "\_projects\installer\make_offline_installer_all.bat"
            If (Test-Path $makeOfflineInstallerBat)
            {
                # Do this action in newer branch only
                makeOfflineInstaller
                copyOfflineInstaller
            }
            Else
            {
                # not exists = old branch
            }
        }
    }
}
