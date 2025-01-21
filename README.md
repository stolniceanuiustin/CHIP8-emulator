__CHIP-8 Emulator__  
This is an emulator, written in C, for the CHIP-8 virtual machine, designed to faithfully replicate the behaviour of the oirginal system, allowing programrs written for it to be runned on any system.  

__Features__  
All 35 instructions of the CHIP-8 arhitecture are implemented.  
Input:  
CHIP8 uses a 16 hexadecimal keypad for user input.  
// CHIP8 keypad to querty  

1 2 3 C  |    1 2 3 4  
4 5 6 D  |   Q W E R  
7 8 9 E  |   A S D F  
A 0 B F  |   Z X C V   

Graphics: CHIP-8 supports a 64x32 monochrome display, my emulator uses that and renders pixel-based graphis as per the CHIP-8 Specifications     
Timers: implements the CHIP-8 delay and sound timers for accurate execution of programs  
Sound: Still not implemented   


__Cross-platform compatibility__   
The emulator is written in C, thus it ensures maximum portability.  


__Tehnical details__  
Platform: Cross_platform, tested on Windows and Linux   
Programming Language: C  
Dependencies: SDL2 for graphics  


__Future Improvements__  
Adding support for the SUPER-CHIP (SCHIP) extensions.  
Enhanced debugging tools, such as a step-through debugger.  
Improved graphical scaling and fullscreen options.  


__About CHIP-8__  
CHIP-8 was originally developed as an interpreted language for early microcomputers like the COSMAC VIP. 
It is still widely used as a beginner-friendly platform for emulator development and retro gaming enthusiasts.

__HOW TO COMPILE__
For Linux: ensure you have SDL2, GCC and MAKE installed in your environment.   
You can get them by running these commands:   
sudo apt-get install libsdl2-dev   
sudo apt-get install gcc   
sudo apt-get install make   
  
After that, simply go to the folder where the makefile is and write make.

