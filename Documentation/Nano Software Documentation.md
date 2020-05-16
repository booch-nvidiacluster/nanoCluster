# Nano Software Documentation
The cluster's essential computational resource.

<img src="/Documentation/Images/Jetson Nano.jpg" alt="Jetson Nano">

Here are instructions for provisioning each Nano in the cluster with a basic stack - Ubuntu, Python, Apache, and MySQL - together with various tools and libraries for general development, frameworks for artificial intelligence, and an infrastructure for hosting microservices.

Several bits of code needed in support of this provisioning reside <a href="../nano">here</a>.

## Ubuntu
 
1. Download the Jetson Nano Developer Kit SD Card Image from https://developer.nvidia.com/embedded/jetpack to a host computer.
 
2. Write the image from the host computer to the Nano's memory card using https://www.balena.io/etcher/.
 
3. Install the memory card in the Nano, attach the Nano to a monitor, keyboard, and mouse, boot the Nano (but first ensure that the SSD is NOT yet connected to the Nano), then follow the on screen start up instructions to configure the computer's name (in the form *nanoCluster#*, where # is the node ID from 0 to 3), user account, and wireless connection (see also https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#setup-first-boot).
 
4. Use the Nano's Terminal application to clone this repository (see also https://www.jetsonhacks.com/2019/09/17/jetson-nano-run-from-usb-drive/).
```
sudo git clone https://github.com/jetsonHacksNano/rootOnUSB
```

5. Attach the SSD to the Nano's USB port, then use the Nano's Disk application to name (in the form *nanoCluster#SSD*, where # is the node ID from 0 to 3), format, and mount the SSD.

      Disks -> Format -> Compatible with modern systems and hard drives<br>
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
    <i>Change the INITRD line to the following.</i>
        INTRD /boot/initrd-xusb.img
    <i>Change the APPEND line to reflect the UUID for sda1.</i>
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

11. Enable SSD swapping (see also https://www.jetsonhacks.com/2019/04/14/jetson-nano-use-more-memory/).
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

15. Patch notify.
```
sudo apt install gir1.2-notify-0.7
```

16. Use the Nano's Software application to remove various superflurous other applications.

      Software ->  installed -> remove

      a. Activity Log Monitor
      b. Aislerot Solitaire      
      c. Amazon      
      d. Calendar      
      e. Cheese      
      f. Deja Dup Backup Tool      
      g. Document Viewer      
      h. Eye of Gnome      
      i. File Roller      
      j. GNOME Calculator      
      k. GNOME Fonts      
      l. GNOME Mahjongg      
      m. GNOME Mines      
      n. GNOME Power Statistics      
      o. GNOME Screenshot      
      p. GNOME Sudkuo      
      q. Ibus Table      
      r. Input Method      
      s. LibreOffice      
      t. LibreOffice Calc      
      u. LibreOffice Draw      
      v. LibreOffice Impress      
      w. LibreOffice Math      
      x. LibreOffice Writer      
      y. Photo Lens for Unity      
      z. PrintSettings      
      aa. Remmina      
      bb. RhythmBox      
      cc. Seahorse      
      dd. Shotwell      
      ee. SimpleScan      
      ff. Thunderbird Mail      
      gg. ToDo      
      hh. Transmission      
      ii. UXTerm      
      jj. Videos      
      kk. Vim      
      ll. XTerm
      
      This leaves eleven user applications.
      
      a. Chromium Web Browser      
      b, Disk Usage Analyzer      
      c. gedit      
      d. GNOME Central Control      
      e. GNOME Disks      
      f. GNOME Help      
      g. GNOME Software      
      h. GNOME System Monitor      
      i. IBus Preferences      
      j. Startup Applications      
      k. Terminal
      
      This also leaves four system applications.
      
      a) Nautilus      
      b) Software      
      c) Software Updater      
      d) Unity Control Center
      
17. Use the Nano's System Settings application to customize the desktop.

      System Settings -> Appearance -> Background -> Colors & Gradients -> black      
      System Settings -> Brightness & Look -> Turn screen off when inactive -> never      
      System Settings -> Brightness & Look -> Lock -> off      
      System Settings -> Brightness & Look -> Require my password when walking from suspend -> off      
      System Settings -> Bluetooth -> off

18. Configure the Nano's launcher.

      Delete all desktop items.      
      Unlock L4T-README from the launcher.      
      Unlock 64 GB Volume from the launcher.
      Launch the Terminal application and lock it to the launcher.      
      Launch the Chromium Web Browser and lock it to the launcher.
      
19. Patch desktop sharing (see also https://www.hackster.io/news/getting-started-with-the-nvidia-jetson-nano-developer-kit-43aa7c298797).
<pre><code>sudo vim /usr/share/glib-2.0/schemas/org.gnome.Vino.gschema.xml
    <i>Add the following key.</i>
        &lt;key name='enabled' type='b'&gt;
	    &lt;summary&gt;
	        Enable remote access to the desktop
	    &lt;/summary&gt;
            &lt;description&gt;
                If true, allows remote access to the desktop via the RFB
                protocol. Users on remote machines may then connect to the
                desktop using a VNC viewer.
            &lt;/description&gt;
            &lt;default&gt;
	        false
	    &lt;/default&gt;
	&lt;/key&gt;
sudo glib-compile-schemas /usr/share/glib-2.0/schemas</code></pre>

20. Use the Nano's System Settings application to customize desktop sharing.

      System Settings -> Desktop Sharing -> Allow others to view your desktop      
      System Settings -> Desktop Sharing -> Allow others to control your desktop      
      System Settings -> Desktop Sharing -> You must confirm access to this machine -> off
      
21. Use the Nano's Startup Applications applications to launch virtual network computing on start up.

      Startup Applications -> Add -> Vino | /usr/lib/vino/vino-server | VNC server

22. Configure virtual network computing.
```
gsettings set org.gnome.Vino require-encryption false
gsettings set org.gnome.Vino prompt-enabled false
```

23. Reboot the Nano.
```
sudo reboot now
```

## Python

1. Install the latest Python package installers (see also https://pypi.org/project/pip/)
```
sudo apt install python-pip
sudo apt install python3-pip
python3 -m pip install --upgrade pip
```

2. Install the latest versions of Python (see also https://docs.python.org/3/)
```
sudo apt install python3.7
sudo apt install python3.8
```

3. Direct Python3 to use Python3.8
```
sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.6 1
sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.7 2
sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 3
```

4. Confirm that Python3 directs to Python3.8
```
sudo update-alternatives --config python3
python3 -V
```

5. Install Cython, a Python superset (see also https://cython.org)
```
sudo apt cython
```

## Apache

1. Install Apache (see also https://ubuntu.com/tutorials/install-and-configure-apache
```
sudo apt install apache2
```

2. Confirm that Apache is properly installed by going to a host computer and entering the Nano's IP address in a browser.

3. Replace the Nano's default web page using files from <a href="../nano">here</a> To get the URL for the html, find the appropriate html (in the form *index#.html* where # is the node ID from 0 to 3) then select Raw. To get the URL for the image (in the form *nanoCluster#.jpg* where # is the node ID from 0 to 3), find the appropriate file then select Download.
<pre><code>cd /var/www/html
sudo rm index.html
sudo wget <i>URL for index#.html</i> -O index.html
sudo wget <i>URL for nanoCluster#.jpg</i> -O nanoCluster#.jpg</code></pre>

3. Confirm that these changes are properly installed by going to a host computer and entering the Nano's IP address in a browser.
 
## MySQL

1. Install MySQL (see also https://ubuntu.com/server/docs/databases-mysql)
```
sudo apt install mysql-server
```

2. Confirm that MySQL is running correctly.
```
sudo ss -tap | grep mysql
```
 
## Tools

1. Install tools for building, testing, and packaging software (see also https://cmake.org)
```
sudo apt install cmake
```

2. Install tools for transferring data (see also https://curl.haxx.se)
```
sudo apt install curl
```

3. Install tools for Fortran (see also https://gcc.gnu.org/wiki/GFortran)
```
sudo apt install gfortran
```

4. Install tools for reporting on system loads (see also http://sebastien.godard.pagesperso-orange.fr)
```
sudo apt install sysstat
```

## Libraries (Hardware)

1. Install libraries for querying CPU information (see also https://pypi.org/project/grpcio/)
```
sudo pip3 install py-cpuinfo
```

2. Install libraries for manipulating the Nano's general purpose input/output pins (see also https://github.com/NVIDIA/jetson-gpio)
```
sudo pip3 install Jetson.GPIO
sudo groupadd -f -r gpio
sudo usermod -a -G gpio nano
sudo cp /opt/nvidia/jetson-gpio/etc/99-gpio.rules /etc/udev/rules.d
sudo reboot now
```

## Libraries (Networking)

1. Install libraries for finding available network ports (see also https://pypi.org/project/portpicker/)
```
sudo pip3 install portpicker
```

## Lbraries (Python)

1. Install libraries for representing Python’s abstract syntax trees (see also https://pypi.org/project/gast/)
```
sudo pip3 install gast
```

2. Install libraries for manipulating Python’s abstract syntax trees (see also https://pypi.org/project/astor/)
```
sudo pip3 install astor
```

3. Install libraries for declaring Python enumerations (see also https://pypi.org/project/enum34/)
```
sudo pip3 install enum34
```

4. Install libraries for constructing Python function wrappers and decorators (see also https://pypi.org/project/wrapt/)
```
sudo pip3 install wrapt
```

5. Install libraries for concurrency (see also https://pypi.org/project/futures3/)
```
sudo pipe install futures3
```

6. Install libraries for supporting cross version Python codebases (see https://pypi.org/project/future/)
```
sudo pip3 install future
```

7. Install libraries for building Python applications (see also https://pypi.org/project/grpcio/).
```
sudo pip3 install absl-py
```

8. Install libraries for testing Python applications (see also https://pypi.org/project/testresources/)
```
sudo pip3 install testresources
```

## Libraries (Data)

1. Install libraries for storing hierarchical data  (see also https://www.hdfgroup.org/solutions/hdf5/)
```
sudo apt install libhdf5-serial-dev hdf5-tools libhdf5-dev zlib1g-dev zip libjpeg8-dev
```

2. Install libraries for manipulating HDF data (see also https://www.h5py.org)
```
sudo pip3 install p5py
```

## Libraries (Numeric)

1. Instal libraries for scientific computing (see also https://numpy.org)
```
sudo pip3 install NumPy
```

2. Install libraries for linear algebra (see also http://math-atlas.sourceforge.net)
```
sudo apt install libatlas-base-div
```

## Libraries (Visualization)

1. Install libraries for formatting terminal output (see also https://pypi.org/project/termcolor/)
```
sudo pip3 install termcolor
```

2. Install libraries for imaging (see also https://pypi.org/project/Pillow/)
```
sudo pip3 install pillow
```

## Frameworks (Networking)

1. Install frameworks for remote procedure calls (see also https://grpc.io)
```
sudo pip install grpcio
```
 
## Microservices
