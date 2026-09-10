$json = Get-Content 'F:\MEG_Reclamation\Saved\AutomationReports\20260910-211842-4777fd0003b94badb180a433c93cdd98\index.json' | ConvertFrom-Json
foreach ($t in $json.tests) {
    if ($t.state -ne 'Success') {
        Write-Output "FAILED: $($t.testDisplayName)"
        foreach ($e in $t.entries) {
            if ($e.event.type -eq 'Error') {
                Write-Output "ERROR: $($e.event.message)"
            }
        }
    }
}
