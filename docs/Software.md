# Universal Light Switch
# A Description
# of the Software
## Version 49
# Notes and definitions

* Please read the README.md which is an overview of the design<br>
* The software is written for the hardware.  Please read docs/hardware<br>
* The language is C++ and is developed in the Arduino Integrated Development Environment ver 2.2.1 (IDE)<br>
* Please see [MQTT](https://mqtt.org/) for more information.<br>
* “Publish” means to send a message to a topic on the MQTT broker.<br>
* “Subscribe” means that a message sent to a topic on the MQTT broker will be forwarded to the sending device.<br>
* “Unsubscribe” means that messages will no longer be forwarded.<br>
* All messages will be sent with [the retain flag](https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/) set. This effectively makes all of the configuration and activity data for the device non-volatile.<br>
* “ISR” means Interrupt Service Routine, in MQTT parlance, callback.

# Environment


## Integrated development Environment (IDE)

The [Arduino IDE version 2.21](https://www.arduino.cc/en/software) is used to develop the software.  The board environment, [ESP8266 by ESP8266](https://github.com/esp8266/Arduino), must be installed into the IDE for the ESP-12F.


## Programmer

A [programmer](https://www.aliexpress.us/item/3256802482050604.html) is required to download the software to the device.  The programmer referenced requires a micro USB cable.


## Useful utilities

After downloading the program into the device the first time, the device can be programmed Over-The-Air (OTA)  Once the device is built into the enclosure, OTA is the medium of choice,  If the device was constructed as explained, it could be possible to use the programmer again.

[PuTTY](https://www.putty.org/), can be used in place of the IDE’s serial monitor.  The advantage is that it is always monitoring the serial port to which the device is connected.

A terminal emulator, or command line interface (CLI) can be used to download the program.  The command that would do that is the line of white text that appears on the IDE when a program is downloaded to the device (assuming an OTA download).


## Libraries

* [EspMQTTClient](https://github.com/plapointe6/EspMQTTClient) version 1.13.3<br>
* [ESP_EEPROM](https://github.com/jwrw/ESP_EEPROM) version 2.2.0<br>
* SPI (part of the IDE)<br>
* [Ucglib](https://github.com/olikraus/ucglib) version 1.52<br>
* [XPT2046](https://github.com/PaulStoffregen/XPT2046_Touchscreen) version 1.4<br>
* [ESP_EEPROM](https://github.com/jwrw/ESP_EEPROM) version 2.2.0<br>
* Please see <span style="text-decoration:underline;">XPTCalibrate_07</span>, For storing the necessary calibration constants for the touch screen


## Variables and constants

A description of the variables and constants used is found in the code


# Program flow


## Hardware and variable initialization (Setup)

- Serial port (for debugging)<br>
- Hardware control of display backlight<br>
- The mode and font to be used by the display<br>
- Clear the display<br>
- In the zone to be used to indicate that the device is to be reset, display the word RESET in a red box<br>
- Set the host topic, derived from hostname which is provided by the hardware<br>
- On the edge of the display, display the host name<br>
- On the edge of the display, display the software version<br>
- Set calibration of touch screen from data stored in non-volatile memory<br>
- Enable the Over-The-Air program update function<br>
- Enable the Last Will of this device on the MQTT broker<br>

## On connection to network and MQTT broker

- Publish that the device is online and the version of the software<br>
- Capture the IP address of the devices and display on the edge of the display<br>
- Subscribe to the subtopic of the host topic that has the message that is the panel index of the last panel that was on the device, (eg. host topic/panel).  This subscription will always be in place so that the panel can be changed from an external source by publishing to the aforementioned topic.<br>
- The callback (ISR) for this subscription will<br>
  - Capture the last known panel index for this device<br>
  - And then use that number to subscribe to the topic of panel configurations. (eg. ULS/panel/_n_ where _n_ is the last known panel index for this device<br>
  - The callback (ISR) `acquireDataForPanel` for this subscription will be `acquireDataForPanel`


## Repeat forever (loop)



* Periodically ensure that the network and connection to the MQTT broker is good.
* Check to see if there has been a touch on the touch screen (`checkTouch`)
* If the panel data has been acquired (set in `acquireDataForPanel`), 
    * unsubscribe from the topic (see On connection to network and MQTT broker) from which the data was acquired. 
    * Note that the panel data was acquired
* If to see if there is data is to be processed for the panel (`processDataForPanel`)
* If the panel hasn’t be touched for some time, turn off the backlight


## Subroutines
### `checkTouch`

This is a really complicated routine. The goal is to determine which key has been pressed then perform the action associated with that key.  Sounds simple, but it’s not.

* If the panel is being touched:<br>
  * Deal with the backlight<br>
    * If the backlight was off
        * Update the status to the on state (future: allow for change in brightness)
        * Turn the backlight on to the the on state
    * Restart the backlight off timer
    * If touch screen was previously touched
        * If this is the first time the display was touched
            * Get the location of the touch
            * Add the location to the averaging variables
            * If the touch timer timed out
                * Display a green frame
                * Indicate that this is an official touch
    * If touch screen was not previously touched
        * Get the location of the touch
        * Initialize averaging variables
        * Indicate that the screen was previously touched
        * Start the timer to check to see if this is a good touch
* If the panel is not being touched being touched
    * Indicate that it hasn’t been initially touched
    * If the panel did get an official touch
        * Turn off the green frame
        * Average the averaging variables
        * If the touch was not in the area of the keys
            * If it was in the RESET zone
                * If the previous touch was in the RESET zone, reset the device
                * If the previous touch was not in the RESET zone indicate that the touch was in the RESET zone
        * If the touch was in the area of the keys
            * indicate that the touch was not in the RESET zone
            * calculate the key
            * Toggle the key (mindful of on state)
            * Perform the action that is associated with the key and send the change to the MQTT broker (`actOnKeyPressed`)


```


## processDataForPanel

```



* Indicate that the data was processed
* Do the following for each key
    * Indicate that it is desired to get the attributes.  
    * If the type is menu (`p`), 
        * set the topic to that which is menu plus index
    * If the type is activity (`a`), 
        * set the topic to that which is activity plus index plus label
    * If the type not is menu (`p`), nor activity (`a`),
        * Indicate that it is desired to get the attributes.
        * Set the key attributes to default (`clearKeyAttributes`)
    * If it is indicated to get the attributes
        * Set the global key variable for passing to the callback (ISR) function
        * Indicate that a key update is in process
        * Display on the display in the location of the key the key attributes are being updated (`displayUpdating`)
        * Wait for a second to ensure that the network is stable
        * Ensure that the network is stable
        * Wait for a tenth of a second
        * Subscribe to the topic formulated above with a callback that will process the data that will be coming in (`updateKeysAttributes`).  When this process is finished it will indicate that a key update is not in process
        * Wait until it is indicated that a key update is in process
            * While waiting check the network
        * Unsubscribe to the aforementioned topic
        * If the type is activity (`a`), 
            * Subscribe to the topic for this key that will monitor changes in the activity which will return the date to the callback routine (`incomingActivityChange`)
        * Clear on the display in the location of the key (`displayUpdating`)
        * If the type is not an activity (`a`), 
            * Display the key according to its ONoff value (`lightUpKey`)
            * (activity key updates will come from the aforementioned subscribe)


```


## actOnKeyPressed

```



* If the key is an activity, type `a`

        ON values are greater than zero and less than 100.  off would be 0 and invalid is 255 which would be displayed as off

    * Display the key according to its ONoff value (`lightUpKey`)
* If the key is a menu, type `p`
    * Publish the next menu to the subtopic of the host topic that has the message that is the panel index of the last panel that was on the device, (see On connection to network and MQTT broker)


```


## lightUpKey

```



* If they key is in the on state
    * Black out the box
    * Set the color from the key attributes
    * Draw box that the is one pixel smaller than the previous box
    * Set the color to black
* If they key is in the off state
    * Set the color from the key attributes
    * Make a box of this color
    * Set the color to black
    * Draw box that the is one pixel smaller than the previous box
    * Set the color from the key attributes
* Print each line in the box


```


## clearKeyAttributes

```



* Set  the attributes of the key to default


```


## displayUpdating

```



* Calculate where on the display for the key to  print
* If it is indicated to display
    * Set the color to cyan
    * Set the position of the text
    * Print the text
* If it is indicated not to display
    * Black out the key


## 	Callbacks (ISR)

The callback for messages coming from the device’s panel topic is described in On connection to network and MQTT broker


```


### acquireDataForPanel

```



* Extract the panel index from the topic
* If the incoming panel index is the same as the current panel, panel is already loaded and doesn’t need to be reloaded exit the routine
* Set the the current panel to the incoming panel
* Extract the type and index for each key

    The index will be used in getting the attribute data for the key


    (in the case of a menu key, the index will be used to point to the next panel)

    * This takes a while, so ensure the network is up to date while doing this
* Indicate that there is panel data to be processed (see: Repeat forever (loop))
* Indicate the panel data was acquired (see: see: Repeat forever (loop))


```


## updateKeysAttributes

```



* Set the key from the global key
* If the type is menu (`p`), 
    * Extract the next menu index from the message
* Extract the lines for the key label from the message
* Extract the color for the key from the message
* Indicate that the key have been updated (see `processDataForPanel`)


```


## incomingActivityChange

```



* Extract the activity index from the topic
* Determine the key using the activity to key array
* Update the key’s state
* Indicate that an activity change came in (so that the loop will process it)


# Data structures

Data structures are in [JSON](https://www.json.org/json-en.html) format.

It is very important to adhere to these structures as an invalid structure will probably cause the device to crash.  The limited memory of the device precludes rigorous data checking.  That checking could be done in a yet-to-be-developed program.


## panel

This data is the configuration for the device.

The six entries are associated with keys and are order dependent.


```
    {"Type":["t","t","t","t","t","t"],"Index":[i,i,i,i,i,i]}

```



* `t` refers to the function of the key:
    * `p` indicates a panel or menu
    * `a` indicates an activity
    * `n` indicates not associated
* `i` refers to the index to subtopic in the topic which contains the data:

Type `p` (aka menu) will be the subtopic (index) in the topic `ULS/menu/subtopic`.  The message of this topic will be how the key will be configured. See menu.

Type `a` (aka activity) will be the subtopic (index) in the topic `ULS/activity/label/subtopic`.  The message of this topic will be how the key will be configured. See activity.


## menu

menus are for configuring a key to switch to a new panel


```
    {"N":n,"L":["l","l","l","l","l","l"],"C":[r,g, b]}

```



* `n` is a number, 0-254, which is the index to the panel that will be loaded when this key is pressed.
* `l` is
    * 0-6 printable characters long
    * Each is a line on the display
    * There must be six entries, even if one is a null entry `""`,
* Color can be a choice of 16,777,215 colors, though most are impractical
    * `r` is a number 0-255 which is the intensity of the color red.
    * `g` is a number 0-255 which is the intensity of the color green.
    * `r` is a number 0-255 which is the intensity of the color blue.


## Activity

Activities are for performing actions.  This code only affords ONoff possibilities, though could lend itself to dimmable devices in yet-to-be-developed software.


```
    {"L":["l","l","l","l","l","l"],"C":[r,g, b]}

```



* `l` is
    * 0-6 printable characters long
    * Each is a line on the display
    * There must be six entries, even if one is a null entry `""`,
* Color can be a choice of 16,777,215 colors, though most are impractical
    * `r` is a number 0-255 which is the intensity of the color red.
    * `g` is a number 0-255 which is the intensity of the color green.
    * `r` is a number 0-255 which is the intensity of the color blue.
