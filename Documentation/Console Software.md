# Console Software
The cluster's monitoring and control console

<img src="/Documentation/Images/console.jpg" alt="Console">

The console serves two use cases: to display the state of each node, and to permit the user to power off individual nodes. The console's display is divided into several rows, each of which represents a single node. During the lifecycle of a node, its state is display here, and once a node is logged in, node's name is displayed along with its IP addres. Below these labels are two bars: the grey one representing the node's CPU utilization, and the blue one representing the node's primary memory utilization. To the right of these lables is a power icon, which can be touched to power off a node. in the center of each power icon is a blinking dot, representing the node's heartbeat.

This is a simple cluster, composed of four NVIDIA Jetson Nano single board computers (which provide the cluster's essential computational resources) together with an AdaFruit Grand Central (which serves as the cluster's monitoring and control console). While the nanoCluster is a general-purpose edge compute platform suitable for many use cases, it was created primarly as an experimental platform for the development of <a href="https://github.com/booch-self/self">Self</a>, a hybrid neuro/symbolic architecture for AGI.

operation: dispay contents/control contens
why no classes
concept basic state machine (init cycle)

nodes
