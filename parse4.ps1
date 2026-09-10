$json = Get-Content 'F:\MEG_Reclamation\Saved\AutomationReports\20260910-212559-9f7df7d587634014b6c0904cb3dbc5cc\index.json' | ConvertFrom-Json
foreach ($t in $json.tests) {
    Write-Output "$($t.state): $($t.testDisplayName)"
}
