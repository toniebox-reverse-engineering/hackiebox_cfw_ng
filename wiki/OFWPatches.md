# HackieboxNG original firmware patches
## Offline / Privacy
It is recommended to keep the toniebox offline if you are using a patched firmware. Using the offline mode is recommended. In addition you should enable the Null certificate patch.

### Null certificate Paths ([noCerts.305.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/noCerts.305.json))
This patch clears the paths to the certificates. This way the box will abort the https connection as it can't check if the boxine server is a valid one. 

## Alternative Tags (SLIX / SLIX2)
If you want to use alternative tags those patches will help you. Even other iso15693 tags may work.

### Block count ([blockCheck.308.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/blockCheck.308.json))
Usally the toniebox checks if the tag has exactly 8 blocks. The check allows the tag to have more than that. (ex. SLIX or SLIX2)

### No privacy password ([noPass.305.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/noPass.305.json))
Usally doesn't allow tags without the boxine specific or the NXP specific privacy mode password. With the patch you can use tags without having privacy password support (ex. SLIX).

### No UID check ([uidCheck.305.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/uidCheck.305.json))
Usally the toniebox checks if the UID of the tag starts with *E0:04:03*. With that patch you may use tags with other UIDs (ex SLIX or SLIX2)


## Additional
### Disable charger wakeup ([noChargWake.305.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/noChargWake.305.json))
The toniebox won't wakeup anymore if its put onto the charger. ***Attention, this patch is only working if you disconnect the battery for a second before loading the patched ofw. If you start the unpatched ofw once, you will have to disconnect the battery again***

### Disable privacy mode ([noPrivacy.305.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/noPrivacy.305.json))
Usally the toniebox puts every tag into privacy mode after reading it. This patch disables that, so you can easily read the UID with any standard iso15693 reader like your phone.


##  Development
### Enable SWD ([swd.json](https://github.com/toniebox-reverse-engineering/hackiebox_cfw_ng/blob/master/sd-bootloader-ng/bootmanager/sd/revvox/boot/patch/swd.json))
Handy patch to debug the OFW and to verify and test patches.
***Attenion, the red led won't work with that patch enabled!***