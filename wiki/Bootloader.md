# HackieboxNG SD bootloader

The HackieboxNG SD bootloader consists of two bootloaders (called stages). Both stages share the same codebase and are relocated to 0x20038000 before run.

## Preloader (Stage 1)
![Start process preloader](https://raw.githubusercontent.com/toniebox-reverse-engineering/hackiebox_cfw_ng/master/wiki/graphs/HBNGSD-Preloader.png)
The preloader runs a fixed file from the sd card (sd:/revvox/boot/ngbootloader.bin) without any checks. An update for shouldn't be necesarry in the future.
It should be installed as primary bootloader for HackieboxNG to flash:/sys/mcuimg.bin. As fallback another bootloader should be placed to flash:/sys/pre-img.bin.

***All error codes for the preloader are in blue.***

## Bootloader (Stage 2)
![Start process bootloader](https://raw.githubusercontent.com/toniebox-reverse-engineering/hackiebox_cfw_ng/master/wiki/graphs/HBNGSD-Bootloader.png)
When no ear is pressed, the bootloader loads the selected standard bootslot. If you hold the big ear while booting you may select a different slot by pressing the small ear for a short moment. Only slots with a file on the sd card can be selected. You may use them in a different way and change the settings within the configuration. The selected slot is indicated by 1-3 blinks in a color assigned to each group. Following slots are available:

Filepath: sd:/revvox/boot/ng-*XXXY*.bin

### Green group - Original firmware
1) **ofw1** - OFW bootloader recommended here
2) **ofw2** - (optional)
3) **ofw3** - specific OFW file with patches (optional)

### Blue group - Custom firmware
1) **cfw1** - Primary firmware (optional)
2) **cfw2** - Backup firmware (optional)
3) **cfw3** - (optional)

### Cyan group - Additonal firmwares
1) **add1** - (optional)
2) **add2** - (optional)
3) **add3** - (optional)


***All error codes for the bootloader are in green.****

### Configuration

### SHA256 check
For each slot an SHA256 check is available. Either as a seperated ng-*XXXY*.sha file or directly appended to the binary

For example all OFW binaries have a SHA256 appended to their file ending (except the ofw bootloader). A fitting ng-ofw1.sha is provided for the ofw bootloader.
The older Hackiebox CFW doesn't have a SHA256 appened. So you may need to create ng-*cfwX*.sha yourself if you want to use it. For HackieboxNG the SHA256 hash will be directly appended to the firmware file itself.

### Patches

## Error codes
