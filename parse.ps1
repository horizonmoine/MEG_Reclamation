$data = Get-Content 'F:\MEG_Reclamation\Saved\AutomationReports\20260910-211313-0314d64378d44ccea1e8ee45f78423bb\index.json' | ConvertFrom-Json
foreach ($t in $data.tests) {
    if ($t.state -ne 'Success') {
        Write-Host "FAILED TEST: " $t.testDisplayName
        foreach ($e in $t.entries) {
            if ($e.event.type -eq 'Error') {
                Write-Host "ERROR: " $e.event.message
            }
        }
    }
}
