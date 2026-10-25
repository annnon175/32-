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
    '-Wall', '-Wextra', '-Werror', '-DUSE_U8G2', '-Wno-unused-parameter',
    '-I' + (Join-Path $projectDir 'Core\Inc'),
    '-I' + (Join-Path $projectDir 'Middlewares\U8g2'),
    '-I' + (Join-Path $projectDir 'Drivers\CMSIS\Core\Include'),
    '-I' + (Join-Path $projectDir 'Drivers\CMSIS\Device\ST\STM32F1xx\Include'),
    '-I' + (Join-Path $projectDir 'Drivers\STM32F1xx_HAL_Driver\Inc')
)

$sources = @(
    'Core\Src\aht20.c',
    'Core\Src\avatar_bitmap.c',
    'Core\Src\board.c',
    'Core\Src\demo.c',
    'Core\Src\font_lab12.c',
    'Core\Src\gfx_u8g2.c',
    'Core\Src\main.c',
    'Core\Src\oled.c',
    'Core\Src\system_stm32f1xx.c',
    'Core\Src\u8g2_port.c',
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
    'Drivers\STM32F1xx_HAL_Driver\Src\stm32f1xx_hal_i2c.c',
    'Middlewares\U8g2\u8g2_bitmap.c',
    'Middlewares\U8g2\u8g2_box.c',
    'Middlewares\U8g2\u8g2_buffer.c',
    'Middlewares\U8g2\u8g2_circle.c',
    'Middlewares\U8g2\u8g2_cleardisplay.c',
    'Middlewares\U8g2\u8g2_d_memory.c',
    'Middlewares\U8g2\u8g2_d_setup.c',
    'Middlewares\U8g2\u8g2_font.c',
    'Middlewares\U8g2\u8g2_hvline.c',
    'Middlewares\U8g2\u8g2_intersection.c',
    'Middlewares\U8g2\u8g2_kerning.c',
    'Middlewares\U8g2\u8g2_line.c',
    'Middlewares\U8g2\u8g2_ll_hvline.c',
    'Middlewares\U8g2\u8g2_setup.c',
    'Middlewares\U8g2\u8x8_8x8.c',
    'Middlewares\U8g2\u8x8_byte.c',
    'Middlewares\U8g2\u8x8_cad.c',
    'Middlewares\U8g2\u8x8_capture.c',
    'Middlewares\U8g2\u8x8_d_ssd1306_128x64_noname.c',
    'Middlewares\U8g2\u8x8_display.c',
    'Middlewares\U8g2\u8x8_gpio.c',
    'Middlewares\U8g2\u8x8_setup.c',
    'Middlewares\U8g2\u8x8_string.c',
    'Middlewares\U8g2\u8x8_u16toa.c',
    'Middlewares\U8g2\u8x8_u8toa.c'
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

$elf = Join-Path $buildDir '02_U8g2_Demo.elf'
$map = Join-Path $buildDir '02_U8g2_Demo.map'
$linker = Join-Path $projectDir 'linker\STM32F103C8T6_FLASH.ld'
& $gcc '-mcpu=cortex-m3' '-mthumb' '-nostartfiles' '--specs=nano.specs' '--specs=nosys.specs' `
    "-T$linker" '-Wl,--gc-sections' "-Wl,-Map=$map" @objects '-o' $elf
if ($LASTEXITCODE -ne 0) { throw '链接失败' }

& $objcopy '-O' 'ihex' $elf (Join-Path $buildDir '02_U8g2_Demo.hex')
& $objcopy '-O' 'binary' $elf (Join-Path $buildDir '02_U8g2_Demo.bin')
& $size $elf
