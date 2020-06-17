# Console Software Documentation
The cluster's monitoring and control console.

<img src="/Documentation/Images/console.jpg" alt="Console">

The console's software is targeted to an Adafruit Grand Central, to which is attached a TFT Touch Shield. The console serves two use cases: to display the state of each node in the cluster and to permit the user to power off individual nodes. The console's display is divided into several rows, each of which represents a single node. During the lifecycle of a node, its state is displayed here. Once a node is logged in, the node's name is displayed along with its IP address. Below these labels are two bars, the gray one representing the node's CPU utilization and the blue one representing the node's primary memory utilization. To the right of these labels is a power icon which can be touched to power off a node. In the center of each power icon is a blinking dot, mirroring the node's heartbeat.

The console's source code resides <a href="../source/console">here</a>.

<img src="/Documentation/Images/Grand Central.jpg" alt="Grand Central">
<img src="/Documentation/Images/TFT Shield.jpg" alt="TFT Shield">

The console's software is conceptually simple. There are abstractions for each of the console's major components: an individual node, a serial communication channel for each node, a node's general purpose IO connections, the display, and the touchscreen. Because these are each just singletons, they are implemented not as classes but as simple data structures. Each node goes through a lifecycle - from powered off to powering on to powered on to booting to booted to logging in to logged in - and once logged in, its CPU and memory utilization are queried regularly according to its heartbeat, until it is powered off again by user command.
