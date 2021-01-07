## Overview

SSL is the universally accepted means by which communication is
authenticated and encrypted on the World Wide Web. SSL certificates are
designed to provide two principles, privacy and authentication. Privacy
is achieved by encryption/decryption and authentication is achieved by
signature/verification. This wiki will introduce a user to SSL/TLS and
its implementation on the CC3200 devices.

## Application details

### Protocol and Ciphers

The SSL Protocol supports the use of a variety of different
encryption/decryption algorithms - also known as ciphers - for use in
operations such as authenticating the connection between a server and
client, transmitting certificates, and establishing session keys.
Depending on the version of SSL supported, clients and servers may
support different sets of ciphers. The following methods and ciphers are
supported by CC3200.

| Method | Cipher                               |
| ------ | ------------------------------------ |
| SSLv3  | RSA\_WITH\_RC4\_128\_SHA             |
| SSLv3  | RSA\_WITH\_RC4\_128\_MD5             |
| TLSv1  | RSA\_WITH\_RC4\_128\_SHA             |
| TLSv1  | RSA\_WITH\_RC4\_128\_MD5             |
| TLSv1  | RSA\_WITH\_AES\_256\_CBC\_SHA        |
| TLSv1  | DHE\_RSA\_WITH\_AES\_256\_CBC\_SHA   |
| TLSv1  | ECDHE\_RSA\_WITH\_AES\_256\_CBC\_SHA |
| TLSv1  | ECDHE\_RSA\_WITH\_RC4\_128\_SHA      |

### API for SSL

The CC3200 has extended the BSD Socket API in order to support the SSL
layer. At the application level, the basic socket flow when using a
secured socket is kept the same; operations such as `connect()`,
`accept()`, `send()`, `recv()` or `select()` are supported.

When a client application 'connects' to a secure socket, the function
will only return successfully if a secure session was established with
the server successfully. An error is returned if the secure session is
not established. After the connection is established, the data path is
secure.

When a server application 'accepts' a connection over a secure opened
socket, the function will only return successfully if a secure session
was established with the client successfully. If the client is rejected
by the secure session, the CC3200 will automatically prepare itself to
accept a new connection from another client without application
interference. In other words, if a secure client connection is not
successful, there is no need to recall the `accept()` function via the
user application. After the connection is established, the data path is
secure.

If the remote side decides to downgrade the connection to an unsecured
socket, `recv()` will return with the `ESECCLOSED` error. How the socket
is handled (whether to close the socket or continue unsecured) will need
to be decided by the application.

The CC3200 cannot initiate a downgrade to an unsecured socket, nor can
it dynamically upgrade from an insecure to secure socket.

### Set Current Time

To ensure a secure connection, the current time must be set in the device. This time is used to validate the certificate. If the time is beyond the validity period of the certificate, `sl_connect()` will return an error.

``` c
g_time.tm_day = DATE;
g_time.tm_mon = MONTH;
g_time.tm_year = YEAR;
g_time.tm_sec = SECOND;
g_time.tm_hour = HOUR;
g_time.tm_min = MINUTE;

retVal = sl_DeviceSet(SL_DEVICE_GENERAL_CONFIGURATION,
        SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
        sizeof(SlDateTime), (unsigned char *)(&g_time));
```

### Open a Secure Socket

A secure socket must be opened by the CC3200 device. The `sl_Socket()`
function may be used with the protocol parameter set to `SL\_SEC\_SOCKET`.

``` c
SockID = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET);
```

### Force specific method (optional)

The CC3200 supports the SSL 3.0, TLS 1.0, TLS 1.1, and TLS 1.2
protocols/methods. By default, SSL 3.0 and TLS 1.2 are enabled. A
specific method can be forced by using the `sl_SetSockOpt()` function.

``` c
char method = SL_SO_SEC_METHOD_SSLV3;

Status = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_SEC_METHOD, &method, sizeof(method));
```

The full list of supported methods is available in the [CC3100/CC3200 Network Processor User's Guide](http://www.ti.com/lit/swru368).

### Force specific cipher (optional)

By default, the CC3200 will pick the most secure cipher suite that both
sides of the connection can support. A specific cipher can be forced by
using the `sl_SetSockOpt()` function.

``` c
long cipher = SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA;

Status = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_SEC_MASK, &cipher,  sizeof(cipher));
```

The full list of supported ciphers is available in the [CC3100/CC3200 Network Processor User's Guide](http://www.ti.com/lit/swru368).

## TLS/SSL Basics

The CC3200 uses files specific to TLS/SSL that may be defined by the
user at the application level. The files needed are listed below based
on the connection type and must be expressed in the DER format.

### Client Files

1.  **Private Key** - used when server verifies the client. If file
    mapped to 0, the connection may be refused by the server if the
    server wishes to verify client.
2.  **Certificate** - used when server verifies the client. If file
    mapped to 0, the connection may be refused by the server if the
    server wishes to verify client.
3.  **CA, Certificate Authority** - used when verifying the certificate
    provided by the server. There is the option to disable server
    verification by mapping file to id 0. In that case, if the secure
    session established successfully, connect will return with a
    specific error (ESECSNOVERIFY). It\`s application decision to close
    socket, or to ignore it and continue with secured data.
4.  **DH, Diffie-Hellman key** - this file is not needed in the client
    case, map to id 0.

### Server Files

1.  **Private Key** - this file is always needed by a server.
2.  **Certificate** - the file is always needed by a server.
3.  **CA, Certificate Authority** - used when server verifies the
    client. By mapping file to id 0, server will not try to verify the
    certificate of the client.
4.  **DH, Diffie-Hellman key** - This key is only needed for the
    following cipher suites: DHE\_RSA\_WITH\_AES\_256\_CBC\_SHA,
    ECDHE\_RSA\_WITH\_AES\_256\_CBC\_SHA, or
    ECDHE\_RSA\_WITH\_RC4\_128\_SHA.

### Defining secure files

The TLS/SSL files and variables can be defined and mapped to a socket using
the following code.

``` c
typedef struct
{
  unsigned char PrivateKey;
  unsigned char Certificate;
  unsigned char CA;
  unsigned char DH;
} SlSockSecureFiles_t;

SlSockSecureFiles_t SecureFiles;
sockSecureFiles.secureFiles[0] = 0;  // mapping private key, 0 file not exist
sockSecureFiles.secureFiles[1] = 0;  // mapping certificate, 0 file not exist
sockSecureFiles.secureFiles[2] = SL_SSL_CA_CERT/*129*/; // mapping CA, 0 file not exist
sockSecureFiles.secureFiles[3] = 0;  // mapping certificate, 0 file not exist

Status = sl_SetSockOpt(SockID, SL_SOL_SOCKET, SL_SO_SEC_FILES, &SecureFiles, sizeof(SlSockSecureFiles));
```

## Source Files briefly explained

- **main.c** - The main file that explains how certificate can be used
	with SSL.  
- **pinmux.c** - Generated by the PinMUX utility.  
- **startup\_\*.c** - Initialize vector table and IDE related functions 
- **gpio\_if.c** - GPIO interface APIs  

## Usage
  
1. Preload valid CA Certificate in DER format into serial flash using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide). In UniFlash, click **Add File** and name the file "/cert/129.der".
	- For details on finding your CA certificate, see the CA Certificate section below.
2. Edit **common.h** and modify the values of the below macros for the device to connect to the AP:
```c
	#define SSID_NAME           "<ap_name>"    		/* AP SSID */
	#define SECURITY_TYPE       SL_SEC_TYPE_OPEN 	/* Security type (OPEN or WEP or WPA*/
	#define SECURITY_KEY        ""              	/* Password of the secured AP */
```
3.  Run the reference application.
      - Open the project in CCS/IAR. Build the application and debug to load to the device, or flash the binary using [UniFlash](http://processors.wiki.ti.com/index.php/CC3100_%26_CC3200_UniFlash_Quick_Start_Guide).
3.  The application will connect to the configured AP and attempt to connect to "www.google.com" securely using the provided CA certificate. `sl\_Connect` should return with a non-negative value indicating successful connection with the server, and this will be printed to the serial terminal.
	- If an error occurs, the red LED will turn on. If the connection is successful, the green LED will turn on.

### CA Certificate

CA Certificates can be downloaded using various methods. For example, the following shows how to find and download the CA Certificate for www.google.com using the Chrome browser:

1. Open Google Chrome and navigate to www.google.com
2. Click on the lock icon in the browser URL bar. It should tell you about your connection to www.google.com. 
3. Select **Certificate**. In the Certificate Path, you should see the root CA certificate is "Google Trust Services - GlobalSign Root CA-R2". Select **View Certificate**.
4. In the Details tab, select **Copy to File** and export the certificate as a DER format.

For Firefox, the procedure is:

1.  Open FireFox. Go to Tools-\>Options
3.  Click the Advanced tab
4.  In the Advanced, click the Certificates tab
5.  Click **View Certificates**
6.  Find "Google Trust Services - GlobalSign Root CA-R2" and export it as DER format.

## Limitations/Known Issues

  - SSL certificates are not encrypted on the CC3200.
