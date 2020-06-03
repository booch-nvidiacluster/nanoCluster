# Nano Software Documentation
The cluster's essential computational resource.

<img src="/Documentation/Images/Jetson Nano.jpg" alt="Jetson Nano">

Here are instructions for provisioning each Nano in the cluster with a basic stack - Ubuntu, Python, Nginx, and MySQL - together with support for other languages, runtimes, and a graph database, plus various tools and libraries for general development, frameworks for artificial intelligence and networking, and an infrastructure for hosting microservices and Kubernetes. Instructions are given for integrating the cluster's nodes with the console, accessing the cluster from a remote computer on the same network, and for commonly used cluster commands.

Several bits of code needed in support of this provisioning reside <a href="../nano">here</a>.

## Ubuntu
 
1. Download the Jetson Nano Developer Kit SD Card Image from https://developer.nvidia.com/embedded/jetpack to a host computer.
 
2. Write the image from the host computer to the Nano's memory card using https://www.balena.io/etcher/.
 
3. Install the memory card in the Nano, attach the Nano to a monitor, keyboard, and mouse, boot the Nano (but first ensure that the SSD is NOT yet connected to the Nano), then follow the on screen start up instructions to configure the computer's name (in the form *nanoCluster#*, where # is the node ID from 0 to 3), user account (we use nano as the user ID), and wireless connection. It is important that you establish a static IP address for each node, as a precondition to configuring the Kubernetes infrastructure (and this is more easily done in the Nano's graphical user interface than in the terminal interface). The start up process may ask you to update your software (which is harmless to do) and to reboot along the way. (see also https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#setup-first-boot).

4. Clone this repository (see also https://www.jetsonhacks.com/2019/09/17/jetson-nano-run-from-usb-drive/).
```
cd /home/nano/Downloads
sudo git clone https://github.com/JetsonHacksNano/rootOnUSB
```

5. Attach the SSD to the Nano's USB port, then use the Nano's Disk application to name (in the form *nanoCluster#SSD*, where # is the node ID from 0 to 3), format, and mount the SSD.

      Disks -> Format -> Compatible with modern systems and hard drives<br>
      Disks -> Add Partition -> 500GB | nanoCluster#SSD | internal disk

6. Copy the root file system to the SSD (you can ignore most of the warnings along the way). Copy the SSD's UUID for later use.
```
cd /home/nano/Downloads/rootOnUSB
./addUSBToInitramfs.sh
./copyRootToUSB.sh -p /dev/sda1
./diskUUID.sh
```

7. Redirect the root file system.
<pre><code>cd /boot/extlinux
sudo vi extlinux.conf
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
sudo jetson_clocks
```

10. Enable SSD swapping. Normally, for reasons of performance, one should not enable swapping when using Kubernetes but here we do so to trade off performance for greater working memory (see also https://www.jetsonhacks.com/2019/04/14/jetson-nano-use-more-memory/).
```
cd /home/nano/Downloads
sudo git clone https://github.com/JetsonHacksNano/installSwapfile
cd installSwapfile
./installSwapfile.sh -s 12
```

11. Make the Nano headless and remove unnecessary applications.
```
sudo systemctl set-default multi-user.target
sudo apt-get purge libreoffice*
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
* sudo apt install python-pip
sudo apt install python3-pip
```

2. Upgrade pip.
```
* sudo -H pip install --upgrade pip
sudo -H pip3 install --upgrade pip
```
3. Make Python3 the default.
```
* TBD
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
* sudo apt install cython
sudo apt install cython3
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
* sudo -H pip install py-cpuinfo
sudo -H pip3 install py-cpuinfo
```

## Libraries (Networking)

1. Install libraries for finding available network ports (see also https://pypi.org/project/portpicker/).
```
* sudo -H pip install portpicker
sudo -H pip3 install portpicker
```

## Lbraries (Python)

1. Install libraries for representing Python’s abstract syntax trees (see also https://pypi.org/project/gast/).
```
* sudo -H pip install gast==0.2.2
sudo -H pip3 install gast==0.2.2
```

2. Install libraries for manipulating Python’s abstract syntax trees (see also https://pypi.org/project/astor/).
```
* sudo -H pip install astor
sudo -H pip3 install astor
```

3. Install libraries for constructing Python function wrappers and decorators (see also https://pypi.org/project/wrapt/).
```
* sudo -H pip install wrapt
sudo -H pip3 install wrapt
```

4. Install libraries for concurrency (see also https://pypi.org/project/futures3/).
```
sudo -H pip3 install futures3
```

5. Install libraries for supporting cross version Python codebases (see https://pypi.org/project/future/)
```
* sudo -H pip install future==0.18.2
sudo -H pip3 install future==0.18.2
```

6. Install libraries for building Python applications (see also https://pypi.org/project/absl-py/).
```
* sudo -H pip install absl-py
sudo -H pip3 install absl-py
```

7. Install libraries for managing Python packages (see also https://pypi.org/project/setuptools/).
```
* sudo -H pip install setuptools
sudo -H pip3 install setuptools
```

8. Install libraries for testing Python applications (see also https://pypi.org/project/testresources/).
```
* sudo -H pip install testresources
sudo -H pip3 install testresources
```

9. Install libraries for unit testing Python applications (see also https://pypi.org/project/unittest2/)
```
* sudo -H pip install unittest2
sudo -H pip3 install unittest2
```

10. Install libraries for testing Python applications (see also https://pypi.org/project/mock/).
```
* sudo -H pip install mock==4.0.2
sudo -H pip3 install mock==4.0.2
```

11. Install libraries for exposing C++ types in Python (see also https://pypi.org/project/pybind11/).
```
* sudo -H pip install pyind11
sudo -H pip3 install pybind11
```

## Libraries (Numeric)

1. Install libraries for linear algebra (see also http://math-atlas.sourceforge.net).
```
sudo apt install libatlas-base-dev
```

## Libraries (Data)

1. Install libraries for storing hierarchical data  (see also https://www.hdfgroup.org/solutions/hdf5/).
```
sudo apt install libhdf5-serial-dev hdf5-tools libhdf5-dev zlib1g-dev zip libjpeg8-dev liblapack-dev libblas-dev
```

2. Install libraries for manipulating HDF data (see also https://pypi.org/project/h5py/).
```
* sudo -H pip install h5py==2.10.0
sudo -H pip3 install h5py==2.10.0
```

3. Install libraries for array manipulation (see also https://pypi.org/project/numpy/)
```
* sudo -H pip install numpy=1.16.1
sudo -H pip3 install numpy==1.16.1
```

## Libraries (Visualization)

1. Install libraries for formatting terminal output (see also https://pypi.org/project/termcolor/).
```
* sudo -H pip install termcolor
sudo -H pip3 install termcolor
```

2. Install libraries for imaging (see also https://pypi.org/project/Pillow/).
```
* sudo -H pip install pillow
sudo -H pip3 install pillow
```

## Frameworks (Networking)

1. Install frameworks for remote procedure calls (see also https://pypi.org/project/grpc/).
```
* sudo -H pip install grpcio
sudo -H pip3 install grpcio
```
 
## Frameworks (Artificial Intelligence)

1. Install Keras preprocessing (https://pypi.org/project/Keras-Preprocessing/).
```
sudo -H pip3 install keras-preprocessing==1.1.2
```

2. Install Keras applications (https://pypi.org/project/Keras-Applications/).
```
sudo -H pip3 install keras-applications==1.0.8
```

3. Install TensorFlow (see also https://pypi.org/project/tensorflow/).
```
sudo -H pip3 install --no-cache-dir --pre --extra-index-url https://developer.download.nvidia.com/compute/redist/jp/v44 tensorflow
```

4. Install Keras (see also https://pypi.org/project/Keras/).
```
sudo -H pip3 install keras
```

## Microservices

1. Verify that Docker is properly installed.
```
sudo docker run hello-world
sudo docker run --gpus all jitteam/devicequery ./deviceQuery
```

2. Install FastAPI (see also https://pypi.org/project/fastapi/).
```
sudo -H pip3 install fastapi
```

3. Install an asynchronous gateway service interface (see also https://pypi.org/project/uvicorn/).
```
sudo -H pip3 install uvicorn
```

## Kubernetes

1. Install K3S on nanoCluster0 as the master node (see also https://k3s.io and https://rancher.com/docs/k3s/latest/en/).
```
sudo curl -sfL https://get.k3s.io | INSTALL_K3S_EXEC="--docker" sh -s - --bind-address <nanoCluster0's IP address>
```

2. On the master node (nanoCluster0) get the node's token. Copy the token for later use.
```
sudo cat /var/lib/rancher/k3s/server/node-token
```

3. Install K3S on nanoCluster1, nanoCluster2, and nanoCluster3 as worker nodes.
```
sudo curl -sfL https://get.k3s.io | INSTALL_K3S_EXEC="--docker" K3S_URL=https://<nanoCluster0's IP address>:6443 K3S_TOKEN=<nanoCluster0's token> sh -
```

4. On the master node (nanoCluster0), set each worker node's role.
```
sudo k3s kubectl label node nanoCluster1 node-role.kubernetes.io/worker=worker
sudo k3s kubectl label node nanoCluster2 node-role.kubernetes.io/worker=worker
sudo k3s kubectl label node nanoCluster3 node-role.kubernetes.io/worker=worker
```

5. On the master node (nanoCluster0), install the Kubernetes Dashboard.
```
GITHUB_URL=https://github.com/kubernetes/dashboard/releases
VERSION_KUBE_DASHBOARD=$(curl -w '%{url_effective}' -I -L -s -S ${GITHUB_URL}/latest -o /dev/null | sed -e 's|.*/||')
sudo k3s kubectl create -f https://raw.githubusercontent.com/kubernetes/dashboard/${VERSION_KUBE_DASHBOARD}/aio/deploy/recommended.yaml
```

6. On the master node (nanoCluster0), create an admin account and set its role using the files found <a href="../nano">here</a>. To get the URL for each file, choose a file then select Raw.
```
cd /home/nano/Downloads
sudo wget <URL for dashboard.nanocluster-admin.yml> -O dashboard.nanocluster-admin.yml
sudo wget <URL for dashboard.nanocluster-admin-role.yml> -O dashboard.nanocluster-admin-role.yml
```

7. On the master node (nanoCluster0), deploy the admin configuration.
```
sudo k3s kubectl apply -f dashboard.nanocluster-admin.yml
sudo k3s kubectl apply -f dashboard.nanocluster-admin-role.yml
```

8. On the master node (nanoCluster0), get the admin's token. Copy the token for later use.
```
sudo k3s kubectl -n kubernetes-dashboard describe secret nanocluster-admin-token | grep ^token
```

9. On the master node (nanoCluster0), get the node's credentials. Copy the credentials for later use.
```
sudo cat /etc/rancher/k3s/k3s.yaml
```

## Node/Console Integration

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

## Remote Access

1. To access the cluster's Kubernetes Dashboard from another computer on the same local network, first install kubectl on that remote computer; here we are using a Macintosh (see also https://kubernetes.io/docs/tasks/access-application-cluster/web-ui-dashboard/).
```
brew install kubectl
mkdir ~/.kube
cd ~/.kube
```

2. Copy the master node's (nanoCluster0) credentials to the remote computer.
<pre><code>vi config
<i>Insert the master node's credentials.</i></code></pre>

3. On the master node (nanoCluster0), start a network proxy.
```
sudo k3s kubectl proxy --address='0.0.0.0' --disable-filter=true
```

4. On the remote computer, open a browser, navigate to the cluster's Kubernetes Dashboard, and use the master node's (nanoCluster0) token to login.
```
http://nanocluster0.local:8001/api/v1/namespaces/kubernetes-dashboard/services/https:kubernetes-dashboard:/proxy/#/login
```

5. On the remote computer, install a Kubernetes package manager (see also https://helm.sh).
```
brew install helm
helm repo add stable https://kubernetes-charts.storage.googleapis.com
helm repo update
```

## Commonly Used Cluster Commands

1. Display the node's hostname.
```
whoami
```

2. Display the node's IP address.
```
ifconfig
```

2. Display the node's JetPack version.
```
sudo apt show nvidia-jetpack
```

3. Display the version of a package on the node.
```
sudo apt show <package name>
* sudo pip show <package name>
sudo pip3 show <package name>
```

4. Update a package on the node.
```
sudo apt update <package name>
sudo apt upgrade <package name>
* sudo -H pip <package name> --upgrade
sudo -H pip3 <package name> -- upgrade
```

5. Clean up unusued packages on the node.
```
sudo apt autoremove
```
