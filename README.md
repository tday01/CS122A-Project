# Gas Monitoring &amp; Transponder Alert System

The system is comprised of two transponder units (up to 20 possible on the same Zigbee network) that are worn by a worker either on the belt or clipped to the shirt. Transponders auto link-up when first powered on so that each transponder can be seamlessly added to the network of transponder units. When an unsafe breathing environment is detected by one transponder, all transponders sound a speech2text voiced notification as well as activate an onboard vibration motor; the worker is then advised with speech from the system to put on protective breathing gear. In addition, each transponder tracks volatile organic compound exposure throughout a work week by calculating the total-weighted-average of exposure. If weekly exposure limit is exceeded then the system triggers the alarm (>500 ppb OSHA guidelines declare unsafe in 40 hour week). All values, warnings, and alerts are displayed real-time on the unit's LCD display; all relevant values are stored in EEPROM. When a worker is wearing protective breathing gear, they can press a button to inform the device at which time all transponders are notified of the worker’s action; in this state exposure isn’t logged. Each transponder unit operates on a 9 volt battery.

##### 2 Transponder Units

![2 Units](https://github.com/tday01/CS122A-Project/blob/master/images/2%20units.jpg)

#### [Short Video Demo of Basic System Features](https://www.youtube.com/watch?v=jCFYcRYhyFw)
