$clang = Get-ChildItem "$env:USERPROFILE\.vscode\extensions\ms-vscode.cpptools-*" `
    -Recurse -Filter clang-format.exe |
    Select-Object -First 1 -ExpandProperty FullName

if (!$clang) {
    Write-Error "Formatter not found"
    exit
}

Get-ChildItem . -Recurse -File -Include *.cpp,*.h,*.hpp |
    Where-Object { $_.FullName -notmatch '\\(\.pio|\.git)\\' } |
    ForEach-Object {
        Write-Host "Formatting $($_.FullName)"
        & $clang -i $_.FullName
    }