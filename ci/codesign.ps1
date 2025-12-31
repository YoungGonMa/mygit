$COMMIT_MESSAGE_PREFIX = "codesign r"
$IGNORE_KEY = "commit only"

function sign($file)
{
    if ((Get-Item $file) -is [system.io.fileinfo]) {
        $ext = [System.IO.Path]::GetExtension($file)
        if ($ext -ieq ".exe" -Or $ext -ieq ".dll" -Or $ext -ieq ".ocx" -Or $ext -ieq ".cab") {
            .\sign\codesign.bat $file
        }
    }
}

svn.exe update
$lastRevision = svn.exe info | Select-String -Pattern "Last Changed Rev: "
$lastRevision = $lastRevision -replace "Last Changed Rev: ",""
Write-Host "Last Changed Rev: " $lastRevision

# 커밋 메시지가 $COMMIT_MESSAGE_PREFIX로 시작하면 무시
$log = svn.exe log -r $lastRevision
$IGNORE_COMMIT_MESSAGE = $COMMIT_MESSAGE_PREFIX
$i = 0
$ignore = $false
foreach ($line in $log.Split("`n")) {
    $i = $i + 1
    if ($i -eq 4) {
        # 4번째 줄이 커밋메시지의 시작
        if ($line.StartsWith($IGNORE_COMMIT_MESSAGE)) {
            $ignore = $true
            Write-Host "ignored to codesign"
        }
		# 브랜치 생성 등 codesign을 하지 않길 원하는 경우
		if ($line.IndexOf($IGNORE_KEY) -ge 0) {
			$ignore = $true
			Write-Host "ignore-key included, ignore"
		}		
        break
    }
}

if ($ignore -eq $false) {
    $lastRevisionMinus1 = $lastRevision - 1
    $param = "$lastRevisionMinus1" + ":" + $lastRevision
    $diff = svn.exe diff --summarize --ignore-properties -r $param

    $ADD_STRING = "A       "
    $MOD_STRING = "M       "
    foreach ($line in $diff.Split("`n")) {
        if ($line.StartsWith($ADD_STRING)) {
            $line = $line -replace $ADD_STRING,""
            Write-Host "A: " $line
            sign $line
        }
        elseif ($line.StartsWith($MOD_STRING)) {
            $line = $line -replace $MOD_STRING,""
            Write-Host "M: " $line
            sign $line
        }
    }

    $commitMessage = $COMMIT_MESSAGE_PREFIX + $lastRevision
    $commitArguments += "-m","`"$commitMessage`""
    svn.exe commit $commitArguments
}