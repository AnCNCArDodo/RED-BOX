# **RED-BOX**

**RED-BOX** is a very cheap and easy rocket data logger, that for now logs altitude and apogee. In the future im plannig on adding speed and other funcions.
for now we have the .ide file for programing in Arduino ide and a phyton app for decoding the data.
**Hardware:**
- Esp 32 c3 Super Mini. Ali link aliexpress.com/item/1005007496189056.html
- Bmp180 barometer. Ali link aliexpress.com/item/1005008727792256.html

**Wiring:**
  
<img width="679" height="623" alt="Zrzut ekranu 2025-11-30 191756" src="https://github.com/user-attachments/assets/c5c0716f-d609-4579-9b93-37dfb119ef33" />

<img width="1000" height="1000" alt="image" src="https://github.com/user-attachments/assets/0889c32f-b43e-4a20-880e-afe3aba03865" />

**Setup:**
-Wire th esp up like shown in the picture 
- Flash the esp board with the firmware
- Connect to the wifi default ssid is "RED-BOX" and passwor is "RED-BOX-admin"
- Connect to 192.168.4.1 in a browser
- Before flight click start logging and after flight click stop logging
- Open the Python data viewer and imput the file that you have downloaded from the website
- See your beautiful flight logs 

**Libraries:**
- **Arduino ide:**
  - Adafruit_BMP085
  - esp 32 in board manager
- **Phyton:**
  - tkinter
  - matplotlib
  - pandas
  - numpy

