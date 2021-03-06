#ifndef __GLOBALDEFINES_H__
#define __GLOBALDEFINES_H__

//#define FIXED_BOOT_IMAGE
//#define DISABLE_WATCHDOG


#define WATCHDOG_CHECK_S 5
#define WATCHDOG_TIMEOUT_S 15
#define WATCHDOG_TIMEOUT_SLOW_S 15

#define WATCHDOG_UtilsDelayMS_MAX 25

#define IMG_OFW_ID_1 0
#define IMG_OFW_ID_2 1
#define IMG_OFW_ID_3 2
#define IMG_CFW_ID_1 3
#define IMG_CFW_ID_2 4
#define IMG_CFW_ID_3 5
#define IMG_ADD_ID_1 6
#define IMG_ADD_ID_2 7
#define IMG_ADD_ID_3 8

#define IMG_MAX_COUNT 9

#define SD_PATH_BASE "/revvox/boot/"
#define SD_PATH_BASE_LEN 13
#define IMG_SD_NAME "ng-CCCN.bin"

#define FALLBACK_IMG_FLASH_PATH "/sys/pre-img.bin"
#define FALLBACK_IMG_FLASH_ENABLED
#define IMG_SD_PATH SD_PATH_BASE IMG_SD_NAME
#define IMG_SD_NAME_REPL1_POS 3
#define IMG_SD_NAME_REPL2_POS 6
#define IMG_SD_PATH_REPL1_POS SD_PATH_BASE_LEN + IMG_SD_NAME_REPL1_POS
#define IMG_SD_PATH_REPL2_POS SD_PATH_BASE_LEN + IMG_SD_NAME_REPL2_POS
#define IMG_OFW_NAME "ofw"
#define IMG_CFW_NAME "cfw"
#define IMG_ADD_NAME "add"
#define CFG_SD_PATH SD_PATH_BASE "ngCfg.json"
#define PATCH_SD_BASE_PATH SD_PATH_BASE "patch/"

#define IMG_SD_BOOTLOADER_NAME "ngbootloader.bin"
#define IMG_SD_BOOTLOADER_PATH SD_PATH_BASE IMG_SD_BOOTLOADER_NAME

#define HASH_SD_NAME "ng-CCCN.sha"
#define HASH_SD_PATH SD_PATH_BASE HASH_SD_NAME

#define COLOR_BLACK 0
#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_RED 4
#define COLOR_CYAN 3

#define HAL_FCPU_MHZ 80U
#define HAL_FCPU_HZ (1000000U * HAL_FCPU_MHZ)
#define HAL_SYSTICK_PERIOD_US 1000U
#define UTILS_DELAY_US_TO_COUNT(us) (((us)*HAL_FCPU_MHZ) / 6)
#define MILLISECONDS_TO_TICKS(ms)    ((HAL_FCPU_HZ / 1000) * (ms))

#define APP_IMG_SRAM_OFFSET 0x20004000
#define CFG_SRAM_OFFSET 0x20000000

#define EAR_BIG_PRCM PRCM_GPIOA0
#define EAR_SMALL_PRCM PRCM_GPIOA0

#define LED_GREEN_PORT GPIOA3_BASE
#define LED_BLUE_PORT GPIOA3_BASE
#define EAR_BIG_PORT GPIOA0_BASE
#define EAR_SMALL_PORT GPIOA0_BASE
#define POWER_SD_PORT GPIOA0_BASE
#define POWER_PORT GPIOA0_BASE
#define CHARGER_PORT GPIOA2_BASE

//#define LED_GREEN_GPIO pin_GP25
#define LED_GREEN_PIN_NUM PIN_21 // GP25/SOP2
#define LED_BLUE_PIN_NUM PIN_17 // GP24
#define EAR_BIG_PIN_NUM PIN_57   // GP02
#define EAR_SMALL_PIN_NUM PIN_59 // GP04
#define POWER_SD_PIN_NUM PIN_58 // GP03
#define POWER_PIN_NUM PIN_61 // GP06
#define CHARGER_PIN_NUM PIN_08 // GP17
#define BATTERY_LEVEL_PIN_NUM PIN_60 // GP05

#define LED_GREEN_PORT_MASK GPIO_PIN_1
#define LED_BLUE_PORT_MASK GPIO_PIN_0
#define EAR_BIG_PORT_MASK GPIO_PIN_2
#define EAR_SMALL_PORT_MASK GPIO_PIN_4
#define POWER_SD_PORT_MASK GPIO_PIN_3
#define POWER_PORT_MASK GPIO_PIN_6
#define CHARGER_PORT_MASK GPIO_PIN_1

#define PATCH_MAX_BYTES 255
#define PATCH_MAX_PER_IMAGE 32
#define PATCH_MAX_NAME_LENGTH 32
#define PATCH_MAX_POSITIONS 10

//#define NO_DEBUG_LOG 

#define DEBUG_LOG_LEVEL_TRACE 0 
#define DEBUG_LOG_LEVEL_DEBUG 1
#define DEBUG_LOG_LEVEL_INFO 2
#define DEBUG_LOG_LEVEL_WARN 3
#define DEBUG_LOG_LEVEL_ERROR 4
#define DEBUG_LOG_LEVEL_FATAL 5

#define DEBUG_LOG_LEVEL DEBUG_LOG_LEVEL_TRACE
#define DEBUG_LOG_COLORED
#define DEBUG_LOG_BAUD 115200

#endif