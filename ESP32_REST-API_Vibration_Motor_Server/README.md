# REST API for Glove Using ESP32

## Code Upload Steps

1. Connect the glove with a micro USB cable to a laptop.

2. Find out the Serial port of the glove microcontroller.

  ```bash
    # bash
    ls /dev/cu.*
  ```

2. Change the glove port name in **platformio.ini**

  ```ini
    # TODO: change the port name
    upload_port = /dev/cu.usbserial-0231BA1F
    monitor_port = /dev/cu.usbserial-0231BA1F
    test_port = /dev/cu.usbserial-0231BA1F
  ```

3. (Optional) Change the WiFi network for the glove in **http_functions.cpp**

  ```cpp
    #define WIFI_SSID "YOUR (ROUTER) NETWORK"
    #define WIFI_PSK  "YOUR (ROUTER) PASSWORD"
  ```

4. Uncomment the motor position array for the corresponding glove.

  ```cpp
    // Left-Hand
    int motorsArr[rows][columns] = {
                                      {19, 26, 27, 14, 23}, // thumb~little
                                      {5, 13, 33, 12, 22},
                                      {21, 17, 16, 18, 4}, // dw1(thumb), dw2, up1, dw3, up2
                                      {32, 25, 15, -1, -1} // back1(pinky)~3
                                    };

      // Right-Hand
      // int motorsArr[rows][columns] = {
      //                                   {23, 12, 33, 13, 19}, // thumb~little
      //                                   {22, 14, 27, 26, 5},
      //                                   {4, 18, 32, 21, 17},  // dwPalm1(thumb), dw2, up1, up2, dw3
      //                                   {16, 15, 25, -1, -1} // back1(pinky)~3
      //                                 };
   ```

5. Upload the code by clicking the **PlatformIO: Upload** button (arrow icon).

6. Check out the glove IP address by clicking the **PlatformIO: Serial Monitor** button (socket icon).

7. Unplug the USB cable and plug two LiPo batteries.

8. Check if the glove is up by getting its uptime via a HTTP GET request.

  ```bash
    # bash & https://httpie.io
    # TODO: change the IP
    http 10.100.8.40/api/uptime
  ```

8. Trigger a vibration event remotely.

  ```bash
    # bash & https://httpie.io
    # TODO: change the IP
    http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=190 mode=1 duration=120 round=3
  ```

## Examples for Launching Vibration Patterns

NB. Every key should be given to a HTTP POST request when launching **preset** vibration patterns!

### Heartbeat Preset Vibration Pattern

```bash
  # TODO: change the IP
  http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=220 mode=1 duration=120 round=10
```

### Droplets Preset Vibration Pattern

```bash
  # TODO: change the IP
  http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=200 mode=2 duration=120 round=3
```

### Water Pillar Preset Vibration Pattern

```bash
  # TODO: change the IP
  http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=220 mode=8 duration=5 round=3
```

### Fireball Preset Single-Point Vibration Pattern

```bash
  # TODO: change the IP
  # gpio=26: vibrating the pointer of a left hand
  http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=250 mode=5 duration=120 round=10
```

### Continuous Single-Point Real-Time Vibration Pattern

```bash
  # TODO: change the IP
  # gpio=26: vibrating the pointer of a left hand

  # Turn ON
  http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=200 mode=0

  # Turn OFF
  http POST 10.100.8.40/api/events time=99999 gpio=26 intensity=0 mode=0
```

## TODO

- API
  - make vibration stop immediately when launching preset vibration patterns
  - replace single **gpio** key with JSON array data type
  - split code for better management
- Gloves (Hardware)
  - fix its electrical wire connection with a soldering iron if needed
