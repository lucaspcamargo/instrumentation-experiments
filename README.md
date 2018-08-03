# instrumentation-experiments

Instrumentation demos written for the Tiva TM4C123G Launchpad atop the TivaWare library, with accompanying Qt interfaces.

+ instr-encoder-tiva/ : reads a motor speed encoder and controls its speed accordingly (fw)
+ instr-sound-tiva/ : samples audio data from a microphone module and sends it up the serial port
+ instr-camera-tiva/ : operates an OV7670 camera module and relays the captured images to the computer (also via serial port)
+ qt/ : the computer GUIs for all the above applications (Qt-based)

For reference, see this [blog post](http://lpcamargo.blogspot.com.br/2016/01/microcontroller-and-instrumentation.html).
