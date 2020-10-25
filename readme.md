
# ESP32 Duktape POC

![sample Gif](/img/duktape.gif)

## Quick overview
POC to inject javascript into the IDF over MQTT

## prerequisites
You need to have the IDF installed. You can view the free section of my course to get up and running
 [https://learnesp32.com](https://learnesp32.com)

1. clone this repo: ` git clone https://github.com/Mair/idf-duktape.git`

2. ensure you have [node red](https://nodered.org/) installed https://nodered.org/docs/getting-started/

3. add an inject node as follows 

![inject](/img/inject.png)

4. set up a mqtt out node as follows

![inject](/img/mqtt_out_node.png)

5. connect the 2 nodes

6. add js / compiled typescript to the inject node payload and send it