# Smart-Traffic-Light-System
IoT-Based smart traffic light management using Arduino Uno and Ultrasonic sensor(HCSR-04) to optimize signal timing based on vehicle density.

## How the System Works
This project is an IoT-based traffic management solution designed to optimize signal timings at a two-lane intersection (Side A and Side B). Using an Arduino Uno and HCSR-04 Ultrasonic Sensors, the system dynamically adjusts light phases based on real-time vehicle density.

🚦**How the Logic Works**
Unlike traditional timers, this system uses a "Traffic Scoring" algorithm:

Default Timing: Each lane has a minimum green time of 5 seconds and a maximum of 25 seconds.

Density Detection: Ultrasonic sensors monitor the "Red" lane. Every time a car is detected (distance < 70cm), the "Traffic Score" for that lane increases.

Smart Switching: * If the current Green lane is empty and the Red lane has accumulated a higher traffic score, the system triggers a transition.

Safety Transition: The light turns Yellow for 3 seconds, then All-Red for 2 seconds to ensure the intersection is clear before switching the Green light to the busy side.

Fairness: If traffic scores are equal, the system alternates lanes to prevent one side from waiting too long.

🛠️ **Hardware Components**
Microcontroller: Arduino Uno

**Sensors:** 2x HCSR-04 Ultrasonic Sensors (Density monitoring)
            HCSR-04 Ultrasonic Sensors (Density detection).
            It has a Echo pin and Trig pin.
  The Basic PrincipleThe sensor has two main parts: a Transmitter (the "eye" that sends sound) and a Receiver (the "eye" that listens for sound).The Trigger: Your Arduino sends a high-signal pulse to the Trig pin for $10\mu s$.The Sound Burst: The transmitter emits a burst of 8 ultrasonic sonic pulses at $40\text{ kHz}$. This sound is far above what humans can hear.The Reflection: If an object (like a car in your traffic lane) is in front of the sensor, the sound waves hit it and bounce back.The Echo: The receiver detects the returning sound and sets the Echo pin to HIGH.The Math Behind the DistanceThe Arduino measures the Time the Echo pin stayed high. To turn that time into a distance (cm), you use the speed of sound: Distance = (Speed*Time)/2


* **Live Simulation Link:** https://www.tinkercad.com/things/lWZrTBf5Fe8-2-way-traffic?sharecode=hCA2sMFBpEWfxjNttjZGXNxdTnro2-AJ0EYQ8EMM1wo

**How to Run**
Open the Smart_Traffic_light_system.ino file in the Arduino IDE.

Connect your Arduino Uno and wire the sensors according to the pin definitions in the code.

Upload the code and open the Serial Monitor (9600 baud) to see real-time score updates and state transitions.
