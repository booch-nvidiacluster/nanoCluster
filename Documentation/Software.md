# Software
The cluster's setup and provisioning.

<img src="/Documentation/Images/Jetson Nano.jpg">

Here are instructions for provisioning each Nano in the cluster with a basic stack - Ubuntu, Python, MySql, and Nginx - together with support for other languages, runtimes, and a graph database; various tools, libraries, and frameworks for general development, artificial intelligence, and networking; and infrastructure for hosting microservices and Kubernetes. A few of these instructions will direct you to certain actions using the Nano's graphical user interface (for example, step 3); most of the instructions will direct you to certain actions using a host terminal connected to the Nano via ssh (for example, step 4). Several bits of code needed in support of this provisioning reside <a href="../source">here</a>.

## Ubuntu
 
1. Download the Jetson Nano Developer Kit SD Card Image from https://developer.nvidia.com/embedded/jetpack to a host computer.
 
2. Write the image from the host computer to the Nano's memory card using https://www.balena.io/etcher/.
 
3. Install the memory card in the Nano, attach the Nano to a monitor, keyboard, and mouse, boot the Nano (but first ensure that its SSD is NOT yet connected to the Nano), then follow the on screen start up instructions to configure the computer. We use a computer name in the form *nanoCluster#* (where # is the node ID from 0 to 3) and the user ID *nano* (with a password of your choice). Along the way, you'll be asked for the computer's location, APP partition size (use 0 to set the maximum size), NVPModel (use the default settings), and wireless connection. It is important that you establish a static IP address for each node as a precondition to configuring Kubernetes, which is more easily done in the Nano's graphical user interface than in the terminal interface. To do so, go to

          System Settings -> Network -> *connection* -> Settings -> IPV4 Settings -> Method -> Manual<br>
          System Settings -> Network -> *connection* -> Settings -> IPV4 Settings -> Add
      
      where you will set an appropriate IP address, netmask, gateway, and DNS server. Along the way, the Nano's start up process may ask you to update your software and to reboot. (see also https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#setup-first-boot).

4. Clone this repository (see also https://www.jetsonhacks.com/2021/03/10/jetson-nano-boot-from-usb/).
```
cd /home/nano/Downloads
sudo git clone https://github.com/JetsonHacksNano/bootFromUSB
```

5. Attach the SSD to the Nano's USB port, then use the Nano's Disk application to name (in the form *nanoCluster#SSD*, where # is the node ID from 0 to 3), format, and mount the SSD.

          Disks -> Format -> Compatible with modern systems and hard drives > 2TB (GPT)<br>
          Disks -> Add Partition -> 499GB | nanoCluster#SSD | Internal disk for use with Linux only (Ext4)<br>
          Disks -> Mount

6. Copy the root file system to the SSD. Copy the SSD's PARTUUID for later use.
```
cd /home/nano/Downloads/bootFromUSB
./copyRootToUSB.sh -p /dev/sda1
./partUUID.sh
```

7. Redirect the root file system.
<pre><code>cd /media/nano/nanoCluster#SSD/boot/extlinux
sudo vi extlinux.conf
    <i>Copy the PRIMARY entry and rename it to sdcard.</i>
    <i>Change the APPEND line of the PRIMARY entry to reflect the PARTUUID for sda1.</i>
        APPEND ${cbootargs} root=PARTUUID=<i>&lt;UUID for sda1&gt;</i> rootwait rootfstype=ext4</code></pre>

8. Shut down the Nano.
```
sudo shutdown now
```

9. Remove the memory card then power up the Nano.

10. Configure the Nano for high power mode.
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

11. Make the Nano headless and remove various unnecessary applications.
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
sudo apt install python3-pip
```

2. Upgrade pip.
```
sudo -H pip3 install --upgrade pip
```
3. Make pip3 the default.
<pre><code>cd /etc
sudo vi bash.bashrc
    <i>Add the following line.</i>
        alias pip=pip3</code></pre>

4. Foobar

## Nginx

1. Install Nginx (see also https://www.nginx.com).
```
sudo apt install nginx
```

2. Confirm that Nginx is properly installed by going to a host computer and entering the Nano's IP address in a browser.

3. Replace the Nano's default web page using files from <a href="../source/nano">here</a>. To get the URL for the html, find the appropriate html (in the form *index#.html* where # is the node ID from 0 to 3) then select Raw. To get the URL for the image (in the form *nanoCluster#.jpg* where # is the node ID from 0 to 3), find the appropriate file then select Download.
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
sudo -H pip3 install py-cpuinfo
```

## Libraries (Networking)

1. Install libraries for finding available network ports (see also https://pypi.org/project/portpicker/).
```
sudo -H pip3 install portpicker
```

## Lbraries (Python)

1. Install libraries for representing Python’s abstract syntax trees (see also https://pypi.org/project/gast/).
```
sudo -H pip3 install gast==0.2.2
```

2. Install libraries for manipulating Python’s abstract syntax trees (see also https://pypi.org/project/astor/).
```
sudo -H pip3 install astor
```

3. Install libraries for constructing Python function wrappers and decorators (see also https://pypi.org/project/wrapt/).
```
sudo -H pip3 install wrapt
```

4. Install libraries for concurrency (see also https://pypi.org/project/futures3/).
```
sudo -H pip3 install futures3
```

5. Install libraries for supporting cross version Python codebases (see https://pypi.org/project/future/)
```
sudo -H pip3 install future==0.18.2
```

6. Install libraries for building Python applications (see also https://pypi.org/project/absl-py/).
```
sudo -H pip3 install absl-py
```

7. Install libraries for managing Python packages (see also https://pypi.org/project/setuptools/).
```
sudo -H pip3 install setuptools
```

8. Install libraries for testing Python applications (see also https://pypi.org/project/testresources/).
```
sudo -H pip3 install testresources
```

9. Install libraries for unit testing Python applications (see also https://pypi.org/project/unittest2/)
```
sudo -H pip3 install unittest2
```

10. Install libraries for testing Python applications (see also https://pypi.org/project/mock/).
```
sudo -H pip3 install mock==4.0.2
```

11. Install libraries for exposing C++ types in Python (see also https://pypi.org/project/pybind11/).
```
sudo -H pip3 install pybind11
```

## Libraries (Numeric)

1. Install libraries for linear algebra (see also http://math-atlas.sourceforge.net).
```
sudo apt install libatlas-base-dev
```

2. Install libraries for scientific processing (see also https://pypi.org/project/scipy/).
```
sudo -H pip3 install scipy==1.4.1
```

## Libraries (Data)

1. Install libraries for storing hierarchical data  (see also https://www.hdfgroup.org/solutions/hdf5/).
```
sudo apt install libhdf5-serial-dev hdf5-tools libhdf5-dev zlib1g-dev zip libjpeg8-dev liblapack-dev libblas-dev
```

2. Install libraries for manipulating HDF data (see also https://pypi.org/project/h5py/).
```
sudo -H pip3 install h5py==2.10.0
```

3. Install libraries for array manipulation (see also https://pypi.org/project/numpy/)
```
sudo -H pip3 install numpy==1.16.1
```

## Libraries (Visualization)

1. Install libraries for formatting terminal output (see also https://pypi.org/project/termcolor/).
```
sudo -H pip3 install termcolor
```

2. Install libraries for imaging (see also https://pypi.org/project/Pillow/).
```
sudo -H pip3 install pillow
```

## Frameworks (Networking)

1. Install frameworks for remote procedure calls (see also https://pypi.org/project/grpc/).
```
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

1. Configure the firewall using files from <a href="../source/nano">here</a>. To get the URL for the daemon, find the json then select Raw.
<pre><code>sudo apt-get install iptables-persistent
sudo iptables -P FORWARD ACCEPT
cd /etc/docker
sudo rm daemon.json
sudo wget <i>&lt;URL for daemon.json&gt;</i> -O daemon.json
sudo systemctl daemon-reload
sudo systemctl restart docker</code></pre>

2. Verify that Docker is properly installed.
```
sudo docker run hello-world
sudo docker run --gpus all jitteam/devicequery ./deviceQuery
```

3. Install FastAPI (see also https://pypi.org/project/fastapi/).
```
sudo -H pip3 install fastapi
```

4. Install an asynchronous gateway service interface (see also https://pypi.org/project/uvicorn/).
```
sudo -H pip3 install uvicorn
```

## Kubernetes

1. As a precondition to configuring the Kubernetes infrastructure, disable Nginx on the node.
```
sudo systemctl stop nginx
sudo systemctl disable nginx
```

0. Install MicroK8s on the node (see also https://microk8s.io/docs).
```
sudo snap install microk8s --classic --channel=1.18/stable
```

1. Join the microk8s group.
```
sudo usermod -a -G microk8s $USER
sudo chown -f -R $USER ~/.kube
su - $USER
```

2. Confirm the initial installation.
```
microk8s status
```

3. For convenience, make an alias to kubectl.
<pre><code>cd /etc
sudo vi bash.bashrc
    <i>Add the following line.</i>
        alias kubectl='microk8s kubectl'</code></pre>

5. Designate one node as master; designate the others as workers
<pre><code><i>On the master node (nanoCluster0), get a join token.</i>
    microk8s add-node
<i>On a woker node, join the node to the master.</i>
   microk8s join <i>&lt;directive from the previous command on the master node&gt;</i>
<i>Repeat these two commands for each worker node in the cluster.</i></code></pre>

6. On the master node, confirm that all the woker nodes are properly connected.
```
kubectl get nodes
```

7. On the master node, set each node's role.
<pre><code><i>Set the master node's role.</i>
    kubectl label node nanocluster0 node-role.kubernetes.io/master=master
<i>Set each worker node's role.</i>
    kubectl label node <i>&lt;work node's name&gt;</i> node-role.kubernetes.io/worker=worker
<i>Repeat the previous command for each worker in the cluster.</i></code></pre>

8. On the master node, enable certain services.
```
microk8s enable dashboard dns ingress
```

## Remotely Accessing the Cluster

1. On the master node, get the credentials. Save the results for later.
```
microk8s config
```

2. On another computer on the same local network as the cluster, install kubectl (here we use a Macintosh).
```
brew install kubectl
```

3. On the remote computer, install the cluster's credentials.
<pre><code>mkdir ~/.kube
cd ~/.kube
sudo vi config
    <i>Insert the cluster's credentials.</i></code></pre>

4. Confirm that the cluster is accessible from the remote computer.
```
kubectl get nodes
```

5. On the master node, create a service account using the files found <a href="../source/nano">here</a>. To get the URL for each file, find the file then select Raw.
<pre><code>cd /home/nano/Downloads
sudo wget <i>&lt;URL for dashboard.nanocluster-admin.yml&gt;</i> -O dashboard.nanocluster-admin.yml
sudo wget <i>&lt;URL for dashboard.nanocluster-admin-role.yml&gt;</i> -O dashboard.nanocluster-admin-role.yml
kubectl apply -f dashboard.nanocluster-admin.yml
kubectl apply -f dashboard.nanocluster-admin-role.yml</code></pre>

6. On the master node, get the service account's token. Copy the token for later use.
```
kubectl -n kube-system describe secret $(kubectl -n kube-system get secret | grep nanocluster-admin  | awk '{print $1}')
```

7. On the remote computer, forward the Kubernetes dashboard service port.
```
kubectl port-forward service/kubernetes-dashboard 8443:443 -n kube-system
```

8. On the remote computer, open a browser window to access the Kubernetes dashboard, using the service account's token.
```
https://localhost:8443
```

## Node/Console Integration

1. Start the Nano's hearbeat using the application found <a href="../source/nano">here</a>. To get the URL for the file, find hearbeat.py then select Raw.
<pre><code>cd /home/nano/Downloads
sudo wget <i>&lt;URL for heartbeat.py&gt;</i> -O heartbeat.py
crontab -e
    <i>Add the following line.</i>
        @reboot python /home/nano/Downloads/heartbeat.py</code></pre>

2. Reboot the nano.
```
sudo reboot now
```

## Commonly Used Cluster Commands

1. On the node, display the hostname.
```
whoami
```

2. On the node, display the IP address.
```
ifconfig
```

2. On the node, display the JetPack version.
```
sudo apt show nvidia-jetpack
```

3. On the node, display the version of a package.
```
sudo apt show <package name>
sudo pip3 show <package name>
```

4. On the node, update a package.
```
sudo apt update <package name>
sudo apt upgrade <package name>
sudo -H pip3 <package name> -- upgrade
```

5. On the node, clean up unused packages.
```
sudo apt autoremove
```

6. On the master node, check the Kubernetes status.
```
microk8s status
```

7. On the master node or the remote computer, check the Kubernetes nodes.
```
kubectl get nodes
```

8. On the master node or the remote computer, check the Kubernetes namespaces.
```
kubectl get all --all-namespaces
```
