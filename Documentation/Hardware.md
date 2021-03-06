# Hardware
The nanoCluster has a small footprint, measuring 4" by 6" by 10" (not counting any space taken up by power, USB, and HDMI cables). It is kept open to simplify cooling and to make all header pins easily accessible for experimentation.

<img src="/Documentation/Images/cluster 1.jpg">

Each of the four NVIDID Jetson Nanos as well as the touch screen LCD are secured to four aluminum rails using nylon nuts, bolts, and spacers. All SSDs are positioned in the center of the cluster, secured using cable ties.

There is a jumper shunt placed on J48 off each Nano, which directs each Nano to draw its power from the barrel jack rather than the micro-USB port (see also https://www.jetsonhacks.com/2019/04/10/jetson-nano-use-more-power/). The cooling fan, anchored on top of each Nano's heatsink, is connected to J15 (see https://www.jetsonhacks.com/2019/09/08/jetson-nano-add-a-fan/). The wireless card and antenna are placed below each Nano, and are connected to M.2 Key E (see https://www.jetsonhacks.com/2019/04/08/jetson-nano-intel-wifi-and-bluetooth/).

The touch screen LCD is connected to one of the Nanos using a USB cable (for power and touch control) and an HDMI cable (for display).

<img src="/Documentation/Images/cluster 2.jpg">
<img src="/Documentation/Images/cluster 3.jpg">
<img src="/Documentation/Images/cluster 4.jpg">
