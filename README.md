# dpdk-echo-server

A simple, very limited, implementation of a UDP echo server using DPDK (23.11).

Based on the Basic Forwarding Sample Application in the DPDK examples directory.

## Build instructions
Follow the instruction in the [Getting Started Guide for Linux](https://doc.dpdk.org/guides/linux_gsg/index.html)


$ export RTE_SDK=/path/to/dpdk
$ cd /path/to/dpdk-echo-server
$ ./setup.sh
$ make

View devices
`$ $RTE_SDK/usertools/dpdk-devbind.py --status`

    Network devices using kernel driver
    ===================================
    0000:00:19.0 '82579LM Gigabit Network Connection (Lewisville) 1502' if=eno1 drv=e1000e unused=vfio-pci 
    0000:01:00.0 '82574L Gigabit Network Connection 10d3' if=enp1s0 drv=e1000e unused=vfio-pci *Active*
    0000:03:00.0 '82599ES 10-Gigabit SFI/SFP+ Network Connection 10fb' if=ens4f0 drv=ixgbe unused=vfio-pci 
    0000:03:00.1 '82599ES 10-Gigabit SFI/SFP+ Network Connection 10fb' if=ens4f1 drv=ixgbe unused=vfio-pci 
    0000:06:00.0 'SFC9220 10/40G Ethernet Controller 0a03' if=ens2f0np0 drv=sfc unused=vfio-pci 
    0000:06:00.1 'SFC9220 10/40G Ethernet Controller 0a03' if=ens2f1np1 drv=sfc unused=vfio-pci 
    
    No 'Baseband' devices detected
    ==============================
    
    No 'Crypto' devices detected
    ============================
    
    No 'DMA' devices detected
    =========================
    
    No 'Eventdev' devices detected
    ==============================
    
    No 'Mempool' devices detected
    =============================
    
    No 'Compress' devices detected
    ==============================
    
    No 'Misc (rawdev)' devices detected
    ===================================
    
    No 'Regex' devices detected
    ===========================
    
    No 'ML' devices detected
    ========================

Bind a device to DPDK
`$ sudo $RTE_SDK/usertools/dpdk-devbind.py --bind=vfio-pci 03:00.0`

The output of the `--status` command should now look something like this
`$ $RTE_SDK/usertools/dpdk-devbind.py --status`

    Network devices using DPDK-compatible driver
    ============================================
    0000:03:00.0 '82599ES 10-Gigabit SFI/SFP+ Network Connection 10fb' drv=vfio-pci unused=ixgbe
    
    Network devices using kernel driver
    ===================================
    0000:00:19.0 '82579LM Gigabit Network Connection (Lewisville) 1502' if=eno1 drv=e1000e unused=vfio-pci 
    0000:01:00.0 '82574L Gigabit Network Connection 10d3' if=enp1s0 drv=e1000e unused=vfio-pci *Active*
    0000:03:00.1 '82599ES 10-Gigabit SFI/SFP+ Network Connection 10fb' if=ens4f1 drv=ixgbe unused=vfio-pci 
    0000:06:00.0 'SFC9220 10/40G Ethernet Controller 0a03' if=ens2f0np0 drv=sfc unused=vfio-pci 
    0000:06:00.1 'SFC9220 10/40G Ethernet Controller 0a03' if=ens2f1np1 drv=sfc unused=vfio-pci 
    
    No 'Baseband' devices detected
    ==============================
    
    No 'Crypto' devices detected
    ============================
    
    No 'DMA' devices detected
    =========================
    
    No 'Eventdev' devices detected
    ==============================
    
    No 'Mempool' devices detected
    =============================
    
    No 'Compress' devices detected
    ==============================
    
    No 'Misc (rawdev)' devices detected
    ===================================
    
    No 'Regex' devices detected
    ===========================
    
    No 'ML' devices detected
    ========================

