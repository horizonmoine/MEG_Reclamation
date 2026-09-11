param(
    [string]$EngineCommand = 'F:\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe',
    [string]$Project = (Join-Path $PSScriptRoot 'MEG_Reclamation.uproject'),
    [ValidatePattern('^[a-zA-Z0-9 ._-]+$')][string]$TestFilter = 'Project.Functional Tests.MEG',
    [ValidateRange(1, 7200)][int]$TimeoutSeconds = 600,
    [string]$ReportRoot = (Join-Path $PSScriptRoot 'Saved\AutomationReports'),
    # Offline mode never launches Unreal and is not a gameplay validation.
    [string]$ValidateReportPath,
    [int]$ReportedProcessExitCode = 0
)
$ErrorActionPreference = 'Stop'
$runDirectory = Join-Path $ReportRoot ((Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runDirectory -Force | Out-Null
$summary = [ordered]@{ mode = 'engine'; filter = $TestFilter; expected = 0; discovered = 0; passed = 0; failed = 0; skipped = 0; missing = @(); tests = @(); processExitCode = $null; timedOut = $false; passedGate = $false; error = $null }
$process = $null
try {
    $testSource = Join-Path (Split-Path -Parent $Project) 'Source\MEG_Reclamation\Tests'
    $declarations = [regex]::Matches(((Get-ChildItem -LiteralPath $testSource -Filter *.cpp -Recurse | ForEach-Object { Get-Content -LiteralPath $_.FullName -Raw }) -join "`n"), 'IMPLEMENT_SIMPLE_AUTOMATION_TEST\s*\([^,]+,\s*"([^"]+)"')
    $expected = @($declarations | ForEach-Object { $_.Groups[1].Value } | Where-Object { $_.StartsWith($TestFilter, [StringComparison]::OrdinalIgnoreCase) })
    $summary.expected = $expected.Count
    if ($expected.Count -eq 0) { throw 'Filter matches no declared MEG tests.' }
    if ($ValidateReportPath) {
        $summary.mode = 'offline-report-validation'
        $reportPath = $ValidateReportPath
        $summary.processExitCode = $ReportedProcessExitCode
    } else {
        if (-not (Test-Path -LiteralPath $EngineCommand)) { throw "Unreal command missing: $EngineCommand" }
        if (-not (Test-Path -LiteralPath $Project)) { throw "Project missing: $Project" }
        $engineLog = Join-Path $runDirectory 'engine.log'
        $arguments = @(('"' + $Project + '"'), ('-ExecCmds="Automation RunTests ' + $TestFilter + '"'), '-unattended', '-nopause', '-nullrhi', '-nosplash', '-testexit="Automation Test Queue Empty"', ('-ReportExportPath="' + $runDirectory + '"'), ('-abslog="' + $engineLog + '"'))
        $process = Start-Process -FilePath $EngineCommand -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $runDirectory 'stdout.log') -RedirectStandardError (Join-Path $runDirectory 'stderr.log')
        # Keep the native handle open so PowerShell can read the real exit code after exit.
        $null = $process.Handle
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            $summary.timedOut = $true
            throw "Automation exceeded $TimeoutSeconds seconds."
        }
        $process.WaitForExit()
        $process.Refresh()
        $summary.processExitCode = $process.ExitCode
        if ($null -eq $summary.processExitCode) { throw 'Engine exit code unavailable; refusing to assume success.' }
        $reportPath = Join-Path $runDirectory 'index.json'
    }
    if (-not (Test-Path -LiteralPath $reportPath)) { throw 'No nominative index.json report produced.' }
    $report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
    $results = @($report.tests | Where-Object { $_.fullTestPath -and $_.fullTestPath.StartsWith($TestFilter, [StringComparison]::OrdinalIgnoreCase) })
    $summary.tests = @($results | Select-Object fullTestPath, state, errors, warnings, duration)
    $summary.discovered = $results.Count
    $summary.missing = @($expected | Where-Object { $_ -notin $results.fullTestPath })
    $summary.passed = @($results | Where-Object { $_.state -eq 'Success' -and $null -ne $_.errors -and $_.errors -eq 0 }).Count
    $summary.failed = @($results | Where-Object { $_.state -eq 'Fail' -or $_.errors -gt 0 }).Count
    $summary.skipped = $results.Count - $summary.passed - $summary.failed
    if ($null -ne $summary.processExitCode -and $summary.processExitCode -ne 0) { throw "Engine process exit: $($summary.processExitCode)" }
    if ($results.Count -ne $expected.Count -or $summary.missing.Count -gt 0 -or @($results.fullTestPath | Select-Object -Unique).Count -ne $results.Count) { throw 'Missing, unexpected, or duplicate test results.' }
    if ($summary.passed -ne $expected.Count -or $summary.failed -gt 0 -or $summary.skipped -gt 0) { throw 'One or more tests failed, were skipped, or did not complete.' }
    if (-not $ValidateReportPath -and (@(Get-Content -LiteralPath $engineLog | Where-Object { $_ -match 'LogAutomationTest: Error:' -and $_ -notmatch 'Condition failed' }).Count -gt 0)) { throw 'Automation errors present in engine log.' }
    $summary.passedGate = $true
} catch {
    $summary.error = $_.Exception.Message
    Write-Host "[FAIL] $($summary.error)" -ForegroundColor Red
} finally {
    # Only stop the process launched here, never another editor session.
    if ($process -and -not $process.HasExited) { Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue }
    $summary | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $runDirectory 'runner-summary.json') -Encoding UTF8
    Write-Host "Report: $runDirectory"
    Write-Host "Mode=$($summary.mode) Expected=$($summary.expected) Discovered=$($summary.discovered) Pass=$($summary.passed) Fail=$($summary.failed) Skip=$($summary.skipped)"
}
if ($summary.passedGate) { exit 0 }
exit 1
