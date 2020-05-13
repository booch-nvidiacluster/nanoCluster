# Construction
The nanoCluster has a small footprint, measuring 24" long and 3" wide (not counting any space taken up by power and USB cables). It is kept open to simplify cooling and to make all header pins easily accessible for experimentation.

<img src="/Documentation/Images/top.jpg" alt="Top View">
<img src="/Documentation/Images/bottom.jpg" alt="Bottom View">

Each of the four NVIDID Jetson Nanos as well as the Adafruit Grand Central are secured to a wooden base using four nylon nuts, bolts, and spaces. Eight rubber feet keep the base raised.

<img src="/Documentation/Images/bottom detail.jpg" alt="Bottom Detail View">

There are four signal connections from each Nano to the console:
* powerOn (black wire)
* heartbeat (white wire)
* serial receive (blue wire)
* serial transmit (green white)

There is one other electrical connection, daisy chained from one Nano to the other then to the console:
* ground (purple wire)

<img src="/Documentation/Images/nano.jpg" alt="Nano">
<img src="/Documentation/Images/nano detail 1.jpg" alt="Nano Detail 1">
<img src="/Documentation/Images/nano detail 2.jpg" alt="Nano Detail 2">

An SSD is positioned to the right of each Nano, using two repurposed cable clamps and then secured to the wooden base using a cable tie

<img src="/Documentation/Images/nano detail 3.jpg" alt="Nano Detail 3">

The console - consisting of the Adafruit Grand Central and a TFT Touch Shield - is positioned to the far left of the wooden base. All the cables coming from each Nano to the console are twisted to reduce crosstalk and then routed under each board. Because of the way the TFT obscures some of the Grand Central pins, it is necessary to clip four of the unused pins on the TFT and then use L-shaped male-to-male adaptors to connect serial pairs from two of the Nanos to the console.

<img src="/Documentation/Images/console.jpg" alt="Console">
<img src="/Documentation/Images/console detail 1.jpg" alt="Console Detail 1">
<img src="/Documentation/Images/console detail 2.jpg" alt="Console Detail 2">
