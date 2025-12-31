Import-Module -Name .\v2\Invoke-MsBuild.psm1

$script:targetModules = @()
# 테스트 모드 설정: Win32, x64, ARM, ARM64 등 필요에 따라 설정
$script:testMode = $false
#$script:testMode = $true

# MSBuild 및 개발자 명령 프롬프트 경로
$script:MSBuildPath = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MsBuild\15.0\Bin\MSBuild.exe"
$script:VsDevCmdPath = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

# 솔루션 및 바이너리 경로
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

	$script:targetModuleString1 = "DestinyECMIF.dll,DestinyECMIF64.dll,DestinyUtilityIF.dll,DestinyUtilityIF64.dll" # DestinyECMAgent.exe,DestinyMsgMgr.exe,DestinyECMIF.dll,DestinyECMIF64.dll,DestinyUtilityIF.dll,DestinyUtilityIF64.dll,DestinyLocalSyncIF.dll,DestinyLocalSyncIF64.dll
    # $script:targetModuleString1 = "DestinyECMAgent.exe,DestinyECMIF.dll,DestinyUtilityIF64.dll" # DestinyECMAgent.exe,DestinyMsgMgr.exe,DestinyECMIF.dll,DestinyECMIF64.dll,DestinyUtilityIF.dll,DestinyUtilityIF64.dll,DestinyLocalSyncIF.dll,DestinyLocalSyncIF64.dll
	# $script:targetModuleString1 = "DestinyECMAgent.exe" # DestinyECMAgent.exe,DestinyMsgMgr.exe,DestinyECMIF.dll,DestinyECMIF64.dll,DestinyUtilityIF.dll,DestinyUtilityIF64.dll,DestinyLocalSyncIF.dll,DestinyLocalSyncIF64.dll
    $script:targetModuleString2 = ""                    # DestinySmartUpdaterX.dll,DestinySUResEN.dll,DestinySUResKO.dll,DestinySUResZH.dll,DestinyECMAgentManager.dll,DestinyDriveManager.dll,DestinyCheckinManager.dll,DestinyLocalSyncManager.dll
    $script:targetModuleString3 = ""                    # DestinySolutionMgr.exe,DestinyAssist.exe
    $script:targetModuleString4 = ""                    # DestinyResEN.dll,DestinyResKO.dll,DestinyResZH.dll,DestinyResJA.dll,DestinyResEN64.dll,DestinyResKO64.dll,DestinyResZH64.dll,DestinyResJA64.dll	
	
}

# 플랫폼별 기본 빌드 설정 및 빌드 순서 통합
# 아래 buildConfigurations 는 빌드가 가능한 범위를 표시 합니다.
# 아래 이외에 ARM/ARM64 빌드를 추가 하고자 한다면 각 프로젝트 ###.sln 전처리기 정의에 _M_ARM64, _M_ARM를 알맞게 추가 바랍니다.
# (buildConfigurations check() 에서 초기화 됩니다.)
$script:buildConfigurations = @(
    @{ Module = "LibCommon";        Platforms = @("Win32", "x64", "ARM", "ARM64") },
    @{ Module = "MsgpackRpc";       Platforms = @("Win32", "x64", "ARM", "ARM64") },
    @{ Module = "UtilityIF";        Platforms = @("Win32", "x64", "ARM", "ARM64") },
    @{ Module = "ECMIF";            Platforms = @("Win32", "x64", "ARM", "ARM64") },
	@{ Module = "LogCollector";     Platforms = @("Win32") },
    @{ Module = "ECMAgent";		    Platforms = @("Win32") },
    @{ Module = "LocalSyncIF";      Platforms = @("Win32", "x64") },
    @{ Module = "MsgMgr";           Platforms = @("Win32") },
    @{ Module = "SURes";            Platforms = @("Win32", "x64") },
    @{ Module = "ECMAgentManager";  Platforms = @("Win32") },
    @{ Module = "DriveManager";     Platforms = @("Win32") },
    @{ Module = "CheckinManager";   Platforms = @("Win32") },
    @{ Module = "LocalSyncManager"; Platforms = @("Win32") },
    @{ Module = "SolutionMgr";      Platforms = @("Win32") },
    @{ Module = "Updater";          Platforms = @("Win32") },
    @{ Module = "UpdateDownloader"; Platforms = @("Win32") },
    @{ Module = "UpdateStatusBoard";Platforms = @("Win32") },
    @{ Module = "SmartUpdaterX";    Platforms = @("Win32") },
    @{ Module = "AssistService";    Platforms = @("Win32") },
    @{ Module = "Res";              Platforms = @("Win32") },
	@{ Module = "InstallerLauncher";Platforms = @("Win32") }
)

function check() {
    Write-Host "MSBuildPath: " $script:MSBuildPath
    Write-Host "VsDevCmdPath: " $script:VsDevCmdPath
    Write-Host "Build target modules: " $script:targetModules

	# buildConfigurations 리스트를 초기화
	$script:buildConfigurations = @()

    # targetModules 값에 따라 buildConfigurations 리스트를 추가
	
	# COMMIT_TARGET
    If ($script:targetModules.Contains("DestinyECMAgent.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "MsgpackRpc"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "ECMAgent"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyECMIF.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "MsgpackRpc"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "ECMIF"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyECMIF64.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("x64", "ARM64") }
        $script:buildConfigurations += @{ Module = "MsgpackRpc"; Platforms = @("x64", "ARM64") }
        $script:buildConfigurations += @{ Module = "ECMIF"; Platforms = @("x64", "ARM64") }
    }

    If ($script:targetModules.Contains("DestinyUtilityIF.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "UtilityIF"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyUtilityIF64.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("x64", "ARM64") }
        $script:buildConfigurations += @{ Module = "UtilityIF"; Platforms = @("x64", "ARM64") }
    }

    If ($script:targetModules.Contains("DestinyLocalSyncIF.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "MsgpackRpc"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "LocalSyncIF"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyLocalSyncIF64.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("x64") }
        $script:buildConfigurations += @{ Module = "MsgpackRpc"; Platforms = @("x64") }
        $script:buildConfigurations += @{ Module = "LocalSyncIF"; Platforms = @("x64") }
    }

    If ($script:targetModules.Contains("DestinyMsgMgr.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "MsgMgr"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("LogCollector.exe")) {
		$script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "LogCollector"; Platforms = @("Win32") }
    }

	# COMMIT_TARGET_SU
    If ($script:targetModules.Contains("DestinySUResEN.dll") -Or
        $script:targetModules.Contains("DestinySUResKO.dll") -Or
        $script:targetModules.Contains("DestinySUResZH.dll") -Or
        $script:targetModules.Contains("DestinySUResJA.dll")) {
        $script:buildConfigurations += @{ Module = "SURes"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyECMAgentManager.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "ECMAgentManager"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyDriveManager.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "DriveManager"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyCheckinManager.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "CheckinManager"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyLocalSyncManager.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "LocalSyncManager"; Platforms = @("Win32") }
    }

	# COMMIT_TARGET_SM
    If ($script:targetModules.Contains("DestinySolutionMgr.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "SolutionMgr"; Platforms = @("Win32") }
    }
	
	If ($script:targetModules.Contains("DestinyUpdateStatusBoard.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "UpdateStatusBoard"; Platforms = @("Win32") }
    }
	
	If ($script:targetModules.Contains("DestinyUpdateDownloader.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "UpdateDownloader"; Platforms = @("Win32") }
    }

	If ($script:targetModules.Contains("DestinySmartUpdater.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "Updater"; Platforms = @("Win32") }
    }
	
    If ($script:targetModules.Contains("DestinySmartUpdaterX.dll")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "SmartUpdaterX"; Platforms = @("Win32") }
    }

    If ($script:targetModules.Contains("DestinyAssist.exe")) {
        $script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "AssistService"; Platforms = @("Win32") }
    }

	# COMMIT_INSTALLER
	# 인스톨러를 빌드할 때, InstallerLauncher가 항상 함께 빌드되어 인스톨러에 포함되어야 한다.
	# 참고로 InstallerLauncher는 독립적으로 빌드되어 커밋될 일은 없다.
	If ($Env:COMMIT_INSTALLER -and $Env:COMMIT_INSTALLER.Contains("installer")) {
		$script:buildConfigurations += @{ Module = "LibCommon"; Platforms = @("Win32") }
        $script:buildConfigurations += @{ Module = "InstallerLauncher"; Platforms = @("Win32") }
    }

	# COMMIT_TARGET_RESDLL
    If ($script:targetModules.Contains("DestinyRes.dll") -Or
        $script:targetModules.Contains("en-US_DestinyRes.dll.mui") -Or
        $script:targetModules.Contains("ko-KR_DestinyRes.dll.mui") -Or
        $script:targetModules.Contains("zh-CN_DestinyRes.dll.mui") -Or
        $script:targetModules.Contains("ja-JP_DestinyRes.dll.mui")) {
        $script:buildConfigurations += @{ Module = "Res"; Platforms = @("Win32") }
    }
		
	# 결과 출력
	Write-Host "Before merging buildConfigurations :"
	$script:buildConfigurations | ForEach-Object { Write-Host "Module: $($_.Module), Platforms: $($_.Platforms -join ', ')" }		
		
	# 정렬할 모듈 순서 (그룹별 우선순위)
	$moduleOrder = @(
		"LibCommon", "MsgpackRpc", "UtilityIF", "ECMIF", "LogCollector", 
		"ECMAgent", "LocalSyncIF", "MsgMgr", "SURes", "ECMAgentManager", 
		"DriveManager", "CheckinManager", "LocalSyncManager", "SolutionMgr", 
		"Updater", "UpdateDownloader", "UpdateStatusBoard", "SmartUpdaterX", 
		"AssistService", "Res", "InstallerLauncher"
	)

	# 초기화
	$script:buildConfigurations2 = @()

	# 복사 로직 (중복 제거 후 복사)
	$script:buildConfigurations | ForEach-Object {
		$item = $_

		# $script:buildConfigurations2에 동일한 항목이 있는지 확인 (Module과 Platforms 기준으로)
		$exists = $script:buildConfigurations2 | Where-Object {
			$_.Module -eq $item.Module -and ($_.Platforms -join ",") -eq ($item.Platforms -join ",")
		}

		# 항목이 없으면 추가
		if (-not $exists) {
			$script:buildConfigurations2 += [PSCustomObject]@{
				Module = $item.Module
				Platforms = $item.Platforms
			}
		}
	}

	# $script:buildConfigurations를 비우고 $script:buildConfigurations2 값을 할당
	$script:buildConfigurations = @()

	# 그룹별 우선순위로 정렬
	$script:buildConfigurations2 = $script:buildConfigurations2 | Sort-Object { 
		# 우선순위 배열에서 해당 모듈의 인덱스를 기준으로 정렬
		$moduleOrder.IndexOf($_.Module)
	}

	# 결과 출력
	Write-Host "After merging buildConfigurations :"
	$script:buildConfigurations = $script:buildConfigurations2
	$script:buildConfigurations | ForEach-Object {
		Write-Host "Module: $($_.Module), Platforms: $($_.Platforms -join ', ')"
	}
}

# MSBuild 매개변수 생성 함수
function Get-MSBuildParameters($platform, $isBuild) {
    $action = if ($isBuild) { "build" } else { "clean" }
    return "-p:Configuration=Release;BuildProjectReferences=false;Platform=$platform;BuildInParallel=true /nr:false -maxcpucount -t:$action"
}

# 빌드 실패 시 종료
function exitIfFailed($buildResult) {
    if ($buildResult.BuildSucceeded -eq $true) {
        Write-Output ("Build completed successfully in {0:N1} seconds." -f $buildResult.BuildDuration.TotalSeconds)
    } else {
        Write-Output ("Build failed after {0:N1} seconds. Log: '$($buildResult.BuildLogFilePath)'." -f $buildResult.BuildDuration.TotalSeconds)
        Exit 1
    }
}

# 빌드/클린 실행 함수
function buildOrClean($sln, $platform, $isBuild) {
    $parameters = Get-MSBuildParameters $platform $isBuild
    $path = $script:slnPath + $sln
    Write-Host "Processing Solution: $sln, Platform: $platform, Action: $(if ($isBuild) { 'Build' } else { 'Clean' })"

    $buildResult = Invoke-MsBuild -Path $path `
        -MsBuildParameters $parameters `
        -BuildLogDirectoryPath PathDirectory `
        -ShowBuildOutputInCurrentWindow `
        -WaitForExit `
        -MsBuildFilePath $script:MSBuildPath `
        -VisualStudioDeveloperCommandPromptFilePath $script:VsDevCmdPath

    exitIfFailed($buildResult)
}

# 클린 함수
function clean($sln, $platform) {
    buildOrClean $sln $platform $false
}

# 빌드 함수
function buildSln($sln, $platform) {
    buildOrClean $sln $platform $true
}

# 모든 빌드 대상 실행
function build() {
    foreach ($config in $script:buildConfigurations) {
        $module = $config.Module
        foreach ($platform in $config.Platforms) {
            buildSln "$module.sln" $platform
        }
    }
}

# 클린 작업 실행
function cleanAll {
    Get-ChildItem $script:binPath -Attributes !Directory -Recurse -Include *.* -Exclude PTCacheMng.dll,app.pdb,lib.pdb,*.bat | Remove-Item -Force
    foreach ($config in $script:buildConfigurations) {
        $module = $config.Module
        foreach ($platform in $config.Platforms) {
            clean "$module.sln" $platform
        }
    }
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

# $script:targetModules = @("DestinyECMIF.dll", "DestinyUtilityIF64.dll", "DestinyMsgMgr.exe")

# 실행
check
if ($script:testMode) {
    Write-Host "Running in test mode with platform: $script:testMode"
	cleanAll
    build
} else {
    Write-Host "Running in normal mode"
    cleanAll
    build
}
