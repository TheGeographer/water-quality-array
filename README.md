# Water Quality Array
This repostity contains the instructions for building the water quality array along with Arduino and Android code for University of Wisconsin-Whitewater water quality array used in the "Testing the Waters" event in May 2016 in cooperation with the Rock River Coalition.

## Device
The device consists of off-the-self components connected together within a water-resistant case. The bill of materials includes:
* SparkFun Red Board, https://www.sparkfun.com/products/13975 (same as Arduino Uno)
* Temperature sensor, DS18B20, https://www.sparkfun.com/products/11050
* Tentacle Shield for Atlas Scientific Probes (with electronic isolation), https://www.atlas-scientific.com/product_pages/components/tentacle-shield.html
* Atlas Scientific pH probe kit, https://www.atlas-scientific.com/product_pages/kits/ph-kit.html
* Atlas Scientific conductivity probe kit (K 0.1 for fresh water), https://www.atlas-scientific.com/product_pages/kits/ec_k0_1_kit.html
* Atlas Scientific dissolved oxygen probe kit, https://www.atlas-scientific.com/product_pages/kits/do_kit.html
* Bluetooth HC-06 module, https://www.amazon.com/Pass-Through-Communication-Compatible-Atomic-Market/dp/B00TNOO438
* AA (x6) battery pack, https://www.amazon.com/Philmore-Battery-Holder-Standard-Connector/dp/B000LFVFU8
* Small breadboard, https://www.sparkfun.com/products/12046
* Breadboard jumper wires, https://www.sparkfun.com/products/1102
* 9V to barrel jack adapter, https://www.sparkfun.com/products/9518
* 4.7K ohm resistor, from https://www.sparkfun.com/products/10969

The image below shows how the components area assembled within the water-resistance housing:

![Assembled array]https://raw.githubusercontent.com/TheGeographer/water-quality-array/master/Assembled_Array.jpg

Assemble as follows:
1. Connect the Tentacle Shield to the Arduino Uno/RedBoard and install all Atlas Scientific "stamp" controllers to the Tentacle Shield.
2. Set the jumpers on the Tentacle Shield to I2C mode.
3. Add jumper wires from 5V and GND pins on the Tentacle Shield to two separate rows on the mini breadboard. This will provide power to the temperature sensor and the Bluetooth adapter.
4. Connect the temperature sensor's power and ground wires to the appropriate power rows in the mini breadboard.
5. Add the 4.7k ohm resistor from the 5V breadboard row to a new row. This will be the "pull-up" resistor for the OneWire temperature sensor (read more at https://cdn-learn.adafruit.com/downloads/pdf/adafruits-raspberry-pi-lesson-11-ds18b20-temperature-sensing.pdf)
6. Connect the data cable from the temperature probe to the new pull-up resistor row (usually the yellow wire).
7. Connect a jumper wire from this row to pin 7 on the RedBoard.
8. Connect jumper wires from the mini bread board 5V power and GND rows to the power and ground on the Bluetooth adaptor.
9. Connect the RXD on the Bluetooth adaptor to pin 6 on the RedBoard.
10. Connect the TXD on the Bluetooth adaptor to pin 5 on the RedBoard.

You're now ready to upload the Arduino code below to the device and initiate testing.


## Arduino code
The Arduino code was built using the Arduino IDE (version 1.6.8, available at https://www.arduino.cc/en/Main/Software) and requires the following libraries (all available in the standard Arduino IDE repository):
1. OneWire
2. DallasTemperature
3. Wire
4. SoftwareSerial

## Android apps
The Android apps were built using MIT App Inventor 2 Beta (nb155 Component Release, available at http://ai2.appinventor.mit.edu) and the exported .aia are available here. To replicate, use the "Import" functionality within App Inventor to import these projects.

