#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

#include "globals.h"
#include "heatSensor.h"

// For heat sensing
HeatSensor heatSensor;

const int BACKLIGHT_DELAY_FRAMES = 80;
const int CLOSE_ENOUGH = 6;  // degrees above 20
const int MIN_BACKLIGHT = 10;  // where the backlight sits when it's idle
int lastBacklight = 0;
int backlightTarget = 0;

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
}

#endif // 0
