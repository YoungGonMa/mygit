
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
    $destPath0 = $Env:WORKSPACE + "\_distribute\" + $product + "\CONVERGENCE\"

    foreach ($item in $_files)
    {
        $binPath = $binPath0
        if ($product -eq "cloudium")
        {
            if ($item -eq "DestinyECMAgent.exe" -Or $item -eq "DestinyECMIF.dll")
            {
                $binPath += "cloudium\"
            }
        }

        if ($item -eq "DestinyECMIF.dll" -Or $item -eq "DestinyOneRPC.dll")
        {
            $destPath = $destPath0 + "DestinyECMAgent\"
            $binPath + $item | Copy-Item -Destination $destPath -Force

            $destPath = $destPath0 + "DestinyDrive\"
            $binPath + $item | Copy-Item -Destination $destPath -Force
         }
         elseif ($item -eq "DestinyLocalSyncIF.dll" -Or $item -eq "DestinyLocalSyncIF64.dll")
         {
            $destPath = $destPath0 + "DestinyLocalSync\"
            $binPath + $item | Copy-Item -Destination $destPath -Force
         }
         else # DestinyECMAgent.exe DestinyUtilityIF.dll DestinyUtilityIF64.dll
              # DestinyResEN.dll DestinyResKO.dll DestinyResEN64.dll DestinyResKO64.dll
         {
            $destPath = $destPath0 + "DestinyECMAgent\"
            $binPath + $item | Copy-Item -Destination $destPath -Force
         }
    }
}

function copyBinSU($product)
{
    $binPath = $Env:WORKSPACE + "\_projects\bin\"
    $destPath = $Env:WORKSPACE + "\_distribute\" + $product + "\CONVERGENCE\DestinySolutionMgr\"

    foreach ($item in $_filesSU)
    {
        $binPath + $item | Copy-Item -Destination $destPath -Force
    }
}

function copyBinSM($product)
{
    $binPath0 = $Env:WORKSPACE + "\_projects\bin\"
    $destPath = $Env:WORKSPACE + "\_distribute\" + $product + "\CONVERGENCE\DestinySolutionMgrX\"
    $destPathAlt = $Env:WORKSPACE + "\_distribute\" + $product + "\CONVERGENCE\DestinySolutionMgr\"

    foreach ($item in $_filesSM)
    {
        $binPath = $binPath0
        if (!$product.StartsWith("ecm") -And $item -eq "DestinySmartUpdater.exe")
        {
            $binPath += "cloudium\"
        }
        if ($item -eq "DestinySmartUpdaterX.dll")
        {
            $binPath + $item | Copy-Item -Destination $destPathAlt -Force
        }
        else
        {
            $binPath + $item | Copy-Item -Destination $destPath -Force
        }
    }
}

function commit($commitMessage, [ref]$items)
{
    $argumentString = ""

    foreach ($item in $_files)
    {
         if ($item -eq "DestinyECMIF.dll" -Or $item -eq "DestinyOneRPC.dll")
         {
            $argumentString += "CONVERGENCE\" + "DestinyECMAgent\" + $item +"|"
            $argumentString += "CONVERGENCE\" + "DestinyDrive\" + $item +"|"
         }
         elseif ($item -eq "DestinyLocalSyncIF.dll" -Or $item -eq "DestinyLocalSyncIF64.dll")
         {
            $argumentString += "CONVERGENCE\" + "DestinyLocalSync\" + $item +"|"
         }
         elseif ($item -ne "") # DestinyECMAgent.exe DestinyUtilityIF.dll DestinyUtilityIF64.dll
                               # DestinyResEN.dll DestinyResKO.dll DestinyResEN64.dll DestinyResKO64.dll
         {
            $argumentString += "CONVERGENCE\" + "DestinyECMAgent\" + $item +"|"
         }
    }
    foreach ($item in $_filesSU)
    {
        if ($item -ne "") 
        {
            $argumentString += "CONVERGENCE\" + "DestinySolutionMgr\" + $item +"|"
        }
    }
    foreach ($item in $_filesSM)
    {
        if ($item -eq "DestinySmartUpdaterX.dll")
        {
            $argumentString += "CONVERGENCE\" + "DestinySolutionMgr\" + $item +"|"
        }
        elseif ($item -ne "")
        {
            $argumentString += "CONVERGENCE\" + "DestinySolutionMgrX\" + $item +"|"
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

# copyBin "ecm"
copyBin "cloudium"

# copyBinSU "ecm"
copyBinSU "cloudium"

# copyBinSM "ecm"
copyBinSM "cloudium"

$Env:WORKSPACE + "\_projects" | Set-Location
Get-Location | Write-Host
$_commitMessage = $Env:BUILD_TAG + " (r." + $Env:SVN_REVISION_1 + ")"

# $Env:WORKSPACE + "\_distribute\ecm" | Set-Location
# Get-Location | Write-Host
# $items = $null
# commit ($_commitMessage + " - ecm") ([ref]$items)
# 
# if ($Env:COPY_NEXT_BRANCH -eq "true")
# {
#     svn.exe update
#     $newRevsion = svn.exe info | Select-String -Pattern "Revision: "
#     $newRevsion = $newRevsion -replace "Revision: ",""
#     Write-Host "newRevsion: " $newRevsion
# 
#     $Env:WORKSPACE + "\_distribute\ecm_next" | Set-Location
#     Get-Location | Write-Host
#     svn.exe update
# 
#     $svnRelativePath = $Env:SVN_URL_2 -replace "https://stardust.cyberdigm.co.kr/DestinyONE",""
#     merge "ecm" ($Env:SVN_URL_2) ($newRevsion) ("{0} - ecm: Copied from r{1} in {2}" -f $_commitMessage, $newRevsion, $svnRelativePath) ($items)
# }

$Env:WORKSPACE + "\_distribute\cloudium" | Set-Location
Get-Location | Write-Host
$items = $null
commit ($_commitMessage + " - cloudium")  ([ref]$items)
if ($Env:COPY_NEXT_BRANCH  -eq "true")
{
    svn.exe update
    $newRevsion = svn.exe info | Select-String -Pattern "Revision: "
    $newRevsion = $newRevsion -replace "Revision: ",""
    Write-Host "newRevsion: " $newRevsion

    $Env:WORKSPACE + "\_distribute\cloudium_next" | Set-Location
    Get-Location | Write-Host
    svn.exe update

    $svnRelativePath = $Env:SVN_URL_3 -replace "https://stardust.cyberdigm.co.kr/DestinyONE",""
    merge "cloudium" ($Env:SVN_URL_3) ($newRevsion) ("{0} - cloudium: Copied from r{1} in {2}" -f $_commitMessage, $newRevsion, $svnRelativePath) ($items)
}
