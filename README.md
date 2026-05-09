# Smart-Traffic-Light-System
IoT-Based smart traffic light management using Arduino Uno and Ultrasonic sensor(HCSR-04) to optimize signal timing based on vehicle density.

## How the System Works
This project implements a smart density-based traffic control logic for a two-lane intersection (**Side A** and **Side B**).

### 1. Default Mode
* The system operates on a standard timer.
* By default, each side has **20 seconds** of Green light before switching to the other side.

### 2. Smart Override (Density Detection)
The system uses **HCSR-04 Ultrasonic sensors** to monitor vehicle density on both lanes.
* If one side detects a significantly higher number of vehicles compared to the other side, the system triggers an immediate switch.
* **Transition:** The current Green light will turn **Yellow for 2 seconds** as a safety warning, then immediately switch to **Green** for the high-traffic side.

### Components Used
* **Microcontroller:** Arduino Uno
* **Sensors:** HCSR-04 Ultrasonic Sensors (Density detection).
               It has a Echo pin and Trig pin.
  The Basic PrincipleThe sensor has two main parts: a Transmitter (the "eye" that sends sound) and a Receiver (the "eye" that listens for sound).The Trigger: Your Arduino sends a high-signal pulse to the Trig pin for $10\mu s$.The Sound Burst: The transmitter emits a burst of 8 ultrasonic sonic pulses at $40\text{ kHz}$. This sound is far above what humans can hear.The Reflection: If an object (like a car in your traffic lane) is in front of the sensor, the sound waves hit it and bounce back.The Echo: The receiver detects the returning sound and sets the Echo pin to HIGH.The Math Behind the DistanceThe Arduino measures the Time the Echo pin stayed high. To turn that time into a distance (cm), you use the speed of sound: Distance = (Speed*Time)/2
* **Outputs:** LEDs (Red, Yellow, Green for both sides)
* Close distance (e.g., < 10cm) = Car is right at the sensor.
* Far distance (e.g.,> 100cm) = Lane is empty.
