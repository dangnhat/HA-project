ABSTRACT
==========
With the development of embedded devices with wireless communication capacity, Internet of Things and its application such as Home Automation has been receiving many interests in research and development for recent years. 

In this project, we will build a Home Automation system based on 6LoWPAN network stack. 6LoWPAN bases on IPv6 with many modifications and compression to fit with embedded devices. It has the ability to connect to the Internet and other IP network easier without any complex conversion than other embedded network stacks such as Zigbee or Z-Wave. Beside, our Home Automation system can control the devices automatically bases on user-defined scenarios; can communicate via Bluetooth Low Energy with mobile device so mobile device could control end devices in 6LoWPAN network and build some scenarios for them. The flexibility for device’s configurations is one of the most importance factors. As a result, end devices can be configured via computer on-the-field without the need of reset or re-programming. We use RIOT and its network stack for operation system services, power management, and network communication. MBoard-1 with STM32F103 micro-controller and CC1101 wireless module is used as basic hardware component for all devices in our system. 

In this stage of development, there are some limitations: optimization for code size, power consumption in both hardware and software as well as the ability to connect to the Internet will not be focused; communication in 6LoWPAN network bases directly on UDP of transport layer rather than application protocol such as CoAP; due to the instability of RPL in RIOT’s 6LoWPAN stack, our system will not have any router and communication could only be done in one hop.

To get started quickly you should download the tarball:
https://drive.google.com/file/d/0B7idR0mbHSqzM0FNc254bGgyRlE/view?usp=sharing
It contains everythings needed.

Final project video:
https://www.youtube.com/watch?v=3W_C0G6DxqU
