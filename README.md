# Obamium, the Remote Start from Scratch

[View the full post on my website](https://www.nqind.com/index.php/project-showcases/20-obamium-the-remote-start-from-scratch-2019)

## Why is it named obamium?

My car is named obamacar. Come see its [instagram page](https://www.instagram.com/2008_lexus_rx350_awd/)

### Info about files

The JSON board files are made with EasyEDA and were exported from EasyEDA. Unfortunately, I am not a KiCad user (yet) so I don't know whether or not you can convert to KiCad reliably. Go ahead and try.

If you wish to just view the schematics alone to use in your own designs, feel free to open the PDF versions. The code is all Arduino sketches, so it shouldn't be hard to understand. Note that we switched to the Arduino Nano Every in Obamium v4, meaning it may use ATmega4809-specific features instead of the tried-and-true ATmega328p

Versions 1 and 2 share the same codebase, while version 3 has no code because it never worked :( However, version 4 got a complete rewrite with a somewhat-better version, using interrupts and states.
