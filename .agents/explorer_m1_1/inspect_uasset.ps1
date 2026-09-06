param([string]$path)
$bytes = [System.IO.File]::ReadAllBytes($path)
$chars = [System.Text.Encoding]::ASCII.GetString($bytes)
$regex = [System.Text.RegularExpressions.Regex]::Matches($chars, '[A-Za-z0-9_/]{4,}')
$set = @{}
foreach ($m in $regex) {
    $v = $m.Value
    if ($v -match 'Mesh|Anim|Manny|Quinn|SK|FirstPerson|Arm|hand') {
        $set[$v] = $true
    }
}
$set.Keys | Sort-Object
