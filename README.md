# SmarterLoad
Smart load control for Sunsynk/Deye inverter

## Hardware
I can not say exactly which hardware I used. Everything was pulled from Aliexpress grab bags accumulated over the years.

The Arduino boards I used where Nanos (but any of the AVR based boards should work just as well). It is important to load the optiboot bootloader on the boards, as the stock bootloader does not work well with lowe power modes and watchdogs.  I loaded optiboot by installing the MiniCore package, selecting the correct board, and selecting 'Burn Bootloader' in the standard Arduino IDE.

Arduino boards must be powered from a 3.3V power supply of your choice.

NRF24L01 boards are connected as per any of the howtos on the web (these also show how to wire up 3.3V power supplies). Make sure the pin assignments match the defines at the top of each file.

For the receiver, I am not going to discuss hacking the timer...  If you are not capable of reverse engineering it yourself, then you should not be opening one up in the first place. (NB - potentially lethal voltages inside, even when disconected!)

If you are not using a timer, any stand-alone 3.3V PSU + suitably rated relay can be used.

