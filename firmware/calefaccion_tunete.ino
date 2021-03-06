const int TMP102_ADDRESS = 0x48;
const int TMP102_REGISTER = 0x00;
const int TMP102_BYTES = 2;
const int TMP102_RETRY = 10;
const int TMP102_DELAY = 100; // ms

double temp = -0xff;
int err = 1;
char buffer[8];
int retry;

const unsigned long SAMPLE_RATE = 30000; // ms
unsigned long lastSample = 0;
const char *publishError = "temperature/error";

const unsigned long PUBLISH_RATE = 300000; // ms
unsigned long lastPublish = 0;
const char *publishEvent = "temperature/5min";

const unsigned long BLINK_RATE = 1000; // ms
unsigned long lastBlink = 0;
int led = LOW;

void setup() {
  Particle.variable("temperature", temp);
  Particle.variable("error", err);
  Particle.variable("retry", retry);
  pinMode(D7, OUTPUT);
  Wire.begin();
}

void loop() {
  if (lastSample + SAMPLE_RATE < millis()) {
    if (!update()) {
      sprintf(buffer, "%d", err);
      Particle.publish(publishError, buffer);
    }
    lastSample = millis();
  }
  if (lastPublish + PUBLISH_RATE < millis()) {
    sprintf(buffer, "%.1f", temp);
    Particle.publish(publishEvent, buffer);
    lastPublish = millis();
  }
  if (lastBlink + BLINK_RATE < millis()) {
    digitalWrite(D7, (led) ? HIGH : LOW);
    led = !led;
    lastBlink = millis();
  }
}

int update() {
  if (!Wire.isEnabled()) {
    temp = -0xff;
    err = 2;
    return 0;
  }
  for (retry = 0; retry < TMP102_RETRY; retry++) {
    Wire.beginTransmission(TMP102_ADDRESS);
    Wire.write(TMP102_REGISTER);
    Wire.endTransmission();
    Wire.requestFrom(TMP102_ADDRESS, TMP102_BYTES);
    if (Wire.available() == TMP102_BYTES) {
      byte MSB = Wire.read();
      byte LSB = Wire.read();
      temp = ((( MSB << 8) | LSB) >> 4) * 0.0625;
      err = 0;
      return 1;
    } else {
      Wire.reset();
      temp = -0xff;
      err = 3;
    }
    delay(TMP102_DELAY);
  }
  return 0;
}
