$ErrorActionPreference = 'Stop'
$projectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $projectDir 'build'
$gcc = (Get-Command arm-none-eabi-gcc).Source
$objcopy = (Get-Command arm-none-eabi-objcopy).Source
$size = (Get-Command arm-none-eabi-size).Source

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$common = @(
    '-mcpu=cortex-m3', '-mthumb', '-DSTM32F10X_MD', '-DUSE_STDPERIPH_DRIVER',
    '-std=c11', '-O0', '-g3', '-ffunction-sections', '-fdata-sections',
    '-Wall', '-Wextra', '-Werror',
    '-I' + (Join-Path $projectDir 'User'),
    '-I' + (Join-Path $projectDir 'Libraries\CMSIS\Core\Include'),
    '-I' + (Join-Path $projectDir 'Libraries\CMSIS\Device\Include'),
    '-I' + (Join-Path $projectDir 'Libraries\STM32F10x_StdPeriph_Driver\inc')
)

$sources = @(
    'User\main.c',
    'Libraries\STM32F10x_StdPeriph_Driver\src\stm32f10x_gpio.c',
    'Libraries\STM32F10x_StdPeriph_Driver\src\stm32f10x_rcc.c'
)

$objects = @()
foreach ($source in $sources) {
    $name = [IO.Path]::GetFileNameWithoutExtension($source) + '.o'
    $object = Join-Path $buildDir $name
    & $gcc @common '-c' (Join-Path $projectDir $source) '-o' $object
    if ($LASTEXITCODE -ne 0) { throw "编译失败：$source" }
    $objects += $object
}

$startupObject = Join-Path $buildDir 'startup.o'
& $gcc '-mcpu=cortex-m3' '-mthumb' '-x' 'assembler-with-cpp' '-c' `
    (Join-Path $projectDir 'startup\startup_stm32f103c8t6.S') '-o' $startupObject
if ($LASTEXITCODE -ne 0) { throw '启动文件编译失败' }
$objects += $startupObject

$elf = Join-Path $buildDir 'exp2_spl.elf'
$map = Join-Path $buildDir 'exp2_spl.map'
$linker = Join-Path $projectDir 'linker\STM32F103C8T6_FLASH.ld'
& $gcc '-mcpu=cortex-m3' '-mthumb' '-nostartfiles' '--specs=nano.specs' '--specs=nosys.specs' `
    "-T$linker" '-Wl,--gc-sections' "-Wl,-Map=$map" @objects '-o' $elf
if ($LASTEXITCODE -ne 0) { throw '链接失败' }

& $objcopy '-O' 'ihex' $elf (Join-Path $buildDir 'exp2_spl.hex')
& $objcopy '-O' 'binary' $elf (Join-Path $buildDir 'exp2_spl.bin')
& $size $elf

