# version2 - ecm,cloudium,startia

# Type define "Product"
Add-Type @"
public struct Product 
{
	public string Name;
	public string SvnUrl;
	public string InstallerFileName;
}
"@
	
# Product list
$productList = @(
	[Product]@{ 
		Name = "ecm"; 
		SvnUrl = ($Env:SVN_URL_2);
		InstallerFileName = "DestinySmartUpdaterInstall.exe"; 
	},
	[Product]@{ 
		Name = "cloudium"; 
		SvnUrl = ($Env:SVN_URL_3);
		InstallerFileName = "cloudiumSmartUpdaterInstall.exe"; 
	},
	[Product]@{ 
		Name = "startia"; 
		SvnUrl = ($Env:SVN_URL_4);
		InstallerFileName = "SecureSAMBAProSmartUpdaterInstall.exe"; 
	}
)

Write-Host "`$Env:WORKSPACE: $Env:WORKSPACE"

$_files = ""
$_filesSU = ""
    # DestinySmartUpdaterX.dll - 5.07.007 or older
    # DestinySUResEN.dll, DestinySUResKO.dll, DestinySUResZH.dll
    # DestinyECMAgentManager.dll, DestinyDriveManager.dll, DestinyCheckinManager.dll, DestinyLocalSyncManager.dll
$_filesSM = ""
    # DestinySolutionMgr.exe
    # DestinySmartUpdater.exe
    # DestinySmartUpdaterX.dll - 5.07.008 or later

Write-Host "COMMIT_TARGET: `"$Env:COMMIT_TARGET`""
if ($Env:COMMIT_TARGET) {
    $_files = $Env:COMMIT_TARGET.Split(",")
}

Write-Host "COMMIT_TARGET_RESDLL: `"$Env:COMMIT_TARGET_RESDLL`""
if ($Env:COMMIT_TARGET_RESDLL) {
    if ($_files -eq "") {
        $_files = $Env:COMMIT_TARGET_RESDLL.Split(",")
    } else {
        $_files += $Env:COMMIT_TARGET_RESDLL.Split(",")
    }
}

Write-Host "COMMIT_TARGET_SU: `"$Env:COMMIT_TARGET_SU`""
if ($Env:COMMIT_TARGET_SU) {
    $_filesSU = $Env:COMMIT_TARGET_SU.Split(",")
}

Write-Host "COMMIT_TARGET_SM: `"$Env:COMMIT_TARGET_SM`""
if ($Env:COMMIT_TARGET_SM) {
    $_filesSM = $Env:COMMIT_TARGET_SM.Split(",")
}

function copyBin($product)
{
    $binPath0 = $Env:WORKSPACE + "\_projects\bin\"
    $destPath0 = $Env:WORKSPACE + "\_distribute\" + $product + "\solution\"

    foreach ($item in $_files)
    {
        $binPath = $binPath0

		# v2에서는 ecm과 같은 파일을 사용
        #if ($product -eq "cloudium")
        #{
        #    if ($item -eq "DestinyECMAgent.exe" -Or $item -eq "DestinyECMIF.dll")
        #    {
        #        $binPath += "cloudium\"
        #    }
        #}

        if ($item -eq "DestinyECMIF.dll" -Or $item -eq "DestinyOneRPC.dll")
        {
            $destPath = $destPath0 + "DestinyECMAgent\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath $item
            $binPath + $item | Copy-Item -Destination $destPath -Force

            $destPath = $destPath0 + "DestinyDrive\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath $item
            $binPath + $item | Copy-Item -Destination $destPath -Force
         }
         elseif ($item -eq "DestinyLocalSyncIF.dll" -Or $item -eq "DestinyLocalSyncIF64.dll")
         {
            $destPath = $destPath0 + "DestinyLocalSync\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath $item
            $binPath + $item | Copy-Item -Destination $destPath -Force
         }
         elseif ($item -eq "ko-KR_DestinyRes.dll.mui")
         {
            $destPath = $destPath0 + "DestinyECMAgent\ko-KR\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath "ko-KR\DestinyRes.dll.mui"
            $binPath + "ko-KR\DestinyRes.dll.mui" | Copy-Item -Destination $destPath -Force
         }
         elseif ($item -eq "en-US_DestinyRes.dll.mui")
         {
            $destPath = $destPath0 + "DestinyECMAgent\en-US\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath "en-US\DestinyRes.dll.mui"
            $binPath + "en-US\DestinyRes.dll.mui" | Copy-Item -Destination $destPath -Force
         }
         elseif ($item -eq "ja-JP_DestinyRes.dll.mui") 
         {
            $destPath = $destPath0 + "DestinyECMAgent\ja-JP\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath "ja-JP\DestinyRes.dll.mui"
            $binPath + "ja-JP\DestinyRes.dll.mui" | Copy-Item -Destination $destPath -Force
         }
         elseif ($item -eq "zh-CN_DestinyRes.dll.mui")
         {
            $destPath = $destPath0 + "DestinyECMAgent\zh-CN\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath "zh-CN\DestinyRes.dll.mui"
            $binPath + "zh-CN\DestinyRes.dll.mui" | Copy-Item -Destination $destPath -Force
         }
         else # DestinyECMAgent.exe DestinyUtilityIF.dll DestinyUtilityIF64.dll DestinyRes.dll
         {
            $destPath = $destPath0 + "DestinyECMAgent\"
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath $item
            $binPath + $item | Copy-Item -Destination $destPath -Force
         }
    }
}

function copyBinSU($product)
{
    $binPath = $Env:WORKSPACE + "\_projects\bin\"
    $destPath = $Env:WORKSPACE + "\_distribute\" + $product + "\solution\DestinySolutionMgr\"

    foreach ($item in $_filesSU)
    {
		Write-Host "destPath: " $destPath
		Write-Host "target: " $binPath $item
        $binPath + $item | Copy-Item -Destination $destPath -Force
    }
}

function copyBinSM($product)
{
    $binPath0 = $Env:WORKSPACE + "\_projects\bin\"
    $destPath = $Env:WORKSPACE + "\_distribute\" + $product + "\solution\DestinySolutionMgrX\"
    $destPathAlt = $Env:WORKSPACE + "\_distribute\" + $product + "\solution\DestinySolutionMgr\"

    foreach ($item in $_filesSM)
    {
        $binPath = $binPath0
        # v2에서는 ecm과 같은 파일을 사용
		#if (!$product.StartsWith("ecm") -And $item -eq "DestinySmartUpdater.exe")
        #{
        #    $binPath += "cloudium\"
        #}

        if ($item -eq "DestinySmartUpdaterX.dll")
        {
			Write-Host "destPathAlt: " $destPathAlt
			Write-Host "target: " $binPath $item
            $binPath + $item | Copy-Item -Destination $destPathAlt -Force
        }
        else
        {
			Write-Host "destPath: " $destPath
			Write-Host "target: " $binPath $item
            $binPath + $item | Copy-Item -Destination $destPath -Force
        }
    }
}

function commit_internal($commitMessage, [ref]$items)
{
    $argumentString = ""

    foreach ($item in $_files)
    {
         if ($item -eq "DestinyECMIF.dll" -Or $item -eq "DestinyOneRPC.dll")
         {
            $argumentString += "solution\" + "DestinyECMAgent\" + $item +"|"
            $argumentString += "solution\" + "DestinyDrive\" + $item +"|"
         }
         elseif ($item -eq "DestinyLocalSyncIF.dll" -Or $item -eq "DestinyLocalSyncIF64.dll")
         {
            $argumentString += "solution\" + "DestinyLocalSync\" + $item +"|"
         }
         elseif ($item -eq "ko-KR_DestinyRes.dll.mui")
         {
            $argumentString += "solution\" + "DestinyECMAgent\ko-KR\DestinyRes.dll.mui" + "|"
         }
         elseif ($item -eq "en-US_DestinyRes.dll.mui")
         {
            $argumentString += "solution\" + "DestinyECMAgent\en-US\DestinyRes.dll.mui" + "|"
         }
         elseif ($item -eq "ja-JP_DestinyRes.dll.mui") 
         {
            $argumentString += "solution\" + "DestinyECMAgent\ja-JP\DestinyRes.dll.mui" + "|"
         }
         elseif ($item -eq "zh-CN_DestinyRes.dll.mui")
         {
            $argumentString += "solution\" + "DestinyECMAgent\zh-CN\DestinyRes.dll.mui" + "|"
         }
         elseif ($item -ne "") # DestinyECMAgent.exe DestinyUtilityIF.dll DestinyUtilityIF64.dll DestinyRes.dll
         {
            $argumentString += "solution\" + "DestinyECMAgent\" + $item +"|"
         }
    }
    foreach ($item in $_filesSU)
    {
        if ($item -ne "") 
        {
            $argumentString += "solution\" + "DestinySolutionMgr\" + $item +"|"
        }
    }
    foreach ($item in $_filesSM)
    {
        if ($item -eq "DestinySmartUpdaterX.dll")
        {
            $argumentString += "solution\" + "DestinySolutionMgr\" + $item +"|"
        }
        elseif ($item -ne "")
        {
            $argumentString += "solution\" + "DestinySolutionMgrX\" + $item +"|"
        }
    }

    if ($argumentString.Length -gt 0)
    {
        $argumentString = $argumentString.Substring(0,$argumentString.Length-1) # remove last '|'
    }
    $arguments = $argumentString.split("|")
    
    if ($Env:COPY_NEXT_BRANCH -eq "true")
    {
        $items.Value = $argumentString.split("|") # $arguments | ForEach-Object { , $_ } # deep copy
        Write-Host "items: " $items
    }

    $arguments += "-m","`"$commitMessage`""

    svn.exe commit $arguments
    Write-Host $arguments
}

function merge($product, $svnUrl, $svnRevision, $commitMessage, $items)
{
    foreach ($item in $items)
    {
        Write-Host "merge item: " $item
        if ($item -ne "")
        {
            $itemU = $item.replace('\', '/') 
            Write-Host "itemU: " $itemU
            Write-Host "svnUrl: " $svnUrl
            $severItem = "{0}/{1}" -f $svnUrl, $itemU
            Write-Host "severItem: " $severItem
            $revision = "-c{0}" -f $svnRevision
            Write-Host "revision: " $revision
            $arguments = "--accept","theirs-full",$revision,$severItem,$item
            svn.exe merge $arguments
            Write-Host "merge arguments: " $arguments
        }
    }
    
    $arguments = $items,"-m","`"$commitMessage`""
    
    svn.exe commit $arguments
    Write-Host "comit arguments: " $arguments
}

function commit($product, $svn_url)
{
	$Env:WORKSPACE + "\_distribute\" + $product | Set-Location
	Get-Location | Write-Host
	$items = $null
	commit_internal ($_commitMessage + " - " + $product)  ([ref]$items)

	if ($Env:COPY_NEXT_BRANCH -eq "true")
	{
		svn.exe update
		$newRevsion = svn.exe info | Select-String -Pattern "Revision: "
		$newRevsion = $newRevsion -replace "Revision: ",""
		Write-Host "newRevsion: " $newRevsion

		$Env:WORKSPACE + "\_distribute\" + $product + "_next" | Set-Location
		Get-Location | Write-Host
		svn.exe update

		$svnRelativePath = $svn_url -replace "https://stardust.cyberdigm.co.kr/DestinyONE",""
		merge $product ($svn_url) ($newRevsion) ("{0} - " + $product + ": Copied from r{1} in {2}" -f $_commitMessage, $newRevsion, $svnRelativePath) ($items)
	}
}

foreach ($product in $productList)
{
	copyBin $product.Name
	copyBinSU $product.Name
	copyBinSM $product.Name
}

#copyBin "ecm"
#copyBin "cloudium"
#copyBin "startia"

#copyBinSU "ecm"
#copyBinSU "cloudium"
#copyBinSU "startia"

#copyBinSM "ecm"
#copyBinSM "cloudium"
#copyBinSM "startia"

$Env:WORKSPACE + "\_projects" | Set-Location
Get-Location | Write-Host
$_commitMessage = $Env:BUILD_TAG + " (r." + $Env:SVN_REVISION_1 + ")"

foreach ($product in $productList)
{
	commit $product.Name $product.SvnUrl
}

#commit "ecm" ($Env:SVN_URL_2)
#commit "cloudium" ($Env:SVN_URL_3)
#commit "startia" ($Env:SVN_URL_4)
