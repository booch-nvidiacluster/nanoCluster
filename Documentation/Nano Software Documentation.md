# Nano Software Documentation
The cluster's essential computational resource.

<img src="/Documentation/Images/Jetson Nano.jpg" alt="Jetson Nano">

Here are instructions for provisioning each Nano in the cluster with a basic stack - Ubuntu, Python, Apache, and MySQL - together with various tools and libraries for general development, frameworks for AI, and an infrastructure for hosting microservices.

Several bits of code needed in support of this provisioning reside <a href="../nano">here</a>.

## Ubuntu
 
1.	Download the Jetson Nano Developer Kit SD Card Image from https://developer.nvidia.com/embedded/jetpack to a host computer.
 
2.	Write the image from the host computer to the Nano's memory card using https://www.balena.io/etcher/.
 
3. Install the memory card in the Nano, attach the Nano to a monitor, keyboard, and mouse, boot the Nano, then follow the on screen start up instructions to configure the computer's name (in the form *nanoCluster#*, where # is the node ID from 0 to 3), user account, and wireless connection (see also https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#setup-first-boot).
 
4. Use the Nano's Terminal application to clone this repository (see also https://www.jetsonhacks.com/2019/09/17/jetson-nano-run-from-usb-drive/).
```
sudo git clone https://github.com/jetsonHacksNano/rootOnUSB
```

5. Attach the SSD to the Nano's USB port, then use the Nano's Disk application to name (in the form *nanoCluster#SSD*, where # is the node ID from 0 to 3), format, and mount the SSD.

      Disks -> Format -> Compatible with modern systems and hard drives
  
      Disks -> Add Partition -> 500GB | nanoCluster#SSD | internal disk

6. Copy the root file system to the SSD (write down the SSD's UUID for later use).
```
cd rootOnUSB
./addUSBToInitramfs.sh
./copyRootToUSB.sh -p /dev/sda1
./diskUUID.sh
```

7. Redirect the root file system.
<pre><code>cd /boot/extlinux
sudo vim extlinux.conf
    <i>Change the INITRD line to</i>
        INTRD /boot/initrd-xusb.img
    <i>Change the APPEND line to the UUID for sda1</i>
        APPEND ${cbootargs} root=UUID=<UUID for sda1> rootwait rootfstype=ext4</code></pre>

8. Reboot the Nano.
```
sudo reboot now
```

9. Configure the Nano for high power mode.
```
sudo nvpmodel -m 0
```
 .
10. Disable zram swap
```
sudo swapoff /dev/zram0
sudo swapoff /dev/zram1
sudo swapoff /dev/zram2
sudo swapoff /dev/zram3
sudo zramctl --reset /dev/zram0
sudo zramctl --reset /dev/zram1
sudo zramctl --reset /dev/zram2
sudo zramctl --reset /dev/zram3
```

11. Enable SSD swapspace (see also https://www.jetsonhacks.com/2019/04/14/jetson-nano-use-more-memory/).
```
cd /home/nano/Downloads
sudo git clone https://github.com/JetsonHacksNano/installSwapfile
cd installSwapfile
./installSwapfile.sh -s 12
```

12. Reboot the Nano.
```
sudo reboot now
```

13. Refresh the installation.
```
sudo apt update
sudo apt upgrade
sudo apt unminimize
sudo apt autoremove
```

14. If you want to run a headless node, disable the GUI then proceed to step 23; otherwise proceed to the step 15.
```
sudo systemctl set-default multi-user.target
```

15.

16.

17.

18.

19.

20.

21.

22.

23. Reboot the Nano.
```
sudo reboot now
```

## Python
 
## Apache
 
## MySql
 
## Libraries
 
## Frameworks
 
## Microservices
