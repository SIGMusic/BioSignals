# PurrData Files

## Usage:

This patch accepts space separated values from an Arduino connected to port 3 (by default).
```
12 135 2 4
31 52 1 4
12 52 2 3
...
```
would mean sensor 1 is sending the values "12, 31, 12", sensor 2 is sending "135, 52, 52", sensor 3 is sending "2, 1, 2" and sensor 4 is sending "4, 4, 3".

Use this patch inside another patch. The patch has 6 outlets, corresponding to each sensor in order.

The inlet accepts the port number to be used. To see the list of ports, make a patch and put `[devices( -- [comport 0 9600]` and then send the `[devices(` message. The list of available ports will be printed in the PurrData console thingy.
