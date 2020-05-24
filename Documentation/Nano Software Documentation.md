# Nano Software Documentation
The cluster's essential computational resource.

<img src="/Documentation/Images/Jetson Nano.jpg" alt="Jetson Nano">

Here are instructions for provisioning each Nano in the cluster with a basic stack - Ubuntu, Python, Nginx, and MySQL - together with various tools and libraries for general development, frameworks for artificial intelligence and networking, and an infrastructure for hosting microservices.

Several bits of code needed in support of this provisioning reside <a href="../nano">here</a>.

## Ubuntu
 
1. Download the Jetson Nano Developer Kit SD Card Image from https://developer.nvidia.com/embedded/jetpack to a host computer.
 
2. Write the image from the host computer to the Nano's memory card using https://www.balena.io/etcher/.
 
3. Install the memory card in the Nano, attach the Nano to a monitor, keyboard, and mouse, boot the Nano (but first ensure that the SSD is NOT yet connected to the Nano), then follow the on screen start up instructions to configure the computer's name (in the form *nanoCluster#*, where # is the node ID from 0 to 3), user account (we use nano as the user ID, and wireless connection. The start up process may ask you to update your software (which is harmless to do) and to reboot along the way. (see also https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#setup-first-boot).
 
4. Use the Nano's Terminal application to clone this repository (see also https://www.jetsonhacks.com/2019/09/17/jetson-nano-run-from-usb-drive/).
```
cd /home/nano/Downloads
sudo git clone https://github.com/jetsonHacksNano/rootOnUSB
```

5. Attach the SSD to the Nano's USB port, then use the Nano's Disk application to name (in the form *nanoCluster#SSD*, where # is the node ID from 0 to 3), format, and mount the SSD.

      Disks -> Format -> Compatible with modern systems and hard drives<br>
      Disks -> Add Partition -> 500GB | nanoCluster#SSD | internal disk

6. Copy the root file system to the SSD (you can ignore most of the warnings along the way). Write down the SSD's UUID for later use.
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
        APPEND ${cbootargs} root=UUID=<i>&lt;UUID for sda1&gt;</i> rootwait rootfstype=ext4</code></pre>

8. Reboot the Nano.
```
sudo reboot now
```

9. Configure the Nano for high power mode.
```
sudo nvpmodel -m 0
```

10. Enable SSD swapping (see also https://www.jetsonhacks.com/2019/04/14/jetson-nano-use-more-memory/).
```
cd /home/nano/Downloads
sudo git clone https://github.com/JetsonHacksNano/installSwapfile
cd installSwapfile
./installSwapfile.sh -s 12
```

11. Make the Nano headless.
```
sudo systemctl set-default multi-user.target
```

12. Refresh the installation.
```
sudo apt update
sudo apt upgrade
sudo apt autoremove
```

13. Reboot the Nano.
```
sudo reboot now
```

## Python

1. Install the latest Python package installers (see also https://pypi.org/project/pip/).
```
sudo apt install python-pip
sudo apt install python3-pip
```

## Nginx

1. Install Nginx (see also https://www.nginx.com).
```
sudo apt install nginx
```

2. Confirm that Nginx is properly installed by going to a host computer and entering the Nano's IP address in a browser.

3. Replace the Nano's default web page using files from <a href="../nano">here</a>. To get the URL for the html, find the appropriate html (in the form *index#.html* where # is the node ID from 0 to 3) then select Raw. To get the URL for the image (in the form *nanoCluster#.jpg* where # is the node ID from 0 to 3), find the appropriate file then select Download.
<pre><code>cd /var/www/html
sudo rm index.nginx-debian.html
sudo wget <i>&lt;URL for index#.html&gt;</i> -O index.html
sudo wget <i>&lt;URL for nanoCluster#.jpg&gt;</i> -O nanoCluster#.jpg</code></pre>

3. Confirm that these changes are properly installed by going to a host computer and entering the Nano's IP address in a browser.
 
## MySQL

1. Install MySQL (see also https://ubuntu.com/server/docs/databases-mysql).
```
sudo apt install mysql-server
```

2. Confirm that MySQL is running correctly.
```
sudo ss -tap | grep mysql
```

## Languages

1. Install Cython (see also https://cython.org).
```
sudo apt install cython
```

2. Install Fortran (see also https://gcc.gnu.org/wiki/GFortran).
```
sudo apt install gfortran
```

## Java Runtimes

1. Install Node.js (see also https://nodejs.org/en/about/).
```
sudo apt install nodejs
sudo apt install npm
```

2. Install the Java Runtime Environment (see also https://www.java.com/en/).
```
sudo apt install default-jre default-jre-headless
```

## Graph Database

1. Install Neo4j (see also https://neo4j.com).
```
wget -O - https://debian.neo4j.com/neotechnology.gpg.key | sudo apt-key add -
echo 'deb https://debian.neo4j.com stable latest' | sudo tee /etc/apt/sources.list.d/neo4j.list
sudo apt-get update
sudo apt-get install neo4j
```

## Tools

1. Install tools for transferring data with URLS (see also https://curl.haxx.se).
```
sudo apt install curl
```

2. Install tools for reporting on system loads (see also http://sebastien.godard.pagesperso-orange.fr).
```
sudo apt install sysstat
```

## Libraries (Hardware)

1. Install libraries for querying CPU information (see also https://pypi.org/project/py-cpuinfo/).
```
sudo -H pip install py-cpuinfo
sudo -H pip3 install py-cpuinfo
```

## Libraries (Networking)

1. Install libraries for finding available network ports (see also https://pypi.org/project/portpicker/).
```
sudo -H pip install portpicker
sudo -H pip3 install portpicker
```

## Lbraries (Python)

1. Install libraries for representing Python’s abstract syntax trees (see also https://pypi.org/project/gast/).
```
sudo -H pip install gast
sudo -H pip3 install gast
```

2. Install libraries for manipulating Python’s abstract syntax trees (see also https://pypi.org/project/astor/).
```
sudo -H pip install astor
sudo -H pip3 install astor
```

3. Install libraries for declaring Python enumerations (see also https://pypi.org/project/enum34/).
```
sudo -H pip install enum34
sudo -H pip3 install enum34
```

4. Install libraries for constructing Python function wrappers and decorators (see also https://pypi.org/project/wrapt/).
```
sudo -H pip install wrapt
sudo -H pip3 install wrapt
```

5. Install libraries for concurrency (see also https://pypi.org/project/futures3/).
```
sudo -H pip3 install futures3
```

6. Install libraries for supporting cross version Python codebases (see https://pypi.org/project/future/)
```
sudo -H pip install future
sudo -H pip3 install future
```

7. Install libraries for building Python applications (see also https://pypi.org/project/absl-py/).
```
sudo -H pip install absl-py
sudo -H pip3 install absl-py
```

8. Install libraries for managing Python packages (see also https://pypi.org/project/setuptools/).
```
sudo -H pip install setuptools
sudo -H pip3 install setuptools
```

8. Install libraries for testing Python applications (see also https://pypi.org/project/testresources/).
```
sudo -H pip install testresources
sudo -H pip3 install testresources
```

9. Install libraries for unit testing Python applications (see https://pypi.org/project/unittest2/)
```
sudo -H pip install unittest2
sudo -H pip3 install unittest2
```

## Libraries (Numeric)

1. Install libraries for linear algebra (see also http://math-atlas.sourceforge.net)
```
sudo apt install libatlas-base-dev
```

2. Install libraries for scientific computing (see also https://numpy.org)
```
sudo -H pip install numpy
sudo -H pip3 install numpy
```

3. Install libraries for mathematical, scientific, and engineering computing (see also https://www.scipy.org)
```
sudo apt install python3-scipy
```

## Libraries (Data)

1. Install libraries for storing hierarchical data  (see also https://www.hdfgroup.org/solutions/hdf5/).
```
sudo apt install libhdf5-serial-dev hdf5-tools libhdf5-dev zlib1g-dev zip libjpeg8-dev liblapack-dev libblas-dev
```

2. Install libraries for manipulating HDF data (see also https://www.h5py.org).
```
sudo -H pip install h5py
sudo -H pip3 install h5py
```

## Libraries (Visualization)

1. Install libraries for formatting terminal output (see also https://pypi.org/project/termcolor/).
```
sudo -H pip install termcolor
sudo -H pip3 install termcolor
```

2. Install libraries for imaging (see also https://pypi.org/project/Pillow/).
```
sudo -H pip install pillow
sudo -H pip3 install pillow
```

## Frameworks (Artificial Intelligence)

1. Install TensorFlow (see also https://www.tensorflow.org and https://docs.nvidia.com/deeplearning/frameworks/pdf/Install-TensorFlow-Jetson-Platform.pdf).
```
TBD
```

## Frameworks (Networking)

1. Install frameworks for remote procedure calls (see also https://grpc.io).
```
sudo -H pip install grpcio
sudo -H pip3 install grpcio
```
 
## Microservices

## Console Integration

1. Start the Nano's hearbeat using the application found <a href="../nano">here</a>. To get the URL for the file, find hearbeat.py then select Raw.
<pre><code>cd /home/nano/Downloads
sudo wget <i>&lt;URL for heartbeat.py&gt;</i> -O heartbeat.py
crontab -e
    <i>Add the following line.</i>
        @reboot python /home/nano/Downloads/heartbeat.py</code></pre>

2. Reboot the nano.
```
sudo reboot now
```
