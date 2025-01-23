<table class="sphinxhide" width="100%">
 <tr width="100%">
    <td align="center"><img src="https://raw.githubusercontent.com/Xilinx/Image-Collateral/main/xilinx-logo.png" width="30%"/><h1>Wrapping your Custom IP with AXI Packaging and Linux Driver Tutorial</h1>
    </td>
 </tr>
</table>

# Linux Driver Development

In this section of the tutorial, you will cover the process of writing Linux drivers for the 1-Wire core IP. The process will target the 1-Wire core IP; it is difficult to give an example that would be applicable to all kind of IPs as drivers are specific to the IP. That being said, the high level process should be applicable to most IPs.

## Outline

1. [Introduction](#introduction)
2. [The 1-Wire Core Linux Drivers Development](#the-1-wire-core-linux-drivers-development)
   1. [Character Device Driver](#character-device-driver)
   2. [1-Wire Subsystem Driver](#1-wire-subsystem-driver)

## Introduction

Linux driver development is a crucial aspect of embedded system design. The AMD PetaLinux Tools streamline the process of creating custom embedded Linux drivers. In this context, kernel drivers play an essential role in enabling direct communication between the Linux operating system and the underlying hardware components, including intellectual properties (IPs), peripherals, and other critical system elements.

Developing Linux drivers enables engineers to establish a solid foundation for running complex applications, while efficiently managing hardware resources and system performance. Using the PetaLinux development environment, developers can create drivers for their specific hardware configurations, identify and solve potential integration issues, and ensure the seamless operation of their hardware with the Linux operating system.

Before commencing Linux driver development, it is essential to validate the underlying hardware using baremetal drivers, as described earlier. This step provides engineers with valuable insights into the fundamental functionality of hardware devices without the complexity introduced by a full operating system. Once a hardware component has been thoroughly tested and validated through baremetal drivers, developers can confidently proceed to create Linux drivers that provide a higher level of abstraction and integration.

Linux driver development using PetaLinux not only facilitates the interaction between hardware and the operating system, but it also streamlines the process of integrating custom IPs and resolving any software conflicts that might arise during the system design process. Moreover, it enables developers to maintain maximum flexibility when configuring, debugging, and optimizing their embedded Linux distributions based on the target hardware.

By providing Linux drivers that are specifically tailored to work with PetaLinux, you empower your customers to rapidly develop and deploy their applications on a wide range of embedded platforms. These drivers greatly simplify the integration process, allowing system designers to seamlessly incorporate your IP into their projects and expedite the development and debugging processes. Ultimately, this leads to faster prototyping, shorter time-to-market, and overall enhanced end-user experience.

## The 1-Wire Core Linux Drivers Development

Certain drivers in the Linux framework exist within specific subsystems designed to manage devices adhering to particular communication protocols. One such subsystem is the 1-Wire Linux subsystem, which caters to devices employing the 1-Wire protocol. The 1-Wire subsystem comprises an extensive list of master and slave devices, which you can find in the [1-Wire subsystem directory](https://github.com/torvalds/linux/tree/master/drivers/w1). The subsystem also features a [1-Wire framework](https://github.com/torvalds/linux/blob/master/include/linux/w1.h) that outlines guidelines for creating an official 1-Wire driver and identifies the minimum required functions.

To qualify as an official 1-Wire driver, a driver must implement at least the touch bit and reset bus functions as part of the core feature set. An example of an official driver is the [AMD 1-Wire driver](https://github.com/torvalds/linux/blob/master/drivers/w1/masters/amd_axi_w1.c), which has been developed specifically for the AMD 1-Wire IP. This component facilitates effective integration and utilization of the IP in a Linux-based system.

In this tutorial, you will not delve into the technicalities of developing an official 1-Wire driver, as that would be too specific. Instead, you will focus on using the character device framework, a more versatile approach to driver development, accommodating various devices that perform read and write operations. This framework is particularly suitable for AXI IP and its respective drivers, which primarily involve reading and writing to AXI registers. Linux systems are particularly adept at handling interrupts, offering several advantages over baremetal systems. By utilizing interrupt-driven mechanisms, drivers in Linux can significantly increase efficiency and improve resource utilization, thereby optimizing the overall system performance and responsiveness.

By employing the character device framework in your Linux driver development, you can build robust, scalable, and flexible drivers that work seamlessly with a wide range of devices. This broad compatibility ultimately simplifies not only the driver development process but also the integration of IPs and hardware components into advanced, high-performance Linux-based embedded systems.

### Character Device Driver

1. Create a PetaLinux project.
   1. Source the PetaLinux installation, usually using a command similar to ```source <petalinux_install>/settings.sh```.
   2. Move into your working directory: ```cd <working_directory>```.
   3. Create the project: ```petalinux-create -t project -n 1wire --template zynqMP```
   4. Move into the project directory: ```cd 1wire```
   5. Update the hardware configuration of the PetaLinux project with the hardware: ```petalinux-config --get-hw-description=<working_directory>/myproject/mydesign_wrapper.xsa```
   6. In the PetaLinux System Configuration window go to *Subsystem Hardware Settings &rarr; Serial Settings* to change the STDIN and STDOUT setting:
      + *PMUFW Serial stdin/stdout*: psu_uart_1
      + *FSBL Serial stdin/stdout*: psu_uart_1
      + *TF-A Serial stdin/stdout*: psu_uart_1
      + *U-boot/Linux Serial stdin/stdout*: psu_uart_1
   7. Exit and save the new configuration
2. Create and implement the driver module:
   1. Create the module: ```petalinux-create -t modules -n xlnxw1 --enable```
   2. Look at the content of the created module under `<working_directory>/1wire/project-spec/meta-user/recipes-modules/xlnxw1`.
   3. You will now edit the driver source file:
      1. Open `<working_directory>/1wire/project-spec/meta-user/recipes-modules/xlnxw1/files/xlnxw1.c`.
      2. Copy the content of `<working_directory>/reference_files/linux_driver/w1chardev.c` to the source file.
   4. You can study the content of the driver.
      + Basic functions to read and write the AXI registers:
         <details>
         <summary>Registers read and write</summary>

         ```C
            static inline void xlnxw1_write_register(u8 reg_offset, u32 val);
            static inline u32 xlnxw1_read_register(u8 reg_offset);
         ```

         </details>
      + These functions are then used to implement the 1-Wire functions:
         <details>
         <summary>1-Wire functions</summary>

         ```C
            static long xlnxw1_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
            {
               switch (cmd)
               {
               case XLNX_IOCTL_RESET_BUS:
                  ...
               case XLNX_IOCTL_READ_BIT:
                  ...
               case XLNX_IOCTL_WRITE_BIT:
                  ...
               case XLNX_IOCTL_READ_BYTE:
                  ...
               case XLNX_IOCTL_WRITE_BYTE:
                  ...
               }
            };
         ```

         </details>
      + ```xlnxw1_open```: Checks if the device is in use, increments the usage count, and ensures that the module is loaded while it is being used.
      + ```xlnxw1_release```: Decrements the usage count of the device and releases the module reference.
      + ```xlnxw1_irq```: Clears the IRQ enable register and wakes up the waiting queue when an interrupt is triggered.
      + ```xlnxw1_probe```: Initializes the driver resources (memory, IRQ, etc.) and stores device-related information.
      + ```xlnxw1_remove```: Releases the driver resources (memory, IRQ, etc.) and deallocates device data structures.
      + ```xlnxw1_of_match```: A table containing compatible device strings used to match the driver with a device tree node.
      + ```xlnxw1_driver```: Defines the platform driver, its name, probe, remove functions, and compatibility table.
      + ```xlnxw1_init```: Registers the character device with a major number, creates the device class and device, and registers the platform driver.
      + ```xlnxw1_exit```: Destroys the device, unregisters the device class, unregisters the character device, and unregisters the platform driver.
3. Create and implement a 1-wire application:
   1. Create the application: ```petalinux-create -t apps -n xlnxw1-app --enable```
   2. Overwrite the application with the one provided with the tutorial: ```cp <working_directory>/reference_files/linux_driver/xlnxw1-app.c <working_directory>/1wire/project-spec/meta-user/recipes-apps/xlnxw1-app/files/xlnxw1-app.c```.
   3. You can have a look at the content of the `xlnxw1-app.c`. The application, probe the 1-Wire temperature sensor for the temperature and display it.
4. Build and test the 1-Wire driver and application:
   1. Build the project: ```petalinux-build```
   2. Connect everything:
      + Connect the 1-Wire temperature sensor to your KD240
      + Connect the USB JTAG/UART cable from the KD240 to your computer
      + Connect the network ethernet cable from the KD240 to a network switch
      + Make sure there is no SD card inserted in the KD240
      + Connect the power supply to the KD240
   3. Load the project using TFTP:
      1. Retrieve your computer IP address using ```ifconfig```.
      2. On the KD240, you should see the ```ZynqMP>``` prompt, do the following to load the bitstream and boot PetaLinux:

         ```bash
            ZynqMP> setenv serverip <your computer IP>
            ZynqMP> tftpboot 0x02000000 system.bit
            ZynqMP> fpga load 0 0x02000000 $filesize
            ZynqMP> pxe get
            ZynqMP> pxe load
         ```

      3. Look at the kernel messages, you should see:

         ```bash
            [    8.660297] Registration successful, 1-wire device's major number is 237.
            [    8.667246] 1-wire class registration successful.
            [    8.676375] 1-wire device created successfully.
            [    8.681842] xlnxw1 a0000000.axi_1wire_host: xlnxw1 mapped to 0x81ea0000, irq=53
         ```

         The timestamps, major number, address and irq might differ on your session.
   4. To login, use the username, `Petalinux`, and set your password.
   5. Run the application by entering ```sudo xlnxw1-app```, you should see a message saying that the device is open and the temperature should be printing out.

### 1-Wire Subsystem Driver

As mentioned previously, there is a specific driver subsystem for the 1-Wire devices family. The AMD 1-Wire IP was develop with its own specific 1-Wire driver that has been upstreamed to the main Linux kernel. Without going through the driver details, you will go through enabling it in PetaLinux to test it with peripherals devices.

1. Modify the PetaLinux project to use the 1-Wire subsystem:
   1. Replace the character driver previously created with the 1-Wire driver: ```cp <working_directory>/reference_files/linux_driver/amd_axi_w1.c <working_directory>/1wire/project-spec/meta-user/recipes-modules/xlnxw1/files/xlnxw1.c```.
   2. Enable the 1-wire subsystem and slave devices:
      1. ```petalinux-config -c kernel```
      2. Go to *Device Drivers*, and find *Dallas's 1-wire support*. Press **Y** to include the 1-Wire subsystem.
      3. Select **Dallas's 1-wire support**, and make sure to include **Userspace communication over connector**.
      4. Select **1-wire Slaves** and include all slaves or only your device if you prefer.
      5. Exit and save the configuration.
   3. Enable the lm sensors package group
      1. ```petalinux-config -c rootfs```
      2. Go to *PetaLinux Package Groups &rarr; packagegroup-lmsensors* and press **Y** to include *packagegroup-lmsensors*.
      3. Exit and save the configuration.
2. Build and test the 1-Wire project:
   1. Rebuild the project: ```petalinux-build```
   2. Load the project using TFTP:
      1. Retrieve your computer IP address using ```ifconfig```
      2. On the KD240, you should see the ```ZynqMP>``` prompt. Do the following to load the bitstream and boot PetaLinux:

         ```bash
            ZynqMP> setenv serverip <your computer IP>
            ZynqMP> tftpboot 0x02000000 system.bit
            ZynqMP> fpga load 0 0x02000000 $filesize
            ZynqMP> pxe get
            ZynqMP> pxe load
         ```

      3. Look at the kernel messages, you should see:

         ```bash
            [    7.906413] w1_master_driver w1_bus_master1: Attaching one wire slave 28.000000040711 crc 2d
         ```

         The timestamps, and device details will be different depending on what device you have connected to the KD240.

      4. To login, use the username,`petalinux`, and set your password.
      5. Type ```sensors``` and you should see the temperature reading from your temperature sensors or whatever information is captured by your 1-wire device.

 ---

   You now have a driver for your integrated, packaged IP. This driver efficiently utilizes the interrupt feature embedded within your IP. Although it is not mandatory to implement interrupts with a Linux driver, it is strongly advised to do so in order to prevent unnecessary consumption of CPU resources.

   For more information on the PetaLinux tools, refer to the *PetaLinux Tools Documentation: Reference Guide* [(UG1144)](https://docs.amd.com/r/en-US/ug1144-petalinux-tools-reference-guide).

---
<p class="sphinxhide" align="center"><sub>Copyright © 2025 Advanced Micro Devices, Inc.</sub></p>
<p class="sphinxhide" align="center"><sup><a href="https://www.amd.com/en/corporate/copyright">Terms and Conditions</a></sup></p>