# ArduinoIBIS

⚡ Implementation of the VDV 300 IBIS Wagenbus protocol for Arduino and Arduino-core based devices 💡

## Introduction
This open-source project aims to be an *Arduino* library that maintains communication with various public transport equipment, such as screens, displays, announcement devices or ticket printers. The term **IBIS** is german and stands for "**I**ntegriertes **B**ord**I**nformations**S**ystem" (eng.: "integrated board information system"). It is defined by the VDV and it's definition as well as it's extensions are nowadays available for reading on the [VDV's knowhow page](https://knowhow.vdv.de/documents/300/).

**IBIS** was designed back in 1992, but it is still widely used to this day, in old and new installations.

## How does IBIS work?
To it's core, **IBIS** is just a plain-standard RS232/TTL serial signal, communicating with a baud rate of *1200*, *7* data bits, *equal* parity and *2* stop bits (`1200 7E2`). The difference to a standard serial signal lies in the voltage levels. While a TTL signal operates between 0V and 3.3V, **IBIS** is inverted and amplified to a *24V* level. Driving a higher voltage makes the data transmission more robust in harsh environments like trains and buses, and also allows to run longer data lines.

**IBIS** is bidirectional, meaning that devices and both send and receive data to/from the bus. Usually, the **IBIS** controller in a vehicle is located near the driver's seat and receives input from the driver or over the air, generates **IBIS** data and sends it to every piece of equipment installed. Those devices can then decide whether they want to react to the sent data, or not.

**IBIS** speaks *telegrams*. All of those telegrams are defined in the [VDV 300 document, page 41 following](https://knowhow.vdv.de/documents/300/). Each of those telegrams has a consecutive number (e.g. `001`, also often refered to as `DS001`) and a specific coding which defines the structure of the telegram's payload.

Due to the higher voltage levels (24V), connecting an **IBIS** device directly to a microcontroller is **NOT** possible. At the very least, you'll need a strong transistor that is triggered from the Arduino and switches a 24V power supply to the **IBIS** port.

> 💡 Together with this library, I've created an [open-source **IBIS** controller utilizing an ESP8266 (Wemos D1)](https://github.com/iUltimateLP/IBIS_WiFi_Controller). You can build the controller yourself, and then use this library to interface with your **IBIS** devices.

## Getting started
Once you've built the hardware side of things, the **IBIS** communication can be established with only a few lines of code:

```cpp
#include <Arduino.h>
#include <ArduinoIBIS.h>

int txPin = 12;
int rxPin = 13;
ArduinoIBIS::Port ibis;

void setup()
{
	  ibis.Begin(txPin, rxPin);
}

void loop()
{
	  ibis.DS003c("Hello World!");
	  delay(3000);
	  ibis.DS003c("I'm ArduinoIBIS!");
	  delay(3000);
	  ibis.DS003c("Nice to meet you!");
	  delay(3000);
}
```

This example begins the **IBIS** communication on the given transmit and receive pins (all pins supported by [ESPSoftwareSerial](https://github.com/plerup/espsoftwareserial) are supported here), and then periodically sends the `DS003c` telegram to the connected display.

See it in action here:

VID!

## Credits
This library wouldn't've been possible without the efforts of the following projects and people. Thanks to:
 - [plerup/ESPSoftwareSerial](https://github.com/plerup/espsoftwareserial), which is used as the serial communication library
 - [CatoLynx](https://github.com/CatoLynx), who helped me debugging my hardware and software since day one (I've also taken his awesome [pyFis](https://github.com/CatoLynx/pyFIS/blob/master/pyfis/ibis/ibis_protocol.py) protocol as a reference)
 - various public transport companies who provided me with tech throughout the years

## License
MIT License

Copyright (c) 2025 Jonathan Verbeek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
