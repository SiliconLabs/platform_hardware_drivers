# NT3H2x11 Driver

Driver to enable EFR/EFM devices to work with NT3H2111 and NT3H2211. 
NT3H2111 and NT3H2211 belong to NXP's [NTAG I2C](https://www.nxp.com/products/rfid-nfc/nfc-hf/nfc-tags-for-electronics/ntag-ic-iplus-i-nfc-forum-type-2-tag-with-ic-interface:NTAG_I2C) family, which is a NFC Forum Type 2 Tag (T2T) compliant tag IC with I2C interface to a MCU host. 

## Gecko SDK Version

Tested on v2.7.x and v3.0.x.

## API Overview

```
 ---------------------------------------------
|                 Application                 | 
|---------------------------------------------|
|                  nt3h2x11.c                 |
|---------------------------------------------|
|    nt3h2x11_i2c.c    |    nt3h2x11_fd.c     |
|---------------------------------------------|
|                    emlib                    |
 ---------------------------------------------
```

[nt3h2x11.c](src/nt3h2x11.c): implements the top level APIs for application.
- intialization API: initialize I2C communication and FD interrupt.
- memory block R/W APIs: read/write a memory block, given memory address.
- specific register read/write APIs: specific register read/write to get and set settings for NT3H2x11.

[nt3h2x11_i2c.c](src/nt3h2x11_i2c.c): implements NT3H2x11 specific I2C APIs, called by [nt3h2x11.c](src/nt3h2x11.c).
- initialization API: initialize I2C communication.
- I2C read/write APIs: read/write a memory block via I2C, given memory address.
- I2C read/wrtie register APIs: read/write a register via I2C, given block memory address and register address.

[nt3h2x11_fd.c](src/nt3h2x11_fd.c): sets up GPIO interrupt for field detection (FD), called by [nt3h2x11.c](src/nt3h2x11.c). This can be used for NFC wake up.
- initialization API: initialize FD interrupt.

## Peripherals Usage
- 1 I2C port for communication. Details see [nt3h2x11_i2c.c](src/nt3h2x11_i2c.c).
- 1 GPIO pin for FD interrupt. Details see [nt3h2x11_fd.c](src/nt3h2x11_fd.c).

## Application Examples

Detailed usage of this driver and hardware setup please refer to the following examples.

[bluetooth_nfc_pairing_t2t_nt3h2x11]()

[nt3h2x11_field_detection]()

[nt3h2x11_i2c_tag_read]()

[nt3h2x11_i2c_tag_write_ndef]()

[nt3h2x11_t2t_format]()

## Disclaimer ##

NTAG is a registered trademark of NXP B.V.
