#if 1 // Change to 1 to enable this code (must enable ONE user*.cpp only!)
// CORRESPONDING LINE IN HeatSensor.cpp MUST ALSO BE ENABLED!

// This User module tracks the center of heat from an IR sensor.
// Optionally, it also uses a servo to rotate the head left and right to track further.

// The pin on which the left-right servo is attached.
// Comment out to disable servo support.
#define SERVO_PIN  3
#define NECK_SLOWEST_UPDATE_PERIOD  30  // ms/degree - larger is slower
#define NECK_FASTEST_UPDATE_PERIOD  20  // ms/degree - larger is slower
#define DEGREES_SWING_PLUS_MINUS  60  // Furthest angle to deviate the servo from the middle
#define SIGHT_HALF_ANGLE  20  // If the focus is at -1 or +1 in the heat sensor, 
  // we should turn the head this many degrees to get it centered.
#define MS_PER_MOVE  300  // Move only after you're pretty sure 
  // the head will have reached the previous move target.
  // This is the delay for a full move of SIGHT_HALF_ANGLE.
#define STARTUP_DELAY_MS  5000  // Will center for this many seconds at startup.

#include "globals.h"
#include "heatSensor.h"

#ifdef SERVO_PIN
  #include <ControlledServo.h>
  #include <Servo.h>
  static Servo s_neck;
  static ControlledServo neck;
  static int headAngle = 90;
  static unsigned long lastMoveMs = 0;  // millis() at time of last movement change
  static bool servoPaused = false;
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
  s_neck.attach(SERVO_PIN);
  neck.setServo(s_neck);
  neck.setRate(NECK_SLOWEST_UPDATE_PERIOD);
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
  // See if we should toggle pausing the servo.
  arcada.readButtons();
  uint32_t buttonState = arcada.justPressedButtons();
  if(buttonState & ARCADA_BUTTONMASK_A)
    servoPaused = !servoPaused;

  unsigned long waitTime;
  int newHeadAngle, angleDelta;
  if (servoPaused || (millis() < STARTUP_DELAY_MS)) {
    newHeadAngle = 90;
    waitTime = 300;
    angleDelta = 0;
  } else {
    // How far would we have to turn to get the head aligned straight at the focus?
    angleDelta = heatSensor.x * SIGHT_HALF_ANGLE;
    newHeadAngle = headAngle + angleDelta;
    // Constrain that to the limits of neck motion.
    newHeadAngle = min(90 + DEGREES_SWING_PLUS_MINUS, max(90 - DEGREES_SWING_PLUS_MINUS, newHeadAngle));
    angleDelta = newHeadAngle - headAngle;
    // Is that enough of a discrepancy to make it worth it, this soon after a previous move?
    // If it's a big difference, do it soon; if it's small, wait longer.
    waitTime = MS_PER_MOVE * SIGHT_HALF_ANGLE * 1.0 / abs(angleDelta);
  }

  // Update the servo position.
  if (millis() - lastMoveMs > waitTime) {
    headAngle = newHeadAngle;
    Serial.print("neck angle: ");
    Serial.print(headAngle);

    // Set a speed proportional to the angle.
    int updatePeriod = map(
        abs(angleDelta), 
        0, SIGHT_HALF_ANGLE, 
        NECK_SLOWEST_UPDATE_PERIOD, NECK_FASTEST_UPDATE_PERIOD
      );
    Serial.print("   angle period, ms: ");
    Serial.println(updatePeriod);
    neck.setRate(updatePeriod);

    neck.moveTo(headAngle);
    lastMoveMs = millis();
  }
  neck.update();
  #endif
}

#endif // 0
