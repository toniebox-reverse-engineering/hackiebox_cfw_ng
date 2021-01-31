# HackieboxNG Installation

## Backup
Please make a **full file based + flash backup** of your toniebox's flash with [cc3200tool](https://github.com/toniebox-reverse-engineering/cc3200tool).

```
python3 cc.py -p /dev/ttyUSB0 read_all_files targetdir/ read_flash backup.bin
```

## HackieboxNG Bootloader

### Get the bootloader
Download the latest version from [here](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/releases).

### Preface
If you have previously installed the CFW SD Bootloader please use *1b)* otherwise for first time installation please use *1a)*.

### 1a) Preloader (Stage 1) - For first time installation

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

### 1b) Preloader (Stage 1) - For updating the sd bootloader
You may have the (old) Hackiebox CFW to upload the preloader. This way you can install it over the air without direct access to the flash if you have already installed a previous version of the cfw bootloader/preloader.

#### Backup
To verify that you have currently installed the ofw bootloader to */sys/pre-img.bin* you may download the file from flash
```
http://*.*.*.*/api/ajax?cmd=get-flash-file&filepath=/sys/pre-img.bin
```
Its sha256 hash should be *e86749c2e47f6c87eb462516e56a0a9ad215f38beb312cc77785c54921c97c05*.

#### Copy over preloader
![CFW Flash upload](https://raw.githubusercontent.com/toniebox-reverse-engineering/hackiebox_cfw_ng/master/wiki/images/InstallCfwFlashUpload.png)

1) Select *Target* "Flash"
2) Select *Local file* "ngpreloader.bin"
3) Set *Overwrite* to "on"
4) Set *SD/Flash path*  to "/sys/mcuimg.bin"
5) Press *Upload*!

#### Verification
To verify that the upload was successful you can download the mcuimg.bin from flash afterwards and compare it with the ng-preloader.bin you have just uploaded.
```
http://*.*.*.*/api/ajax?cmd=get-flash-file&filepath=/sys/mcuimg.bin
```

### 2) Bootloader (Stage 2)
#### SD files
Copy over all the contents of the *sd* folder the sd card of the toniebox.
Copy over the original bootloader (mcuimg.bin from your backup) to the first ofw slot *sd:/revvox/boot/ng-ofw1.bin*.

Note: The preloader will load any file placed to *sd:/revvox/boot/ngbootloader.bin*. 

[More about the different firmware slots here.](Bootloader.md)