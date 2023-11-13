# <center>Universal Light Switch</center>


## <center>Overview</center>


# Abstract

The Universal Light Switch (ULS) is a component of a larger system, one that would consist of a controller which employs Message Queuing Telemetry Transport, [MQTT](https://mqtt.org/).  It is initially intended for a home automation system, though it could be used in other applications. 

The concept behind the ULS is a switch that would provide control over any device that is controllable and can be placed anywhere there is power.

This device is low cost, 20% of the cost of most comparable devices and 50% the cost of a similar product. As presented here it is a do-it-yourself project.

While it relies on WiFi, it is not cloud based.

The design is for a location where a typical light switch be.  That is, it fits into a single gang box.  It requires power, 86 - 270 VAC.

Presented on a touch screen will be six zones on which a person can touch.  The action that is taken is dependent on the home automation system.  That is, the change will be reflected on a data flow controller (MQTT) which will forward the change to the home automation system.  The home automation system will decide the action based upon this information.

As currently designed, the ULS does not provide physical control of any device.  That is, no relay nor dimmer.  It is assumed that the device to be controlled has the necessary electronics.  Eg. For an LED light, a WiFi controlled pulse width modulator.

The system, of which the ULS is a part, would consist of a controller which employs Message Queuing Telemetry Transport, [MQTT](https://mqtt.org/).  This controller would control the system by subscribing and publishing to a broker.   In the case of a home automation system it could be [Home Assistant](https://www.home-assistant.io/).  The ULS would provide ON/OFF information via the MQTT broker.  It is envisioned that the ULS could be upgraded to provide other information such as scale, a number 0-100.


# Reason

There are two things wrong with most home automation devices.  First, they’re very expensive.  One rationale is that a lot of engineering goes into making them.  This is probably true for the first Insteon devices, but the company fell far short of its potential when it did not support the hardware with quality software.  Today there are a plethora of companies that offer home automation devices.  Most are designed with maximum profit in mind, not good customer experience.

Secondly, in order to garner maximum profit, they collect data on the end user.  This data is very valuable and it is sold to the maximum number of buyers.  I’m referring to cloud based devices, the ones that require a phone app to be functional.  

A second part of cloud based devices is that at some point in the future, when one is significantly invested in the technology, that the manufacturer will require a subscription fee to continue using the devices.  The flip side of this is that the manufacturer finds the product no longer profitable and ceases to support it in any way or the manufacturer goes out of business. Either way, all the devices become useless.  Unless another company buys the technology, this new company probably won’t take on the liability of privacy.  _Lastpass_ was just such a company, whose support was legendary but now is non-existent.

This is your house.  It will be around for a hundred and more years!  One would expect the same for all of the devices in it.

Cost, privacy, support.


# Features



* 6 touch buttons
* Program button function through a home automation system
    * Function of button
    * Label of button
    * Color of button
* Button Functions
    * On/Off
    * Dimmer (to be developed)
    * Menu for another group of devices to be controlled (home automation system function)
* Reset via touchpad
* Time and temperature when idle (to be developed)


# Pros and Cons


## Pros:



* Approximately $10 (3D printed enclosure not included)
* Private as the network to which it is attached
* Support on which one can rely: oneself
* The computer that runs the home automation system can be an old laptop or refurbished computer (less than $100)


## Cons:



* DIY: requires tools, time and patience
* Not UL approved
* No fuse (fuse is more expensive than the components)
* Home automation (eg. Home Assistant) is required along with an MQTT broker.
    * Home automation software can be daunting, especially one that is open source
    * The software should run on a dedicated computer running linux (Windows reboots itself too often)
    * Computer has to be on all the time
* Devices to be controlled (eg. lights) require a module in the light fixture, like the yet-to-be-designed “ULS controlled module” or a Sonoff/Tasmota D1 module.
* The life of the product is unknown.
* The long term availability of parts is questionable
* End user has to 3D print the enclosure
* Until a printed circuit board is designed, connections are wire wrapped
