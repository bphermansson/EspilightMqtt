# EspilightMqtt
This is a sketch for Esp8266-based devices that act as a RF-to-Mqtt gateway. A RF-receiver is connected to the Esp's Gpio. The signals that are received are decoded by the Espilight (https://github.com/puuu/ESPiLight) library, which is based on Pilight (https://pilight.org/). The device can decode RF-devices that are supported by Pilight. 
When a valid code is received the data is published via Mqtt to the Wifi network.
