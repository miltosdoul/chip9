g++ -Wall -g main.cpp chip9/chip9.cpp -o chip9 `
    -lpthread -lmingw32 -lSDL2main `
    -lSDL2    -mwindows -mconsole

if ( !$LASTEXITCODE ) {
    ./chip9
} else {
    Write-Host "ERROR: g++ returned non-zero exit code."
}