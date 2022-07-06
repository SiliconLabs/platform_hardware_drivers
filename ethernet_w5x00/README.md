# W5500 Ethernet Module #

## Description ##

This project aims to implement a hardware driver for the W5500 ethernet controller that is integrated on the ETH Wiz click board. The W5500 chip is a Hardwired TCP/IP embedded Ethernet controller that enables easier internet connection for embedded systems using SPI (Serial Peripheral Interface).

W5500 enables users to have the Internet connectivity in their applications just by using the single chip to implement TCP/IP Stack, 10/100 Ethernet MAC and PHY. Hardwired TCP/IP stack supports TCP, UDP, IPv4, ICMP, ARP, IGMP, and PPPoE. W5500 uses a 32Kbytes internal buffer as its data communication memory. For more information about the W5500, see the following [source](https://docs.wiznet.io/img/products/w5500/w5500_ds_v109e.pdf).

ETH Wiz click communicates with the target microcontroller over SPI interface that corresponds to the pinout on the mikroBUS™ socket as shown below.

![mikroBus](doc/images/mikrobus.png)

## Gecko SDK version ##

GSDK v4.1.0

## Hardware Required ##

- [A BGM220P Explorer Kit board.](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)

- [A ETH WIZ Click.](https://www.mikroe.com/eth-wiz-click)

- A Ethernet Cable, e.g. [Ethernet Roll Cable](https://www.mikroe.com/ethernet-roll-transparent).

## Connections Required ##

The ETH Wiz click board can just be "clicked" into its place. Be sure that the board's 45-degree corner matches the Explorer Kit's 45-degree white line.

![board](doc/images/board.png)

## Setup ##

To test this application, you should connect the BMG220 Explorer Kit Board to the PC using a microUSB cable.

You can either import the provided **ethernet_w5x00_simple.sls** project file or start with an empty example project as basis:

1. Create a "Platform - Empty C Project" project for the "BGM220 Explorer Kit Board" using Simplicity Studio v5. Use the default project settings. Be sure to connect and select the BGM220 Explorer Kit Board from the "Debug Adapters" on the left before creating a project.

2. Copy all attached files in inc and src folders into the project root folder (overwriting existing app.c).

3. Install the software components:

   - Open the .slcp file in the project.

   - Select the *SOFTWARE COMPONENTS* tab.

   - Install the following components:

     - **[Platform] > [Driver] > [SPIDRV]** with the default instance name: **mikroe**. Then select **CS controlled by the application**.
       ![spi configure](doc/images/spi_configure.png)
     - **[Services] > [IO Stream] > [IO Stream: USART]** with the default instance name: **vcom**.
     - **[Services] > [Sleep Timer]**.
     - **[Services] > [Microsecond Delay]**.
     - **[Application] > [Utility] > [Log]**.
     - **[Application] > [Utility] > [Assert]**.

4. Build and flash the project to your device.

## How It Works ##

### API Overview ###

A detailed description of each function can be found in [doc/doxygen](doc/doxygen/html/modules.html).

The driver diveded into a Application layer, a TCP/IP stack and a Physical layer.
The Application layer is where applications requiring network communications live. Examples of these applications include email clients and web browsers. These applications use the TCP/IP stack to send requests to connect to remote hosts.

The TCP/IP stack establishes the connection between applications running on different hosts. It uses TCP for reliable connections and UDP for fast connections. It keeps track of the processes running in the applications above it by assigning port numbers to them and uses the Network layer to access the TCP/IP network.

The physical layer provides integration to the host microcontroller hardware-dependent codes.

![software_layers](doc/images/software_layers.png)

[dhcp.c](src/dhcp.c): DHCP library. The dynamic host configuration protocol (DHCP) is the application responsible for requesting and offering IP addresses.

[dns.c](src/dns.c): DNS library. A Domain Name System (DNS) enables to browse to a website by providing the website or domain name instead of the website’s IP address.

[sntp.c](src/sntp.c): SNTP library. SNTP stands for Simple Network Time Protocol, which is a service that provides the time of day to network devices. Typical accuracy is in the range of hundreds of milliseconds.

[http_server.c](src/http_server.c): HTTP server library. The Hypertext Transfer Protocol (HTTP) is the most commonly used TCP/IP application as it transfers web pages from a web server to a web browser.

[socket.c](src/socket.c): Enables applications to connect to a Transmission Control Protocol/Internet Protocol (TCP/IP) network.

[ethernet_udp.c](src/ethernet_udp.c): Library to send/receive UDP packets.

[ethernet_server.c](src/ethernet_server.c): Library is for all Ethernet server based calls. It is not called directly, but invoked whenever you use a function that relies on it.

[ethernet_client.c](src/ethernet_client.c): Library is for all Ethernet client based calls. It is not called directly, but invoked whenever you use a function that relies on it.

[w5x00.c](src/w5x00.c): Implements public interfaces to interact with W5500 chip.

[w5x00_platform.c](src/w5x00_platform.c): Integrates the Silabs SPI driver for SPI communication.

### Testing ###

This example demonstrates the http client features of the driver.
The connection diagram of this example is shown in the image below:

![diagram](doc/images/diagram.png)

The following diagram shows the program flow as implemented in the app.c file:

![Work Flow](doc/images/flow.png)

Use a terminal program, such as the Console that is integrated in Simplicity Studio or a third-party tool terminal like PuTTY to receive the logs from the virtual COM port. You should expect a similar output to the one below.

![log](doc/images/log.png)

## .sls Projects Used ##

[ethernet_w5x00_simple.sls](/SimplicityStudio/ethernet_w5x00_simple.sls)
