# version2 - ecm,cloudium,startia
# version3 - ecm,cloudium,startia,ktbizez
# version4 - variable product type

Param(
    [Parameter(Position=0, ValueFromRemainingArguments=$true)]
    [string[]]$productNames
)

# Type define "Product"
Add-Type @"
public struct Product 
{
	public string Name;
	public string InstallerFileName;
}
"@

$UseECM = 0
$UseCloudium = 0
$UseStartia = 0
$UseKtbizez = 0

$productList = New-Object System.Collections.ArrayList
# Product list
foreach($item in $productNames)
{
    if($item -eq "ecm") {
        $productList.Add([Product]@{ 
		Name = "ecm"; 
		InstallerFileName = "DestinySmartUpdaterInstall.exe"; 
	    })
        $UseECM = 1
    }

    if($item -eq "cloudium") {
        $productList.Add([Product]@{ 
		Name = "cloudium"; 
		InstallerFileName = "cloudiumSmartUpdaterInstall.exe"; 
	    })
        $UseCloudium = 1
    }

    if($item -eq "startia") {
        $productList.Add([Product]@{ 
		Name = "startia"; 
		InstallerFileName = "SecureSAMBAProSmartUpdaterInstall.exe"; 
	    })
        $UseStartia = 1
    }

    if($item -eq "ktbizez") {
        $productList.Add([Product]@{ 
		Name = "ktbizez"; 
		InstallerFileName = "EZDriveSmartUpdaterInstall.exe"; 
	    })
        $UseKtbizez = 1
    }
}

if($productList.Count -eq 0)
{
    $productList.Add([Product]@{ 
	Name = "ecm"; 
	InstallerFileName = "DestinySmartUpdaterInstall.exe"; 
	})
    


    $productList.Add([Product]@{ 
	Name = "cloudium"; 
	InstallerFileName = "cloudiumSmartUpdaterInstall.exe"; 
	})
    


    $productList.Add([Product]@{ 
	Name = "startia"; 
	InstallerFileName = "SecureSAMBAProSmartUpdaterInstall.exe"; 
	})
    


    $productList.Add([Product]@{ 
	Name = "ktbizez"; 
	InstallerFileName = "EZDriveSmartUpdaterInstall.exe"; 
	})
    
    $UseECM = 1
    $UseCloudium = 1
    $UseStartia = 1
    $UseKtbizez = 1
}

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
    .\make_target_installer.bat $UseECM $UseCloudium $UseStartia $UseKtbizez
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

if ($null -ne $Env:COMMIT_INSTALLER -And $Env:COMMIT_INSTALLER -ne "")
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
