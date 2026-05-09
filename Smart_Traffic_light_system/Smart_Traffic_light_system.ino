// === Enums ===
enum Direction { SIDE_A, SIDE_B };
enum Phase { GREEN_PHASE, YELLOW_PHASE, ALL_RED_PHASE };

// === Function Prototypes ===
void setState(Direction side, Phase phase);
void switchToNextSide();
int getDistance(int trigPin, int echoPin);
void updateTrafficScores();

// === Sensor Pins ===
const int trigPinA = 6, echoPinA = 7;
const int trigPinB = 8, echoPinB = 9;

// === LED Pins for Side A ===
const int redA = 2;
const int yellowA = 3;
const int greenA = 4;

// === LED Pins for Side B ===
const int redB = 5;
const int yellowB = 10;
const int greenB = 11;

// === Variables ===
Direction currentSide = SIDE_A;
Phase currentPhase = GREEN_PHASE;

unsigned long previousMillis = 0; // For 1-second interval updates (for countdowns and main loop print)
unsigned long currentPhaseStartTime = 0; // To track how long the current phase has been active
unsigned long lastScoreUpdateTime = 0;  // To control how often scores are updated

int countdown = 0; // For Yellow/All-Red phases countdown display
int lastPrintedGreenTime = -1; // To store the last printed green time to avoid redundant prints

// --- Traffic Scoring Variables ---
int trafficScoreA = 0;
int trafficScoreB = 0;
bool wasCarDetectedA = false; // To detect rising edge (new car detected) for Side A
bool wasCarDetectedB = false; // To detect rising edge (new car detected) for Side B

// === Timing Constants (in milliseconds) ===
const unsigned long yellowDuration = 3000;      // Yellow time (3 seconds)
const unsigned long allRedDuration = 2000;      // All Red time (2 seconds)
const unsigned long minGreenDuration = 5000;    // Minimum green time (5 seconds)
const unsigned long maxGreenDuration = 25000;   // Maximum green time (25 seconds)

// Interval for updating traffic scores when a side is red
const unsigned long scoreUpdateInterval = 100; // Check for cars more frequently (every 100ms)

const int carDetectionThreshold = 70; // cm - Distance at which a car is considered detected (adjust if needed)

void setup() {
  pinMode(trigPinA, OUTPUT); pinMode(echoPinA, INPUT);
  pinMode(trigPinB, OUTPUT); pinMode(echoPinB, INPUT);

  pinMode(redA, OUTPUT); pinMode(yellowA, OUTPUT); pinMode(greenA, OUTPUT);
  pinMode(redB, OUTPUT); pinMode(yellowB, OUTPUT); pinMode(greenB, OUTPUT);

  Serial.begin(9600);
  Serial.println("Traffic Light System Initialized.");
  setState(SIDE_A, GREEN_PHASE); // Start with Side A GREEN
}

void loop() {
  unsigned long currentMillis = millis();

  // --- Update Traffic Scores (High Frequency) ---
  // This runs regardless of current phase to always monitor traffic on red lights
  if (currentMillis - lastScoreUpdateTime >= scoreUpdateInterval) {
    lastScoreUpdateTime = currentMillis;
    updateTrafficScores(); // Update scores for the side(s) that are currently red
  }

  // --- Handle Phase Transitions and Serial Output (Once per second) ---
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis; // Update previousMillis for the next second tick

    Serial.print("Scores - A: "); Serial.print(trafficScoreA);
    Serial.print(", B: "); Serial.println(trafficScoreB);

    if (currentPhase == GREEN_PHASE) {
      unsigned long elapsedGreenTime = currentMillis - currentPhaseStartTime;
      int remainingTimeSeconds; // For calculating and printing remaining time

      if (elapsedGreenTime < minGreenDuration) {
        remainingTimeSeconds = (minGreenDuration - elapsedGreenTime) / 1000;
        if (remainingTimeSeconds != lastPrintedGreenTime) {
          Serial.print("Current Side GREEN (Min Time Remaining): ");
          Serial.println(remainingTimeSeconds);
          lastPrintedGreenTime = remainingTimeSeconds;
        }
      } else {
        // After minGreenDuration, check if a switch is needed based on other side's traffic
        // Or if maxGreenDuration is reached for the current side

        // Re-read current side's detection to influence extension decision
        bool currentCarDetectedOnCurrentSide;
        if (currentSide == SIDE_A) {
          currentCarDetectedOnCurrentSide = (getDistance(trigPinA, echoPinA) < carDetectionThreshold);
        } else { // SIDE_B
          currentCarDetectedOnCurrentSide = (getDistance(trigPinB, echoPinB) < carDetectionThreshold);
        }

        // Only consider switching if the other side has accumulated traffic
        bool otherSideHasAccumulatedTraffic = (currentSide == SIDE_A && trafficScoreB > 0) ||(currentSide == SIDE_B && trafficScoreA > 0);

        // Conditions to switch from GREEN to YELLOW:
        // 1. Max green duration reached (force switch)
        // 2. Other side has traffic AND current side has no car AND min green duration passed
        //    (This ensures current side only holds green if a car is still present AND
        //     the other side isn't waiting, or if max time reached)
        if (elapsedGreenTime >= maxGreenDuration ||
           (otherSideHasAccumulatedTraffic && !currentCarDetectedOnCurrentSide)) {
             setState(currentSide, YELLOW_PHASE); // Time to switch
             Serial.println("Switching from GREEN: Max time or Other side needs it.");
        } else {
          // Continue green: Current side has car, OR other side has no traffic, OR not at max time
          remainingTimeSeconds = (maxGreenDuration - elapsedGreenTime) / 1000;
          if (remainingTimeSeconds != lastPrintedGreenTime) {
            Serial.print("Current Side GREEN (Waiting for switch or max time): ");
            Serial.println(remainingTimeSeconds);
            lastPrintedGreenTime = remainingTimeSeconds;
          }
        }
      }
    } else { // YELLOW_PHASE or ALL_RED_PHASE
      Serial.print("Countdown: ");
      Serial.println(countdown);
      countdown--;

      if (countdown <= 0) {
        if (currentPhase == YELLOW_PHASE) {
          setState(currentSide, ALL_RED_PHASE);
        } else { // ALL_RED_PHASE
          switchToNextSide(); // After ALL_RED, decide which side gets green
        }
      }
    }
  }
}

int getDistance(int trigPin, int echoPin) {
  // Clear the trigPin by setting it LOW for a moment
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Set the trigPin HIGH for 10 microseconds to send a pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse (timeout after 30ms)
  long duration = pulseIn(echoPin, HIGH, 30000);

  // Check for timeout or out-of-range readings
  // 0 duration means timeout (no echo received)
  // Max distance for 30ms timeout is approx (30000 * 0.034 / 2) = 510 cm.
  // 10000 microsecond corresponds to about 170 cm. Anything much higher
  // means the object is likely out of the useful detection range or there's noise.
  if (duration == 0 || duration > 10000) return 999;

  // Calculate distance in cm (Speed of sound = 0.034 cm/microsecond)
  int distance = duration * 0.034 / 2;
  return distance;
}

// Function to update traffic scores based on sensor readings
void updateTrafficScores() {
  // Read current detection status for both sides
  bool currentCarDetectedA = (getDistance(trigPinA, echoPinA) < carDetectionThreshold);
  bool currentCarDetectedB = (getDistance(trigPinB, echoPinB) < carDetectionThreshold);

  // --- Logic for Side A's Score ---
  // Only accumulate score for Side A if it is currently RED or during ALL_RED
  if (currentSide == SIDE_B || currentPhase == ALL_RED_PHASE) {
    if (currentCarDetectedA && !wasCarDetectedA) { // Detect a rising edge (car just appeared)
      trafficScoreA++;
      Serial.print("!New car on Side A! Score A: "); Serial.println(trafficScoreA);
    }
    wasCarDetectedA = currentCarDetectedA; // Update previous state
  } else { // Side A is green, so reset its score
    if (trafficScoreA > 0) {
      Serial.println("Side A went GREEN, Resetting Score A.");
    }
    trafficScoreA = 0;
  }

  // --- Logic for Side B's Score ---
  // Only accumulate score for Side B if it is currently RED or during ALL_RED
  if (currentSide == SIDE_A || currentPhase == ALL_RED_PHASE) {
    if (currentCarDetectedB && !wasCarDetectedB) { // Detect a rising edge (car just appeared)
      trafficScoreB++;
      Serial.print("!New car on Side B! Score B: "); Serial.println(trafficScoreB);
    }
    wasCarDetectedB = currentCarDetectedB; // Update previous state
  } else { // Side B is green, so reset its score
    if (trafficScoreB > 0) {
      Serial.println("Side B went GREEN, Resetting Score B.");
    }
    trafficScoreB = 0;
  }
}

void switchToNextSide() {
  Serial.println("Deciding next green light...");
  Serial.print("Final Scores - A: "); Serial.print(trafficScoreA);
  Serial.print(", B: "); Serial.println(trafficScoreB);

  Direction nextSide;

  if (trafficScoreA > trafficScoreB) {
    nextSide = SIDE_A;
    Serial.println("Side A has more traffic. Switching to SIDE A.");
  } else if (trafficScoreB > trafficScoreA) {
    nextSide = SIDE_B;
    Serial.println("Side B has more traffic. Switching to SIDE B.");
  } else {
    // If scores are equal, alternate from the side that just had green.
    // This ensures fair rotation even with no traffic or equal traffic.
    if (currentSide == SIDE_A) { // If A was just green, then B goes next
      nextSide = SIDE_B;
      Serial.println("Scores equal. Alternating to SIDE B.");
    } else { // If B was just green, then A goes next
      nextSide = SIDE_A;
      Serial.println("Scores equal. Alternating to SIDE A.");
    }
  }

  // Reset scores after deciding the next green light
  trafficScoreA = 0;
  trafficScoreB = 0;

  setState(nextSide, GREEN_PHASE);
}

void setState(Direction side, Phase phase) {
  // Turn off all LEDs first for a clean state transition
  digitalWrite(redA, LOW); digitalWrite(yellowA, LOW); digitalWrite(greenA, LOW);
  digitalWrite(redB, LOW); digitalWrite(yellowB, LOW); digitalWrite(greenB, LOW);

  currentSide = side;
  currentPhase = phase;
  currentPhaseStartTime = millis(); // Reset phase start time for the new phase
  lastPrintedGreenTime = -1; // Reset this so the first print for the new green phase appears

  if (phase == ALL_RED_PHASE) {
    digitalWrite(redA, HIGH);
    digitalWrite(redB, HIGH);
    countdown = allRedDuration / 1000; // Convert ms to seconds for countdown display
    Serial.println("STATE: BOTH SIDES RED");
  } else if (side == SIDE_A) {
    if (phase == GREEN_PHASE) {
      digitalWrite(greenA, HIGH);
      digitalWrite(redB, HIGH); // Side B is RED when Side A is GREEN
      Serial.println("STATE: SIDE A GREEN");
      // Green duration is dynamic, managed in loop()
    } else { // YELLOW_PHASE
      digitalWrite(yellowA, HIGH);
      digitalWrite(redB, HIGH); // Side B remains RED during Side A's YELLOW
      countdown = yellowDuration / 1000; // Convert ms to seconds
      Serial.println("STATE: SIDE A YELLOW");
    }
  } else { // SIDE_B
    if (phase == GREEN_PHASE) {
      digitalWrite(greenB, HIGH);
      digitalWrite(redA, HIGH); // Side A is RED when Side B is GREEN
      Serial.println("STATE: SIDE B GREEN");
      // Green duration is dynamic, managed in loop()
    } else { // YELLOW_PHASE
      digitalWrite(yellowB, HIGH);
      digitalWrite(redA, HIGH); // Side A remains RED during Side B's YELLOW
      countdown = yellowDuration / 1000; // Convert ms to seconds
      Serial.println("STATE: SIDE B YELLOW");
    }
  }
}
