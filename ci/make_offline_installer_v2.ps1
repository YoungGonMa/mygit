# version2 - ecm,cloudium,startia

# Type define "Product"
Add-Type @"
public struct Product 
{
	public string Name;
	public string InstallerFileNameFromBatch;
	public string InstallerFileName;
}
"@
	
# Product list
$productList = @(
	[Product]@{ 
		Name = "ecm"; 
		InstallerFileNameFromBatch = "DestinySmartUpdaterOfflineInstall"; 
		InstallerFileName = "DestinyECMStandAloneInstaller"; 
	},
	[Product]@{ 
		Name = "cloudium"; 
		InstallerFileNameFromBatch = "cloudiumSmartUpdaterOfflineInstall"; 
		InstallerFileName = "cloudiumStandAloneInstaller"; 
	},
	[Product]@{ 
		Name = "startia"; 
		InstallerFileNameFromBatch = "SecureSAMBAProSmartUpdaterOfflineInstall"; 
		InstallerFileName = "SecureSAMBAProStandAloneInstaller"; 
	}
)

function printTitle($title)
{
    Write-Host "########################################################################"
    Write-Host "####"
    Write-Host "#### $title"
    Write-Host "####"
    Write-Host "########################################################################"
}

function copyOfflineInstaller_($product, $lang)
{
    $srcFilePath = $Env:WORKSPACE + "\_projects\installer\bin\" + $product.Name +"\" + $lang +"\"
    
    Write-Host "#### copyOfflineInstaller : " $product.Name
    $srcFilePath += $product.InstallerFileNameFromBatch
	$srcFilePath += ".??.exe"

	Write-Host "#### * from: " $srcFilePath

	if (Test-Path $srcFilePath)
	{
		$destPath = $Env:WORKSPACE + "\_offline_installer\"
		$destPath += $product.InstallerFileName
		$destPath += "."
		$destPath += $lang
		$destPath += ".exe"

		Write-Host "#### *   to: " $destPath

		Copy-Item -Path $srcFilePath -Destination $destPath -Force
	}
	else
	{
		# 없는 인스톨러 (스타티아는 일본어만 만드므로 나머지는 없을것임)
		Write-Host "#### -> skip (not exist)"
	}
}

function copyOfflineInstaller($product)
{
    copyOfflineInstaller_ $product "KO"
	copyOfflineInstaller_ $product "EN"
	copyOfflineInstaller_ $product "ZH"
	copyOfflineInstaller_ $product "JA"
}

function copyOfflineInstallerAll
{
	foreach ($product in $productList)
	{
		copyOfflineInstaller $product
	}
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
            if (Test-Path $makeOfflineInstallerBat)
            {
                # do this action in newer branch only
                makeOfflineInstaller
                copyOfflineInstallerAll
            }
            else
            {
                # not exists = old branch
            }
        }
    }
}
