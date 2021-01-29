# HackieboxNG Installation

## Backup
Please make a **full file based + flash backup** of your toniebox's flash with [cc3200tool](https://github.com/toniebox-reverse-engineering/cc3200tool).

```
python3 cc.py -p /dev/ttyUSB0 read_all_files targetdir/ read_flash backup.bin
```

## HackieboxNG Bootloader

### Get the bootloader
Download the latest version from [here](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/releases).

### Preloader (Stage 1)
#### Move original bootloader
First of all you need to copy you just backuped original mcuimg.bin from your toniebox to a different location where the preloader can run it as fallback.
```
python3 cc.py -p /dev/ttyUSB0 read_file /sys/mcuimg.bin pre-img.bin
python3 cc.py -p /dev/ttyUSB0 write_file pre-img.bin /sys/pre-img.bin
```
#### Install bootloader
```
python3 cc.py -p /dev/ttyUSB0 write_file ngpreloader.bin /sys/mcuimg.bin
```
#### Or as oneliner
```
python3 cc.py -p /dev/ttyUSB0 read_file /sys/mcuimg.bin mcuimg.bin write_file mcuimg.bin /sys/pre-img.bin write_file ngpreloader.bin /sys/mcuimg.bin
```

### Bootloader (Stage 2)
#### SD files
Copy over all the contents of the *sd* folder the sd card of the toniebox.
Copy over the original bootloader (mcuimg.bin from your backup) to the first ofw slot *sd:/revvox/boot/ng-ofw1.bin*.

Note: The preloader will load any file placed to *sd:/revvox/boot/ngbootloader.bin*. 

[More about the different firmware slots here.](Bootloader.md)