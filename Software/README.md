Some brief info for you to make code with :)

The keyboard uses an TCA8418 IC to save i/o pins. This stores the key events in a buffer so that they can be read as needed
https://www.ti.com/lit/gpn/TCA8418

The RTC is a RV-3032-C7 which allows the date/time to be stored for a better user experience
https://www.microcrystal.com/fileadmin/Media/Products/RTC/App.Manual/RV-3032-C7_App-Manual.pdf

The screen is GDEY029T71H from GoodDisplay. It is a higher resolution 2.9" screen and still has the fast refresh
https://www.good-display.com/product/540.html

The add-on cards will have an EEPROM chip on them, to allow the storage of some information about the card, and some sample code for using it. This is still a work in progress, but will probably be M24M02-DRMN6TP
https://www.st.com/resource/en/datasheet/m24m02-dr.pdf


There will be 2 i2c ports (stemma/qwiic), the USB port, then the m.2 add-on slot

I2c 1 is on the same pins as the internal stuff for the keyboard and RTC (SDA:pin 1 - SCL:pin 2 - INT:pin 3)
I2c 2 share some of the data pins from the m.2 slot (SDA: pin 47 - SCL:pin 48)
The USB is connected directly to the ESP
The m.2 slot has basically everything connected. USB/UART/i2c/pins 8,15,16,17,18,21,35,36,37,38,39,40,41,42,47,48 (although pins 35,36,37 are tied to the PSRAM so may be useless!!)


