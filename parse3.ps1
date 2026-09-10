$json = Get-Content 'F:\MEG_Reclamation\Saved\AutomationReports\20260910-211842-4777fd0003b94badb180a433c93cdd98\index.json' | ConvertFrom-Json
foreach ($t in $json.tests) {
    Write-Output "$($t.state): $($t.testDisplayName)"
}
