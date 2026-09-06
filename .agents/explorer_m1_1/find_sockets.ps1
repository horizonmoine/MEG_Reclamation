param([string]$path)
$bytes = [System.IO.File]::ReadAllBytes($path)
$chars = [System.Text.Encoding]::ASCII.GetString($bytes)
$regex = [System.Text.RegularExpressions.Regex]::Matches($chars, '[A-Za-z0-9_]{3,}(?:[Ss]ocket|[Hh]and|[Gg]un|[Aa]ttach)[A-Za-z0-9_]*')
$set = @{}
foreach ($m in $regex) {
    $set[$m.Value] = $true
}
$set.Keys | Sort-Object
