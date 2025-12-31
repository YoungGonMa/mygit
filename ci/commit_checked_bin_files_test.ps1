
Write-Host "`$Env:WORKSPACE: $Env:WORKSPACE"

$_files = ""
$_filesSU = ""
    # DestinySmartUpdaterX.dll
    # DestinySUResEN.dll, DestinySUResKO.dll, DestinySUResZH.dll
    # DestinyECMAgentManager.dll, DestinyDriveManager.dll, DestinyCheckinManager.dll, DestinyLocalSyncManager.dll
$_filesSM = ""
    # DestinySolutionMgr.exe
    # DestinySmartUpdater.exe

Write-Host "COMMIT_TARGET: `"$Env:COMMIT_TARGET`""
if ($Env:COMMIT_TARGET) {
    $_files = $Env:COMMIT_TARGET.Split(",")
}

Write-Host "COMMIT_TARGET_RESDLL: `"$Env:COMMIT_TARGET_RESDLL`""
if ($Env:COMMIT_TARGET_RESDLL) {
    $_files += $Env:COMMIT_TARGET_RESDLL.Split(",")
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
        if ($product -eq "cloudium")
        {
            if ($item -eq "DestinyECMAgent.exe" -Or $item -eq "DestinyECMIF.dll")
            {
                $binPath += "cloudium\"
            }
        }

        if ($item -eq "DestinyECMIF.dll" -Or $item -eq "DestinyOneRPC.dll" -Or $item -eq "abc.dll")
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
    $destPath = $Env:WORKSPACE + "\_distribute\" + $product + "\solution\DestinySolutionMgr\"

    foreach ($item in $_filesSU)
    {
        $binPath + $item | Copy-Item -Destination $destPath -Force
    }
}

function copyBinSM($product)
{
    $binPath0 = $Env:WORKSPACE + "\_projects\bin\"
    $destPath = $Env:WORKSPACE + "\_distribute\" + $product + "\solution\DestinySolutionMgrX\"

    foreach ($item in $_filesSM)
    {
        $binPath = $binPath0
        if (!$product.StartsWith("ecm") -And $item -eq "DestinySmartUpdater.exe")
        {
            $binPath += "cloudium\"
        }

        $binPath + $item | Copy-Item -Destination $destPath -Force
    }
}

function commit($commitMessage)
{
    $argumentString = ""

    foreach ($item in $_files)
    {
         if ($item -eq "DestinyECMIF.dll" -Or $item -eq "DestinyOneRPC.dll" -Or $item -eq "abc.dll")
         {
            $argumentString += "solution\" + "DestinyECMAgent\" + $item +"|"
            $argumentString += "solution\" + "DestinyDrive\" + $item +"|"
         }
         elseif ($item -eq "DestinyLocalSyncIF.dll" -Or $item -eq "DestinyLocalSyncIF64.dll")
         {
            $argumentString += "solution\" + "DestinyLocalSync\" + $item +"|"
         }
         else # DestinyECMAgent.exe DestinyUtilityIF.dll DestinyUtilityIF64.dll
              # DestinyResEN.dll DestinyResKO.dll DestinyResEN64.dll DestinyResKO64.dll
         {
            $argumentString += "solution\" + "DestinyECMAgent\" + $item +"|"
         }
    }
    foreach ($item in $_filesSU)
    {
         $argumentString += "solution\" + "DestinySolutionMgr\" + $item +"|"
    }
    foreach ($item in $_filesSM)
    {
         $argumentString += "solution\" + "DestinySolutionMgrX\" + $item +"|"
    }

    if ($argumentString.Length -gt 0)
    {
        $argumentString = $argumentString.Substring(0,$argumentString.Length-1) # remove last '|'
    }
    $arguments = $argumentString.split("|")

    $arguments += "-m","`"$commitMessage`""

    svn.exe commit $arguments
    Write-Host $arguments
}

copyBin "ecm"
copyBin "cloudium"

copyBinSU "ecm"
copyBinSU "cloudium"

copyBinSM "ecm"
copyBinSM "cloudium"

$Env:WORKSPACE + "\_projects" | Set-Location
Get-Location | Write-Host
$commitMessage = $Env:BUILD_TAG + " (r" + $Env:SVN_REVISION_1 + ")"

$Env:WORKSPACE + "\_distribute\ecm" | Set-Location
Get-Location | Write-Host
commit($commitMessage + " - ecm")

$Env:WORKSPACE + "\_distribute\cloudium" | Set-Location
Get-Location | Write-Host
commit($commitMessage + " - cloudium")
