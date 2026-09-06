param([string]$path)
$bytes = [System.IO.File]::ReadAllBytes($path)
$chars = [System.Text.Encoding]::ASCII.GetString($bytes)
$regex = [System.Text.RegularExpressions.Regex]::Matches($chars, 'RelativeLocation|RelativeRotation|RelativeScale3D')
foreach ($m in $regex) {
    Write-Host $m.Value
}
