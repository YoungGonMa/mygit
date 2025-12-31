Import-Module -Name .\v2\Invoke-MsBuild.psm1

$script:testMode = $false
#$script:testMode = $true

$script:buildLibCommon32 = $false
$script:buildLibCommon64 = $false
$script:buildECMAgent = $false
$script:buildECMIF = $false
$script:buildOneRPC = $false
$script:buildMsgpackRpc32 = $false
$script:buildMsgpackRpc64 = $false
$script:buildUtilityIF = $false
$script:buildUtilityIF64 = $false
$script:buildLocalSyncIF = $false
$script:buildLocalSyncIF64 = $false
$script:buildMsgMgr = $false
$script:buildSURes32 = $false
$script:buildSURes64 = $false
$script:buildECMAgentManager = $false
$script:buildDriveManager = $false
$script:buildCheckinManager = $false
$script:buildLocalSyncManager = $false
$script:buildSolutionMgr = $false
$script:buildSmartUpdater = $false
$script:buildSmartUpdaterX = $false
$script:buildAssistService = $false
$script:buildRes = $false

$script:targetModules = @()

$script:MSBuildPath = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MsBuild\15.0\Bin\MSBuild.exe"
$script:VsDevCmdPath = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

$script:slnPath = $Env:WORKSPACE + "\_projects\sln\ci-release\"
$script:binPath = $Env:WORKSPACE + "\_projects\bin\"
$script:targetModuleString1 = $Env:COMMIT_TARGET
$script:targetModuleString2 = $Env:COMMIT_TARGET_SU
$script:targetModuleString3 = $Env:COMMIT_TARGET_SM
$script:targetModuleString4 = $Env:COMMIT_TARGET_RESDLL

if ($script:testMode) {
    # 테스트용 경로
    $curPath = Convert-Path . # current path - 'build' path 예) D:\Projects\DestinyOneX\trunk\build
    $script:slnPath = $curPath + "\..\sln\ci-release\"
    $script:binPath = $curPath + "\..\bin\"
    $script:targetModuleString1 = "DestinyECMAgent.exe" # DestinyECMAgent.exe,DestinyMsgMgr.exe,DestinyECMIF.dll,DestinyOneRPC.dll,DestinyUtilityIF.dll,DestinyUtilityIF64.dll,DestinyLocalSyncIF.dll,DestinyLocalSyncIF64.dll
    $script:targetModuleString2 = ""                    # DestinySmartUpdaterX.dll,DestinySUResEN.dll,DestinySUResKO.dll,DestinySUResZH.dll,DestinyECMAgentManager.dll,DestinyDriveManager.dll,DestinyCheckinManager.dll,DestinyLocalSyncManager.dll
    $script:targetModuleString3 = ""                    # DestinySolutionMgr.exe,DestinySmartUpdater.exe,DestinyAssist.exe
    $script:targetModuleString4 = ""                    # DestinyResEN.dll,DestinyResKO.dll,DestinyResZH.dll,DestinyResJA.dll,DestinyResEN64.dll,DestinyResKO64.dll,DestinyResZH64.dll,DestinyResJA64.dll
}

function exitIfFailed($buildResult) {
    If ($buildResult.BuildSucceeded -eq $true)
    {
        Write-Output ("Build completed successfully in {0:N1} seconds." -f $buildResult.BuildDuration.TotalSeconds)
    }
    ElseIf ($buildResult.BuildSucceeded -eq $false)
    {
        # type $buildResult.BuildErrorsLogFilePath
        Write-Output ("Build failed after {0:N1} seconds. Check the build log file '$($buildResult.BuildLogFilePath)' for errors." -f $buildResult.BuildDuration.TotalSeconds)
        Exit 1
    }
    ElseIf ($buildResult.BuildSucceeded -eq $null)
    {
        Write-Output "Unsure if build passed or failed: $($buildResult.Message)"
        Exit 1
    }
}

function buildOrClean($sln, $is32, $isBuild) {
    $msBuildParameters = ""

    # NodeReuse(nr) 옵션은 기본이 true 라서 MSBuild.exe 가 종료되지 않고 재사용된다.
    # 우리는 ExitCode 가 필요하기 때문에 재사용하면 안됨.
    If ($is32) {
        $msBuildParameters = "-p:Configuration=Release;BuildProjectReferences=false;PreferredToolArchitecture=x64;BuildInParallel=true;Platform=Win32 /nr:false -maxcpucount -t:"
    } Else {
        $msBuildParameters = "-p:Configuration=Release;BuildProjectReferences=false;PreferredToolArchitecture=x64;BuildInParallel=true;Platform=x64 /nr:false -maxcpucount -t:"
    }

    If ($isBuild) {
        $msBuildParameters += "build"
    } Else {
        $msBuildParameters += "clean"
    }

    $path = $script:slnPath + $sln

    Write-Host "Building Solution: " $sln " , is32:" $is32

    # cmd.exe /k " "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" & "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MsBuild\15.0\Bin\MSBuild.exe" "..\sln\ci-release\ECMAgent.sln" -p:Configuration=Release;BuildProjectReferences=false;PreferredToolArchitecture=x64;BuildInParallel=true;Platform=Win32 -maxcpucount -t:build /fileLoggerParameters:LogFile="C:\WINDOWS\sln\ci-release\ECMAgent.sln.msbuild.log";verbosity=normal /fileLoggerParameters1:LogFile="C:\WINDOWS\sln\ci-release\ECMAgent.sln.msbuild.errors.log";errorsonly /p:UseSharedCompilation=false  &  Exit" 

    $buildResult = Invoke-MsBuild -Path $path `
        -MsBuildParameters $msBuildParameters `
        -BuildLogDirectoryPath PathDirectory `
        -ShowBuildOutputInCurrentWindow `
        -WaitForExit `
        -MsBuildFilePath $script:MSBuildPath `
        -VisualStudioDeveloperCommandPromptFilePath $script:VsDevCmdPath

    exitIfFailed($buildResult)

    # Async(PassThru) 로 처리하는 경우, InCurrentWindow 옵션을 주면 ExitCode 를 받지 못함 (원인불명)
    
#    $p = Invoke-MsBuild -Path $path -MsBuildParameters $msBuildParameters -BuildLogDirectoryPath PathDirectory -PassThru -ShowBuildOutputInNewWindow #-ShowBuildOutputInCurrentWindow 
#    $p = Invoke-MsBuild -Path $path -MsBuildParameters $msBuildParameters -BuildLogDirectoryPath PathDirectory -PassThru -ShowBuildOutputInCurrentWindow 
#    if (!$p.HasExited)
#    {
#        $p.WaitForExit()
#    }
#    Write-Host "exit code: " $p.ExitCode
#    if ($p.ExitCode -eq 1)
#    {
#        Write-Output ("Build failed. "")
#        Exit
#    }
}

function clean($sln, $is32) {
    buildOrClean $sln $is32 $false
}

function buildSln($sln, $is32) {
    buildOrClean $sln $is32 $true
}

function build() {
    If ($script:buildLibCommon32) {
        buildSln "LibCommon.sln" $true
    }
    If ($script:buildLibCommon64) {
        buildSln "LibCommon.sln" $false
    }
	
	If ($script:buildMsgpackRpc32) {
        buildSln "MsgpackRpc.sln" $true
    }
	
	If ($script:buildMsgpackRpc64) {
        buildSln "MsgpackRpc.sln" $false
    }
	
	If ($script:buildOneRPC) {
        buildSln "OneRPC.sln" $true
    }
    If ($script:buildECMAgent) {
        buildSln "ECMAgent.sln" $true
    }
    If ($script:buildECMIF) {
        buildSln "ECMIF.sln" $true
    }

    If ($script:buildUtilityIF) {
        buildSln "UtilityIF.sln" $true
    }
    If ($script:buildUtilityIF64) {
        buildSln "UtilityIF.sln" $false
    }

    If ($script:buildLocalSyncIF) {
        buildSln "LocalsyncIF.sln" $true
    }
    If ($script:buildLocalSyncIF64) {
        buildSln "LocalsyncIF.sln" $false
    }

    If ($script:buildMsgMgr) {
        buildSln "MsgMgr.sln" $true
    }

    If ($script:buildSURes32) {
        buildSln "SURes.sln" $true
    }
    If ($script:buildSURes64) {
        buildSln "SURes.sln" $false
    }

    If ($script:buildECMAgentManager) {
        buildSln "ECMAgentManager.sln" $true
    }
    If ($script:buildDriveManager) {
        buildSln "DriveManager.sln" $true
    }
    If ($script:buildCheckinManager) {
        buildSln "CheckinManager.sln" $true
    }
    If ($script:buildLocalSyncManager) {
        buildSln "LocalSyncManager.sln" $true
    }

    If ($script:buildSolutionMgr) {
        buildSln "SolutionMgr.sln" $true
    }
    If ($script:buildSmartUpdater) {
        buildSln "SmartUpdater.sln" $true
    }
    If ($script:buildSmartUpdaterX) {
        buildSln "SmartUpdaterX.sln" $true
    }
    If ($script:buildAssistService) {
        buildSln "AssistService.sln" $true
    }

    If ($script:buildRes) {
        buildSln "Res.sln" $true
    }
}

function check() {
    Write-Host "MSBuildPath: " $script:MSBuildPath
    Write-Host "VsDevCmdPath: " $script:VsDevCmdPath
    Write-Host "Build target modules: " $script:targetModules

    If ($script:targetModules.Contains("DestinyECMAgent.exe")) {
        $script:buildLibCommon32 = $true
        $script:buildOneRPC = $true
		$script:buildMsgpackRpc32 = $true
        $script:buildECMAgent = $true
    }

    If ($script:targetModules.Contains("DestinyECMIF.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildOneRPC = $true
		$script:buildMsgpackRpc32 = $true
        $script:buildECMIF = $true
    }

    If ($script:targetModules.Contains("DestinyOneRPC.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildOneRPC = $true
    }

    If ($script:targetModules.Contains("DestinyUtilityIF.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildUtilityIF = $true
    }

    If ($script:targetModules.Contains("DestinyUtilityIF64.dll")) {
        $script:buildLibCommon64 = $true
        $script:buildUtilityIF64 = $true
    }

    If ($script:targetModules.Contains("DestinyLocalSyncIF.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildLocalSyncIF = $true
		$script:buildMsgpackRpc32 = $true
    }

    If ($script:targetModules.Contains("DestinyLocalSyncIF64.dll")) {
        $script:buildLibCommon64 = $true
        $script:buildLocalSyncIF64 = $true
		$script:buildMsgpackRpc64 = $true
    }

    If ($script:targetModules.Contains("DestinyMsgMgr.exe")) {
        $script:buildLibCommon32 = $true
        $script:buildMsgMgr = $true
    }

    If ($script:targetModules.Contains("DestinySUResEN.dll") -Or
        $script:targetModules.Contains("DestinySUResKO.dll") -Or
        $script:targetModules.Contains("DestinySUResZH.dll") -Or
        $script:targetModules.Contains("DestinySUResJA.dll") ) {
        $script:buildSURes32 = $true
    }

    If ($script:targetModules.Contains("DestinySUResEN64.dll") -Or
        $script:targetModules.Contains("DestinySUResKO64.dll") -Or
        $script:targetModules.Contains("DestinySUResZH64.dll") -Or
        $script:targetModules.Contains("DestinySUResJA64.dll") ) {
        $script:buildSURes64 = $true
    }

    If ($script:targetModules.Contains("DestinyECMAgentManager.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildECMAgentManager = $true
    }

    If ($script:targetModules.Contains("DestinyDriveManager.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildDriveManage = $true
    }

    If ($script:targetModules.Contains("DestinybuildLocalSyncManager.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildLocalSyncManager = $true
    }
    
    If ($script:targetModules.Contains("DestinyCheckinManager.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildCheckinManager = $true
    }

    If ($script:targetModules.Contains("DestinySolutionMgr.exe")) {
        $script:buildLibCommon32 = $true
        $script:buildSolutionMgr = $true
    }

    If ($script:targetModules.Contains("DestinySmartUpdater.exe")) {
        $script:buildLibCommon32 = $true
        $script:buildSmartUpdater = $true
    }

    If ($script:targetModules.Contains("DestinySmartUpdaterX.dll")) {
        $script:buildLibCommon32 = $true
        $script:buildSmartUpdaterX = $true
    }

    If ($script:targetModules.Contains("DestinyAssist.exe")) {
        $script:buildLibCommon32 = $true
        $script:buildAssistService = $true
    }

    If ($script:targetModules.Contains("DestinyRes.dll") -Or
        $script:targetModules.Contains("en-US_DestinyRes.dll.mui") -Or
        $script:targetModules.Contains("ko-KR_DestinyRes.dll.mui") -Or
        $script:targetModules.Contains("zh-CN_DestinyRes.dll.mui") -Or
        $script:targetModules.Contains("ja-JP_DestinyRes.dll.mui") ) {
        $script:buildRes = $true
    }

}
function cleanAll {
    
    Get-ChildItem $script:binPath -Attributes !Directory -Recurse -Include *.* -Exclude PTCacheMng.dll,app.pdb,lib.pdb,*.bat | Remove-Item

    # Win32 ================================

    clean "LibCommon.sln" $true

    clean "ECMAgent.sln" $true
    clean "ECMIF.sln" $true
    clean "OneRPC.sln" $true
	clean "MsgpackRpc.sln" $true
    clean "UtilityIF.sln" $true
    clean "LocalsyncIF.sln" $true
    clean "MsgMgr.sln" $true

    clean "SolutionMgr.sln" $true
    clean "SmartUpdater.sln" $true
    clean "SmartUpdaterX.sln" $true
    clean "AssistService.sln" $true

    clean "Res.sln" $true
    clean "SURes.sln" $true

    clean "ECMAgentManager.sln" $true
    clean "DriveManager.sln" $true
    clean "CheckinManager.sln" $true
    clean "LocalSyncManager.sln" $true

    # x64 ================================

    clean "LibCommon.sln" $false

    clean "UtilityIF.sln" $false
    clean "LocalsyncIF.sln" $false

    clean "SURes.sln" $false
}

if ($script:targetModuleString1) {
    $script:targetModules += $script:targetModuleString1.Split(",")
}
if ($script:targetModuleString2) {
    $script:targetModules += $script:targetModuleString2.Split(",")
}
if ($script:targetModuleString3) {
    $script:targetModules += $script:targetModuleString3.Split(",")
}
if ($script:targetModuleString4) {
    $script:targetModules += $script:targetModuleString4.Split(",")
}

# Let's go
check
if (!$script:testMode) {
    cleanAll
}
build
