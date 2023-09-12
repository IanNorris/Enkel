# Get the drive based on its drive letter
$drive = Get-WmiObject Win32_LogicalDisk -Filter "DeviceID='D:'"

# Eject the drive
if ($drive -ne $null) {
    $driveEject = $drive.DeviceID
    (New-Object -ComObject Shell.Application).Namespace(17).ParseName($driveEject).InvokeVerb("Eject")
    Write-Host "USB Drive D: has been ejected."
} else {
    Write-Host "USB Drive D: not found."
}
