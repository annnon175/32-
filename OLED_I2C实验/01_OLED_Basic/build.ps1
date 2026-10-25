$ErrorActionPreference = 'Stop'
$projectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $projectDir 'build'
$gcc = (Get-Command arm-none-eabi-gcc).Source
$objcopy = (Get-Command arm-none-eabi-objcopy).Source
$size = (Get-Command arm-none-eabi-size).Source

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$common = @(
    '-mcpu=cortex-m3', '-mthumb', '-DSTM32F103xB', '-DUSE_HAL_DRIVER',
    '-std=c11', '-Os', '-g3', '-ffunction-sections', '-fdata-sections',
    '-Wall', '-Wextra', '-Werror',
    '-I' + (Join-Path $projectDir 'Core\Inc'),
    '-I' + (Join-Path $projectDir 'Drivers\CMSIS\Core\Include'),
    '-I' + (Join-Path $projectDir 'Drivers\CMSIS\Device\ST\STM32F1xx\Include'),
    '-I' + (Join-Path $projectDir 'Drivers\STM32F1xx_HAL_Driver\Inc')
)

$sources = @(
    'Core\Src\aht20.c',
    'Core\Src\avatar_bitmap.c',
    'Core\Src\board.c',
    'Core\Src\demo.c',
    'Core\Src\font_raw.c',
    'Core\Src\gfx_basic.c',
    'Core\Src\main.c',
    'Core\Src\oled.c',
    'Core\Src\system_stm32f1xx.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_cortex.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_gpio.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_dma.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_flash.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_flash_ex.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_rcc.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_rcc_ex.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_pwr.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_uart.c',
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_i2c.c'
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
    (Join-Path $projectDir 'startup\startup_stm32f103xb.s') '-o' $startupObject
if ($LASTEXITCODE -ne 0) { throw '启动文件编译失败' }
$objects += $startupObject

$elf = Join-Path $buildDir '01_OLED_Basic.elf'
$map = Join-Path $buildDir '01_OLED_Basic.map'
$linker = Join-Path $projectDir 'linker\STM32F103C8T6_FLASH.ld'
& $gcc '-mcpu=cortex-m3' '-mthumb' '-nostartfiles' '--specs=nano.specs' '--specs=nosys.specs' `
    "-T$linker" '-Wl,--gc-sections' "-Wl,-Map=$map" @objects '-o' $elf
if ($LASTEXITCODE -ne 0) { throw '链接失败' }

& $objcopy '-O' 'ihex' $elf (Join-Path $buildDir '01_OLED_Basic.hex')
& $objcopy '-O' 'binary' $elf (Join-Path $buildDir '01_OLED_Basic.bin')
& $size $elf
