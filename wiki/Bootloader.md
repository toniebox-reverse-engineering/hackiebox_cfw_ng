# HackieboxNG SD bootloader

The HackieboxNG SD bootloader consists of two bootloaders (called stages). Both stages share the same codebase and are relocated to 0x20038000 before run.

## Preloader (Stage 1)
![Start process preloader](https://raw.githubusercontent.com/toniebox-reverse-engineering/hackiebox_cfw_ng/master/wiki/graphs/HBNG-SDPreloader.png)

The preloader runs a fixed file from the sd card (sd:/revvox/boot/ngbootloader.bin) without any checks. An update for shouldn't be necesarry in the future.
It should be installed as primary bootloader for HackieboxNG to flash:/sys/mcuimg.bin.

***All error codes for the preloader are in blue.***

## Bootloader (Stage 2)
![Start process bootloader](https://raw.githubusercontent.com/toniebox-reverse-engineering/hackiebox_cfw_ng/master/wiki/graphs/HBNG-SDBootloader.png)

When no ear is pressed, the bootloader loads the selected standard bootslot. If you hold the big ear while booting you may select a different slot by pressing the small ear for a short moment. Only slots with a file on the sd card can be selected. You may use them in a different way and change the settings within the configuration. The selected slot is indicated by 1-3 blinks in a color assigned to each group. Following slots are available:

Filepath: sd:/revvox/boot/ng-*XXXY*.bin

### Green group - Original firmware
1) **ofw1** - OFW bootloader from flash:/sys/pre-img.bin  recommended here
2) **ofw2** - simulate OFW behaviour and load the same image like the OFW would, but with patches
3) **ofw3** - specific OFW file with patches (optional)

### Blue group - Custom firmware
1) **cfw1** - Primary firmware (optional)
2) **cfw2** - Backup firmware (optional)
3) **cfw3** - (optional)

### Cyan group - Additonal firmwares
1) **add1** - (optional)
2) **add2** - (optional)
3) **add3** - (optional)


***All error codes for the bootloader are in green.***

### Configuration
The configuration for the bootloader is saved within [sd:/revvox/boot/ngCfg.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/ngCfg.json). All sections or keys starting with and underscore "_" a comments and will be ignored.
#### General Section
| Key | Description | Values | Default |
| - | - | - | - | 
| activeImg | Sets the firmware slot to select at startup | ofw1, ofw2, ofw3, cfw1, cfw2, cfw3, add1, add2, add3 | ofw1 |
| waitForPress | Waits for an earpress on startup with a blink sequence (blue, green, cyan, black)| true, false | false |
| waitForBoot | Waits for an earpress on before firmware boot with a blink sequence (blue, green, cyan, black)| true, false | false |
| waitTimeoutInS | Timeout in seconds for waitForPress if no earpress (hibernation) | 1-255 | 60 |
| minBatteryLevel | Poweroff voltage to protect the battery. Divide through around 700 to get voltage (Standard 3V) | | 2100 |
| ofwFixValue | Magic bytes to be placed into the OFW Image during boot (can be extracted from OFW BL data[-8:-4]) | hex array with 4 bytes | ["4C", "01", "10", "00"] |
| ofwFixFlash| Magic bytes read from the ofw bootloader on flash | ex. /sys/pre-img.bin| |
| serialLog | Enable log to UART (TX) @921600 baud. Only works for debug build! | true, false | true |
| logLevel | Set Log level 0:Trace - 5:Fatal | 0-5 | DEBUG_LOG_LEVEL |
| logColor | Enable colored log | true, false | false |

#### Firmware Section
There are nine firmware slots, named ofw1, ofw2, ofw3, cfw1, cfw2, cfw3, add1, add2 and add3.

| Key | Description | Values | Default |
| - | - | - | - | 
| checkHash | Check hash of firmware | true, false | true |
| hashFile | Chech hash from ng-XXX?.sha file (true) or from the last 64 byte of the firmware itself (ofw) | true, false | false |
| watchdog | Keep watchdog enabled when booting firmware (if booting fails, box will restart) | true, false | false |
| ofwFix | Add magic bytes to the firmware image to make ofw directly boot | true, false | false
| ofwSimBL | Read image to boot from flash:/sys/mcubootinfo.bin and load the image from flash:/sys/mcuimgN.bin instead of reading from sd (like the ofw bootloader)| true, false | false
| bootFlashImg | Read firmware from file on flash | true, false | false |
| flashImg | Path to the file on flash | ex. /sys/pre-img.bin | |
| patches | List of patches to load, see [patch directory](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/tree/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch) or [patch wiki](OFWPatches)  | ["noCerts.305", "noPass3.305"] | [] |



### SHA256 check
For each slot an SHA256 check is available. Either as a seperated ng-*XXXY*.sha file or directly appended to the binary

For example all OFW binaries have a SHA256 appended to their file ending (except the ofw bootloader). A fitting ng-ofw1.sha is provided for the ofw bootloader.
The older Hackiebox CFW doesn't have a SHA256 appened. So you may need to create ng-*cfwX*.sha yourself if you want to use it. For HackieboxNG the SHA256 hash will be directly appended to the firmware file itself.

#### Windows
Command: `Get-Filehash FILENAME.BIN -Algorithm SHA256`
#### Linux
Command: `shasum256 FILENAME.BIN`

### Patches
The integrated patch engine allows to apply patches to the loaded firmware in-memory. Currently just a simple dup2 patcher style *Search & Replace* engine ist implemented. You may patch up to 256 bytes per patch and apply up to 32 patches per slot. The patchname is limited to 32 characters.
[More about available ofw patches](OFWPatches)

### Error codes
If the bootloader detects a problem, it blinks in a defined pattern. The preloader on the flash blinks blue, the bootloader on the sd blinks green.
#### SD related codes
If a sd related problem occurs, the box combines two patterns. The first one indicates where the problem roughly occured. The second one gives you more information about it.
##### First pattern
###### SD not found - 2x500ms, wait 500ms
Please check if the sd is placed in the holder correctly and the sd is okay. The OFW will blink in red and shut off.
###### File could not be opened - 3x500ms, wait 2000ms
Problem opening the firmware file
###### File could not be read - 4x500ms, wait 2000ms
Problem reading the firmware file
##### Second pattern (X times 1000ms)
1. FR_DISK_ERR, /* (1) A hard error occurred in the low level disk I/O layer */
2. FR_INT_ERR, /* (2) Assertion failed */
3. FR_NOT_READY, /* (3) The physical drive cannot work */
4. FR_NO_FILE, /* (4) Could not find the file */
5. FR_NO_PATH, /* (5) Could not find the path */
6. FR_INVALID_NAME, /* (6) The path name format is invalid */
7. FR_DENIED, /* (7) Access denied due to prohibited access or directory full */
8. FR_EXIST, /* (8) Access denied due to prohibited access */
9. FR_INVALID_OBJECT, /* (9) The file/directory object is invalid */
10. FR_WRITE_PROTECTED, /* (10) The physical drive is write protected */
11. FR_INVALID_DRIVE, /* (11) The logical drive number is invalid */
12. FR_NOT_ENABLED, /* (12) The volume has no work area */
13. FR_NO_FILESYSTEM, /* (13) There is no valid FAT volume */
14. FR_MKFS_ABORTED, /* (14) The f_mkfs() aborted due to any problem */
15. FR_TIMEOUT, /* (15) Could not get a grant to access the volume within defined period */
16. FR_LOCKED, /* (16) The operation is rejected according to the file sharing policy */
17. FR_NOT_ENOUGH_CORE, /* (17) LFN working buffer could not be allocated */
18. FR_TOO_MANY_OPEN_FILES, /* (18) Number of open files > _FS_LOCK */
19. FR_INVALID_PARAMETER /* (19) Given parameter is invalid */
#### Other
##### Battery Low - 2x66ms, 2x133ms, 2x66ms
Battery is low. Value is under the minimum defined in minBatteryLevel. Box hibernates
##### Hash differs - 10x50ms
The actual hash of the firmware is different from the one defined in the firmware itself or in the hashfile (depens on the config). Checking the UART-output may help.
##### Watchdog reset - 5x33ms, 5x66ms, 5x33ms
The watchdog reseted the box, because the box was in an unintended state or the firmware is broken.
##### Application error - 3x33ms, 3x66ms, 3x33ms
Application error. This shouldn't happen. 