# version2 - ecm,cloudium,startia

# Type define "Product"
Add-Type @"
public struct Product 
{
	public string Name;
	public string InstallerFileName;
}
"@
	
# Product list
$productList = @(
	[Product]@{ 
		Name = "ecm"; 
		InstallerFileName = "DestinySmartUpdaterInstall.exe"; 
	},
	[Product]@{ 
		Name = "cloudium"; 
		InstallerFileName = "cloudiumSmartUpdaterInstall.exe"; 
	},
	[Product]@{ 
		Name = "startia"; 
		InstallerFileName = "SecureSAMBAProSmartUpdaterInstall.exe"; 
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

function makeInstaller()
{
    printTitle("make installer...")
    $Env:WORKSPACE + "\_projects\installer" | Set-Location
    .\make_installer_all.bat
}

function copyInstaller_($product, $lang)
{
    $srcFilePath = $Env:WORKSPACE + "\_projects\installer\bin\" + $product.Name +"\" + $lang +"\"
    
	Write-Host "copyInstaller : " + $product.Name
    $srcFilePath += $product.InstallerFileName

    $destPath = $Env:WORKSPACE + "\_distribute\" + $product.Name + "\solution\SmartUpdaterInstall\" + $lang + "\"
    Copy-Item -Path $srcFilePath -Destination $destPath -Force
}

function copyInstaller($product)
{
    copyInstaller_ $product "KO" 
    copyInstaller_ $product "EN" 
    copyInstaller_ $product "ZH" 
    copyInstaller_ $product "JA" 
}

function copyInstallerAll
{
	foreach ($product in $productList)
	{
		copyInstaller $product
	}
}

function commit_internal($commitMessage)
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

function commit($product, $commitMessage)
{	
	$Env:WORKSPACE + "\_distribute\" + $product | Set-Location
	Get-Location | Write-Host
	commit_internal ($commitMessage + " - " + $product)
}

function commitAll($commitMessage)
{
	foreach ($product in $productList)
	{
		commit $product.Name $commitMessage
	}
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
            copyInstallerAll

            printTitle "commit installer..."

            $Env:WORKSPACE + "\_projects" | Set-Location
            Get-Location | Write-Host
            $commitMessage = $Env:BUILD_TAG + " agent installer (r" + $Env:SVN_REVISION_1 + ")"

			commitAll $commitMessage
        }
    }
}
