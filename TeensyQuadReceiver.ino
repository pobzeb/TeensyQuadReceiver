#define RADIO_SERIAL_BAUD_RATE 115200

#define LED 13
#define HEARTBEAT_PIN 6
#define THROTTLE_CHANNEL_PIN 7
#define YAW_CHANNEL_PIN 8
#define PITCH_CHANNEL_PIN 9
#define ROLL_CHANNEL_PIN 10
#define ARM_PIN 11

// Status transmit timeout in microseconds
#define STATUS_TX_TIMEOUT 50000

// Radio input values.
HardwareSerial *radio;
int bytesAvailable;
int state = 0;
int rInputs[7];
bool messageReady = false;
bool displayResults = false;
char rb = ' ';
int b = 0;

// Hold input Throttle, roll, pitch and yaw
int throttle_input, roll_input, pitch_input, yaw_input;

void setup() {
  radio = &Serial1;
  Serial.begin(9600);
  radio->begin(RADIO_SERIAL_BAUD_RATE);

  // Zero out the radio buffer
  Serial.println(F("Beginning Setup"));
  pinMode(LED, OUTPUT);
  while(radio->read() != -1);

  // Now LED off
  delay(200);
  flashLED(6);
  digitalWriteFast(LED, LOW);
  Serial.println(F("Setup Complete"));
  delay(200);
}

// Function to quickly flash led
void flashLED(int times) {
  for (int i = 1; i <= times; i++) {
    digitalWriteFast(LED, HIGH);
    delay(90);
    digitalWriteFast(LED, LOW);
    delay(90);
  }
}

void loop() {
  // Check to see if there are new values
  handleRadioInput();
}

void handleRadioInput() {
  // Get the number of bytes available
  bytesAvailable = radio->available();

  // If there is data to read, read it
  if (bytesAvailable > 0) {
    digitalWriteFast(LED, HIGH);
    while (bytesAvailable > 0) {
      // Look for start of message character.
      if (state == 0) {
        b = 0;
        messageReady = false;
        displayResults = false;
        rb = (char)radio->read();

        // Check to see if this is the beginning of the message
        if (rb == 'm') {
          // Move to next state
          state = 1;
        }
        else if (rb == 'h') {
          // Move to heartbeat state
          state = 2;
        }
        else {
          // Reset state
          state = 0;
        }
        continue;
      }
      // Read message bytes.
      else if (state == 1) {
        // Read the byte.
        rInputs[b] = radio->read();

        // Check to see if we found the start of another message instead.
        if ((char)rInputs[b] == 'm') {
          messageReady = false;
          b = 0;
          continue;
        }

        // Increment byte counter.
        b++;

        // Check to see if that was the end of the message.
        if (b == 7) messageReady = true;
      }
      else if (state == 2) {
          radio->read();
          Serial.println(F("HEARTBEAT"));
          digitalWriteFast(HEARTBEAT_PIN, HIGH);
          delay(20);
          digitalWriteFast(HEARTBEAT_PIN, LOW);

          // Reset for the next command
          state = 0;
          messageReady = false;
          b = 0;
      }

      // If the message is ready, parse the bytes.
      if (messageReady == true) {
        for (b = 0; b < 7; b++) {
          switch (b) {
            case 0: /* Throttle */ {
              throttle_input = (int)map(rInputs[b], 0, 255, 1000, 2000);
              digitalWriteFast(THROTTLE_CHANNEL_PIN, HIGH);
              delay(throttle_input);
              digitalWriteFast(THROTTLE_CHANNEL_PIN, LOW);
              break;
            }
            case 1: /* Yaw */ {
              yaw_input = (int)map(rInputs[b], 0, 255, 1000, 2000);
              digitalWriteFast(YAW_CHANNEL_PIN, HIGH);
              delay(yaw_input);
              digitalWriteFast(YAW_CHANNEL_PIN, LOW);
              break;
            }
            case 2: /* Pitch */ {
              pitch_input = (int)map(rInputs[b], 0, 255, 1000, 2000);
              digitalWriteFast(PITCH_CHANNEL_PIN, HIGH);
              delay(pitch_input);
              digitalWriteFast(PITCH_CHANNEL_PIN, LOW);
              break;
            }
            case 3: /* Roll */ {
              roll_input = (int)map(rInputs[b], 0, 255, 1000, 2000);
              digitalWriteFast(ROLL_CHANNEL_PIN, HIGH);
              delay(roll_input);
              digitalWriteFast(ROLL_CHANNEL_PIN, LOW);
              break;
            }
            case 4: /* L_Trigger */ {
//              float value = map(rInputs[b], 0, 255, -1.0f, 1.0f);
              break;
            }
            case 5: /* R_Trigger */ {
//              float value = map(rInputs[b], 0, 255, -1.0f, 1.0f);
              break;
            }
            case 6: /* Buttons */ {
              // if (rInputs[b] == 1) {
              //   // Run sensor calibration
              //   doCalibration();
              // }
              if (rInputs[b] == 2) {
                // Arm the motors
                digitalWriteFast(ARM_PIN, HIGH);
                delay(20);
                digitalWriteFast(ARM_PIN, LOW);
              }
              // if (rInputs[b] == 4) {
              //   if (flightState != STATE_RUNNING) {
              //     // Clear sensor values
              //     sensorRoll = 0.0;
              //     sensorPitch = 0.0;
              //     sensorYaw = 0.0;
              //     rollCalibration = 0.0;
              //     pitchCalibration = 0.0;
              //     yawCalibration = 0.0;
              //   }
              // }
              break;
            }
          }
        }

        // Reset state for next message
        state = 0;
      }

      bytesAvailable = radio->available();
    }
    digitalWriteFast(LED, LOW);
  }
}
