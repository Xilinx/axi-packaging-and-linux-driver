<table class="sphinxhide" width="100%">
 <tr width="100%">
    <td align="center"><img src="https://raw.githubusercontent.com/Xilinx/Image-Collateral/main/xilinx-logo.png" width="30%"/><h1>Wrapping your Custom IP with AXI Packaging and Linux Driver Tutorial</h1>
    </td>
 </tr>
</table>

# Introduction

This tutorial covers the essentials steps in packaging your own intellectual property (IP) core with an AXI wrapper that can then be used with AMD Vivado&trade; for different projects. It also covers the steps to develop and integrate a Linux driver for that IP.  
This tutorial will be using the AXI 1-Wire Host design as an example of how to package an IP and develop a Linux driver. The reference files are publicly available here:

* Packaged 1-Wire Host IP Design: [AMD AXI 1-Wire Host Design](https://github.com/Xilinx/axi_1wire_host-design)
* 1-Wire subsystem compatible Linux driver [AMD AXI 1-Wire Driver](https://github.com/Xilinx/linux-xlnx/blob/master/drivers/w1/masters/amd_axi_w1.c)
* 1-Wire HDL, constraint, and driver files: [AXI Packaging and Linux Driver Tutorial reference file](./reference_files/)

The [AMD Kria&trade; KD240 Drives Starter Kit](https://www.amd.com/en/products/system-on-modules/kria/k24/kd240-drives-starter-kit.html) is targeted to support this tutorial as it is equipped with a 1-Wire interface, but any other AMD platform with a processing system (PS) (such as an Arm processor or a soft processor like an [AMD MicroBlaze™](https://www.amd.com/en/products/software/adaptive-socs-and-fpgas/microblaze.html) or an [AMD MicroBlaze™ V](https://www.amd.com/en/products/software/adaptive-socs-and-fpgas/microblaze-v.html)) can be used.

> **IMPORTANT**  
> If you are using a platform not equipped with a dedicated 1-Wire interface, use an external pull-up resistor as stated by the 1-Wire device manufacturer.

## Before You Begin

Required tools:

* Host machine with Linux installed (refer to the *PetaLinux Tools Documentation: Reference Guide* ([UG1144](https://docs.amd.com/r/en-US/ug1144-petalinux-tools-reference-guide/Installation-Requirements)) (for the supported OS)
* AMD Vivado&trade; Design Suite 2024.2
* AMD Vitis&trade; Unified Software Platform 2024.2
* AMD PetaLinux Tools 2024.2
* TFTP server configured on your Linux machine (refer to [Setting-Up-TFTP-Server-for-PetaLinux](https://www.instructables.com/Setting-Up-TFTP-Server-for-PetaLinux/))

### Accessing the Tutorial Reference files

In the terminal, enter the `git clone https://gitenterprise.xilinx.com/thomasd/AXI-packaging-and-Linux-driver` command.

## Tutorial Overview

1. [AXI IP Packaging](./1_axi_packaging.md): Goes over the steps to properly package an AXI IP using the Vivado Design Suite.
2. [Baremetal Driver Development](./2_baremetal_driver.md): Walks through the process of developing a baremetal driver for the packaged AXI IP using the Vitis Unified Software Platform.
3. [Linux Driver Design and Deployment](./3_linux_driver.md): Shows how to develop a general Linux driver for the packaged AXI IP using the AMD PetaLinux Tools and how to deploy it.

<p align="center"><b>Start the next step: <a href="./1_axi_packaging.md">AXI IP Packaging</a></b></p>


<p class="sphinxhide" align="center"><sub>Copyright © 2025 Advanced Micro Devices, Inc.</sub></p>
<p class="sphinxhide" align="center"><sup><a href="https://www.amd.com/en/corporate/copyright">Terms and Conditions</a></sup></p>