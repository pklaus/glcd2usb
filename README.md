
### This is a fork of Till Harbaum's glcd2usb

It adds a small tool which sets the brightness of the backlight
to zero and exits. This is helpful if you want to be able to shut
the display down and save power while having it still connected
to the computer as the brightness setting is preserved when the
tool exits.

Compiling:

    cd ./testclient
    make

Usage:

    ./dimdown

