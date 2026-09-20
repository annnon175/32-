param(
    [ValidateSet('all', 'three', 'four')]
    [string]$Variant = 'all'
)

$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$buildRoot = Join-Path $projectRoot 'build'

$gcc = (Get-Command arm-none-eabi-gcc -ErrorAction Stop).Source
$objcopy = (Get-Command arm-none-eabi-objcopy -ErrorAction Stop).Source
$size = (Get-Command arm-none-eabi-size -ErrorAction Stop).Source

New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null

$commonFlags = @(
    '-mcpu=cortex-m3',
    '-mthumb',
    '-std=c11',
    '-Os',
    '-ffreestanding',
    '-ffunction-sections',
    '-fdata-sections',
    '-Wall',
    '-Wextra',
    '-Werror'
)

function Invoke-Build([string]$name, [int]$includeOnboard) {
    $variantDir = Join-Path $buildRoot $name
    New-Item -ItemType Directory -Force -Path $variantDir | Out-Null

    $mainObj = Join-Path $variantDir 'main.o'
    $startupObj = Join-Path $variantDir 'startup.o'
    $elf = Join-Path $variantDir ("bluepill-led-{0}.elf" -f $name)
    $hex = Join-Path $variantDir ("bluepill-led-{0}.hex" -f $name)
    $bin = Join-Path $variantDir ("bluepill-led-{0}.bin" -f $name)
    $map = Join-Path $variantDir ("bluepill-led-{0}.map" -f $name)

    & $gcc @commonFlags "-DINCLUDE_ONBOARD_LED=$includeOnboard" `
        '-c' (Join-Path $projectRoot 'src\main.c') '-o' $mainObj
    if ($LASTEXITCODE -ne 0) { throw "main.c compile failed: $name" }

    & $gcc '-mcpu=cortex-m3' '-mthumb' '-x' 'assembler-with-cpp' `
        '-c' (Join-Path $projectRoot 'startup\startup_stm32f103c8t6.S') '-o' $startupObj
    if ($LASTEXITCODE -ne 0) { throw "startup compile failed: $name" }

    & $gcc '-mcpu=cortex-m3' '-mthumb' '-nostartfiles' '--specs=nano.specs' '--specs=nosys.specs' `
        "-T$(Join-Path $projectRoot 'linker\STM32F103C8T6_FLASH.ld')" `
        '-Wl,--gc-sections' "-Wl,-Map=$map" $startupObj $mainObj '-o' $elf
    if ($LASTEXITCODE -ne 0) { throw "link failed: $name" }

    & $objcopy '-O' 'ihex' $elf $hex
    if ($LASTEXITCODE -ne 0) { throw "hex conversion failed: $name" }
    & $objcopy '-O' 'binary' $elf $bin
    if ($LASTEXITCODE -ne 0) { throw "bin conversion failed: $name" }

    Write-Host "`n[$name] build OK"
    & $size $elf
}

if ($Variant -in @('all', 'three')) { Invoke-Build 'three-led' 0 }
if ($Variant -in @('all', 'four'))  { Invoke-Build 'four-led'  1 }

