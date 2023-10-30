#if 1 // Change to 1 to enable this code (must enable ONE user*.cpp only!)
// CORRESPONDING LINE IN HeatSensor.cpp MUST ALSO BE ENABLED!

// This User module tracks the center of heat from an IR sensor.
// Optionally, it also uses a servo to rotate the head left and right to track further.

// The pin on which the left-right servo is attached.
// Comment out to disable servo support.
#define SERVO_PIN  3
#define DEGREES_SWING_PLUS_MINUS  60  // Furthest angle to deviate the servo from the middle
#define SIGHT_HALF_ANGLE  20  // If the focus is at -1 or +1 in the heat sensor, 
  // we should turn the head this many degrees to get it centered.
#define MS_PER_MOVE  1000  // Move only after as you're pretty sure 
  // the head will have reached the previous move target.
  // This is the delay for a full move of SIGHT_HALF_ANGLE.

#include "globals.h"
#include "heatSensor.h"

#ifdef SERVO_PIN
  #include <Servo.h>
  static Servo myServo;
  static int headAngle = 90;
  static unsigned long lastMoveMs = 0;  // millis() at time of last movement change
#endif

// For heat sensing
HeatSensor heatSensor;

const int BACKLIGHT_DELAY_FRAMES = 80;
const int CLOSE_ENOUGH = 6;  // degrees above 20
const int MIN_BACKLIGHT = 10;  // where the backlight sits when it's idle
static int lastBacklight = 0;
static int backlightTarget = 0;

// This file provides a crude way to "drop in" user code to the eyes,
// allowing concurrent operations without having to maintain a bunch of
// special derivatives of the eye code (which is still undergoing a lot
// of development). Just replace the source code contents of THIS TAB ONLY,
// compile and upload to board. Shouldn't need to modify other eye code.

// User globals can go here, recommend declaring as static, e.g.:
// static int foo = 42;

// Called once near the end of the setup() function. If your code requires
// a lot of time to initialize, make periodic calls to yield() to keep the
// USB mass storage filesystem alive.
void user_setup(void) {
  showSplashScreen = false;
  moveEyesRandomly = false;
  heatSensor.setup();
  heatSensor.rotation = HeatSensor::ROTATE_90;

  #ifdef SERVO_PIN
  myServo.attach(SERVO_PIN);
  #endif
}

// Called periodically during eye animation. This is invoked in the
// interval before starting drawing on the last eye (left eye on MONSTER
// M4SK, sole eye on HalloWing M0) so it won't exacerbate visible tearing
// in eye rendering. This is also SPI "quiet time" on the MONSTER M4SK so
// it's OK to do I2C or other communication across the bridge.
void user_loop(void) {
  // Estimate the focus position.
  heatSensor.find_focus();

  // Set values for the new X and Y.
  eyeTargetX = heatSensor.x;
  eyeTargetY = -heatSensor.y;

  // Set the backlight brightness according to how close the visitor is.
  // But change it slowly.
  backlightTarget = map(min(6, heatSensor.magnitude), 0, CLOSE_ENOUGH, MIN_BACKLIGHT, 255);
  int nextBacklight = (lastBacklight * BACKLIGHT_DELAY_FRAMES + backlightTarget) / (BACKLIGHT_DELAY_FRAMES + 1);
  arcada.setBacklight(nextBacklight);
  lastBacklight = nextBacklight;

  #ifdef SERVO_PIN
  // How far would we have to turn to get the head aligned straight at the focus?
  int angleDelta = heatSensor.x * SIGHT_HALF_ANGLE;
  int newHeadAngle = headAngle + angleDelta;
  // Constrain that to the limits of neck motion.
  newHeadAngle = min(90 + DEGREES_SWING_PLUS_MINUS, max(90 - DEGREES_SWING_PLUS_MINUS, newHeadAngle));
  angleDelta = newHeadAngle - headAngle;
  // Is that enough of a discrepancy to make it worth it, this soon after a previous move?
  // If it's a big difference, do it soon; if it's small, wait longer.
  unsigned long waitTime = MS_PER_MOVE * SIGHT_HALF_ANGLE * 1.0 / abs(angleDelta);
  if (millis() - lastMoveMs > waitTime) {
    // For testing, just track the heat source with the servo.
    headAngle = newHeadAngle;
    Serial.println(headAngle);
    myServo.write(headAngle);
    lastMoveMs = millis();
  }
  #endif
}

#endif // 0
