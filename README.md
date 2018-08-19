# ArduCAM_ESP32S_UNO
ArduCAM ESP32 Series Development Boards
## Steps to install Arduino ESP32 support on Windows
### Tested on 32 and 64 bit Windows 10 machines

# ArduCAM ESP32 UNO MINI Camera Demo Tutorial
[![IMAGE ALT TEXT](https://github.com/UCTRONICS/pic/blob/master/Arducam_ESP32_Camera.jpeg)](https://youtu.be/o8jauiegWuI  "ArduCAM ESP32 UNO MINI Camera Demo Tutorial")

1. Download and install the latest Arduino IDE ```Windows Installer``` from [arduino.cc](https://www.arduino.cc/en/Main/Software)
2. Download and install Git from [git-scm.com](https://git-scm.com/download/win)
3. Start ```Git GUI``` and run through the following steps:
    - Select ```Clone Existing Repository```
    
        ![Step 1](https://github.com/zk109/test/blob/master/win-gui-1.png)
        
    - Select source and destination
        - Source Location: ```https://github.com/ArduCAM/ArduCAM_ESP32S_UNO.git```
        - Target Directory: ```C:/Users/[YOUR_USER_NAME]/Documents/Arduino/hardware/ArduCAM/ArduCAM_ESP32S_UNO```
        - Change this to your Sketchbook Location if you have a different directory listed underneath the "Sketchbook location" in Arduino preferences.
        - Click ```Clone``` to start cloning the repository
        
            ![Step 2](https://github.com/zk109/test/blob/master/win-gui-2.png)
            ![Step 3](https://github.com/zk109/test/blob/master/win-gui-3.png)
        
    - Open ```C:/Users/[YOUR_USER_NAME]/Documents/Arduino/hardware/ArduCAM/ArduCAM_ESP32S_UNO/tools``` and double-click ```get.exe```
    
        ![Step 4](https://github.com/zk109/test/blob/master/win-gui-4.png)
        
    - When ```get.exe``` finishes, you should see the following files in the directory
    
        ![Step 5](https://github.com/zk109/test/blob/master/win-gui-5.png)
        
4. Plug your ESP32 board and wait for the drivers to install (or install manually any that might be required)
