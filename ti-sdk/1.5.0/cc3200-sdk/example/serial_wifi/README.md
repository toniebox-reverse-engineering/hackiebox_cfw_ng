# Overview

Serial Wi-Fi is a capability designed to provide easy, self-contained
terminal access behavior over UART interface. This provides a
"driver-less" solution and allows out-of-the box operation based on
ASCII character set interpretation. This application runs over the UART
interface and behaves as a command line interpreter. Leveraging the
complete network stack integration, it allows secure, robust end-to-end
communication.    

# Application details

The serial Wi-Fi application supports the following features:
  
1. Support to connect to an AP
2. Auto discovery of available peer using mDNS  
3. All communication uses secure sockets  
4. Once configured to a network, auto connect to the AP on every startup
5. Provision to override default mnemonic strings 
6. Local/Remote Control modes, to issue commands to the
locally/remotely connected Simplelink devices 
  
The Serial Wi-Fi application operates in three modes:  

1. **Terminal/Interpreter mode:** In this case the interpreter mode behaves as pure point-to-point cable
replacement. This is the most common and obvious use in which a serial
cable formerly used to carry information is replaced by SimpleLink
devices on both end of the line.  
2. **Local control mode:** In this mode it is possible to issue commands to the locally connected
device. These commands encompass a close set of options to control the
SimpleLink device while guaranteeing sandbox restrictions. The mode is
entered using a predefined escape sequence `//<` 
3. **Remote control mode:** This allows issuing commands to the remotely connected device. These commands encompass the same set of options to control the remote
SimpleLink as the local device, while guaranteeing sandbox restrictions.
The mode is entered using a predefined escape sequence `//>`

**Warning:** Current time must be set in the device. This time is used
to validate the certificate. If the date is beyond the validity period
of Certificate, `sl_connect` will return error. Update the below
macros in **serial\_wifi.h**.

```c
    #define DATE                01    /* Current Date */
    #define MONTH               1     /* Month 1-12 */
    #define YEAR                2019  /* Current year */
    #define HOUR                12    /* Time - hours */
    #define MINUTE              00    /* Time - minutes */
    #define SECOND              00     /* Time - seconds */
```

## Source Files briefly explained

- **main.c** - Creates to two tasks. Task 1: the interpreter task to operate the serial wifi functionality. Task 2: The UART task to receive data from the UART.  
- **serial\_wifi.c** - ALl the functionality of the serial wifi is implemented.  
- **uart\_config.c** - Configures the UART and the task to send and receive chars over the UART.
- **conversions.c** - All conversion utility API into different number formats.

# Usage

1. Modify the date macros in serial_wifi.h as shown above. Rebuild the application and flash the binary to **two** LaunchPads using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
2. Upon Reset, the device connects to the stored AP by the AUTO connect
policy.If the device doesn't connect to an AP in 6secs, the application
prompts the user to connect to a known AP using the Local control mode.
User can use the below command to connect to an
AP  in the local control mode:<br>
	`wlan_connect [ssid] [type] [Sec] [User] [key]`
3. If this is the first boot after an Serial FLASH erase, one of the
devices need to be configured for as mDNS server. This can be done by
using the below command in local control mode.<br>
	`mDNS  0  <0-server, 1-client>`
4. The client device automatically connects to the advertiser.
5. The communication is based on secure sockets. So, the server key,
server certificate, client certificate need to be flashed in the SFLASH
at file IDs 129,130,131 respectively. Please refer the Generating SSL
certificates section for generating certificates using OpenSSL.  
For flashing the certificates using UniFlash, refer to the [Uniflash documentation](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide). 

1. Click **Add File**
2. Rename the newly created file to "/cert/129.der"  
3. In the Url field mention the local path to the Server key certificate.  
4. Repeat the above steps for "/cert/130.der" and "/cert/131.der" for server key and client certificate respectively.  
5. Check the Erase and Update check boxes and **Program**
6. Now the devices are connected in Interpreter mode.  
7. `\\<` command is used to enter the local control mode, mode in which we control the local CC3200 device.  
8. `\\>` command is used to enter the remote control mode, mode to control the remote CC3200 device.  

Commands supported in the control modes are:
	
| `help` | Show menu |
|-----------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `wlan_connect [ssid] [type] [Sec] [User] [key]` | Connect to WLAN AP <br>ssid:Name of the AP <br>type: 0-Personal,1-ENT <br>Sec: 0-OPEN,1-WEP,2-WPA <br>User: user name in case of ENT <br>Key: password in case of WEP and WPA |
| `connect [IP / name] [port] [TCP / UDP]` | Connect to specified IP or named server with an optionally specified port. If not specified, default port is used.<br> Can optionally set transport protocol explicitly. If IP/name not specified, default is UDP. Otherwise, default is TCP. |
| `disconnect` | Disconnect connected end point. |
| `\\<` | Enter local control mode. |
| `\\>` | Enter remote control mode. |
| `exit` | Terminate control mode, and resume interpreter mode operation. |
| `quit` | Terminate interpreter mode, and free up all related resources. |
| `mode [A / B / H]` | Set data interpretation mode <br>A - ASCII <br>B - Binary <br>H - Hex |
| `wrap [B / L]` | Set terminal wrap mode level <br>B - Byte <br>L - Line |
| `send [dest] [data]` | Send provided data to dest. If TCP is used, dest is ignored. |
| `recv [dest] [length] [timeout]` | Receive data from dest. <br>If TCP is used, dest is ignored. If length not specified, 1 is used. Command will block until length bytes received, the optional timeout `us` is reached, or `^C` is received. |
| `ping [IP / name] [number]` | Ping the provided dest for an optionally specified number of times. If not specified, ping will run until `^C` is received. |
| `mDNS [option]` | mDNS configuration <br>0-Advertise <br>1- Listen |
	
All communication is secure using secure sockets. Security method SSLV3 and Cipher `RSA WITH RC4 128 SHA` 

## Generating certificates using OpenSSL

The following are the steps for generating certificates using OpenSSL
tool.

1. Generate CA file "ca.pem" and CA key "privkey.pem"
	- `openssl req -out ca.pem -new -x509`
2. Generate server certificate/key pair
	- `openssl genrsa -out server.key 1024`
	- `openssl req -key server.key -new -out server.req`
	- `openssl x509 -req -in server.req -CA CA.pem -CAkey privkey.pem -CAserial file.srl -out server.pem`
		- Contents of "file.srl" is a two digit number. eg. "00"
3. Convert the certificates and key generated to DER format.  
	- For certificates: `openssl x509 -in <input.crt> -inform PEM –out <output.crt> -outform DER`
	- For key: `rsa -in input.key -inform PEM -out output.key -outform DER`
4. Flash the server key and the server certificate at /cert/129.der and /cert/130.der respectively.  
5. Flash the CA certificate at /cert/131.der

## Limitations/Known Issues
