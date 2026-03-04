## **AudioStage – EchoPyramid Audio Demo**

WAV Playback + Flash Recording + Beat RGB Effects Demo
Platform: **M5AtomS3R (EchoPyramid)**
IDE: **Arduino IDE 2.3.5**

### Arduino IDE Configuration

#### Board Settings

In **Arduino IDE**, configure the board as follows:

```
Board: M5AtomS3R
Flash Size: 8MB (64Mb)
Partition Scheme: Custom
PSRAM: OPI PSRAM
Upload Speed: 921600
USB Mode: Hardware CDC and JTAG
```

⚠ Make sure:

```
Partition Scheme: Custom
```

#### Replace the Default Partition Table (Important)

Since this project uses the **M5Stack** customized ESP32 core,a local partitions.csv inside the sketch folder will NOT be used automatically.

You must manually replace the partition file inside the core directory.

**Partition File Location**

Navigate to:

```
C:\Users\<your_username>\AppData\Local\Arduino15\packages\m5stack\hardware\esp32\3.2.6\tools\partitions
```

Locate:

```
default_8MB.csv
```

Replace Its Content With:
```
# Name,   Type, SubType, Offset,   Size,Flags

nvs,      data, nvs,     0x9000,   0x5000
otadata,  data, ota,     0xE000,   0x2000
app0,     app,  ota_0,   0x10000,  0x290000
spiffs,   data, spiffs,  0x2A0000, 0x400000
audio,    data, 0x99,    0x700000, 0x100000
```

**Partition Explanation**
       Name	Purpose

- spiffs	WAV file storage
- audio	Raw PCM recording storage
- app0	Main firmware
- nvs	System parameters

**Important Notes**

- Close Arduino IDE before editing

- Restart Arduino IDE after modification

- If permission is denied, run as Administrator


#### Install SPIFFS Upload Plugin

You need the Arduino IDE 2.x SPIFFS uploader plugin:

👉[arduino-spiffs-upload](https://github.com/espx-cz/arduino-spiffs-upload)

**Installation Steps**

1、Download the .vsix file from the repository

2、Place it in:

```
C:\Users\<your_username>\.arduinoIDE\plugins
```

(Create the plugins folder if it does not exist.)

3、Restart Arduino IDE

#### Upload SPIFFS Filesystem

**Step 1 – Create data Folder**

Inside your project directory:

```
AudioStage/
 ├── AudioStage.ino
 └── data/
       └── file_name.wav
```

**Step 2 – Upload SPIFFS**

In Arduino IDE:

```
Ctrl + Shift + P
```

Type:

```
spiffs
```

Select:

```
Upload SPIFFS to Pico/ESP8266/ESP32
```

The plugin will build a 4MB SPIFFS image and flash it to the device.

### PlatformIO

PlatformIO handles SPIFFS automatically.

------

#### Step 1 – Project Structure

```
AudioStage/
 ├── src/
 │    └── main.cpp
 └── data/
      └── file_name.wav
```

Just place your WAV files inside the `data` folder.

#### Step 2 – Build Filesystem Image

In PlatformIO:

```
PlatformIO → Build Filesystem Image
```

------

#### Step 3 – Upload Filesystem Image

```
PlatformIO → Upload Filesystem Image
```

That’s it ✅

No manual partition replacement required if your `platformio.ini` already defines the partition table.

###  ESP-IDF Workflow

Under **ESP-IDF**, the process is even cleaner.

------

#### Step 1 – Create SPIFFS Folder

In your project root:

```
Eoch-Pyramid/
 ├── main/
 ├── spiffs/
 │     └── file_name.wav
 └── CMakeLists.txt
```

Put your audio files inside the `spiffs` directory.

------

#### Step 2 – Modify CMakeLists.txt

Add:

```
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Eoch-Pyramid)

spiffs_create_partition_image(storage spiffs FLASH_IN_PROJECT)
```

Explanation:

- `storage` → must match partition name in partition table
- `spiffs` → folder name
- `FLASH_IN_PROJECT` → auto-flash during build

------

#### Step 3 – Build and Flash

```
idf.py build
idf.py flash
```

The SPIFFS image will automatically:

- Build
- Be packed
- Be flashed with firmware

No extra tools needed.