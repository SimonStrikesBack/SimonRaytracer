foreach ($file in Get-ChildItem -Recurse -File -Exclude "convert.ps1") {  
    $name = $file.DirectoryName + "\" +$file.BaseName
    $old_name = $name + ".ppm"
    $new_name = $name + ".png"
    Write-Output $old_name $new_name
    magick.exe $old_name $new_name
}