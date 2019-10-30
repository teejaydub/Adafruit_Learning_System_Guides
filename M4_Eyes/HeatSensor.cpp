/* Read the IR sensor and try to figure out where the heat is located. 
*/

#include "HeatSensor.h"

#include <Wire.h>
#include <Adafruit_AMG88xx.h>

// Any movement of the focus point that's greater than this in either X or Y
// relative to the last reading is suppressed.
// This to prevent the occasional wild reading, whether from electrical issues,
// firmware bugs, timing issues, or a busy image from causing nonsensically rapid movement.
// The larger the value, the more tolerant the filter is of big changes.
const float SQUELCH = 0.6;

Adafruit_AMG88xx amg;

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];

HeatSensor::HeatSensor()
{
    rotation = ROTATE_0;
}

void HeatSensor::setup()
{
    x = 0;
    y = 0;
    magnitude = 0;

    bool status;
    
    // default settings
    status = amg.begin();
    if (!status) {
        Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
        while (1);
    }

    yield();
    delay(100); // let sensor boot up
}

void HeatSensor::rotate_coords(float& x, float& y)
{
    float oldX = x;
    float oldY = y;

    switch (rotation) {
    case ROTATE_90:
        y = oldX;
        x = -oldY;
        break;
    case ROTATE_180:
        x = -oldX;
        y = -oldY;
        break;
    case ROTATE_270:
        y = -oldX;
        x = oldY;
        break;
    }
}

void swap(float& a, float& b) {
    float temp = a;
    a = b;
    b = temp;
}

void HeatSensor::filter_coords(float& x, float& y)
{
    static float oldX = 0.0;
    static float oldY = 0.0;

    if (abs(x - oldX) >= SQUELCH || abs(y - oldY) >= SQUELCH) {
        // Filter this one out.
        // So, use the last coordinates, but save these for comparison next time.
        #if SERIAL_OUT == 3 || SERIAL_OUT == 2
            // Print coordinates and brightness
            Serial.print("filtering out: ");
            Serial.print(x);
            Serial.print(',');
            Serial.println(y);
        #endif
        swap(x, oldX);
        swap(y, oldY);
    } else {
        // Just store these for next time; don't change them.
        oldX = x;
        oldY = y;
    }
}

// Find the approximate X and Y values of the peak temperature in the pixel array,
// along with the magnitude of the brightest spot.
void HeatSensor::find_focus()
{
    amg.readPixels(pixels);
    yield();

    x = 0, y = 0, magnitude = 0;
    float minVal = 100, maxVal = 0;
    int i = 0;
    for (float yPos = 3.5; yPos > -4; yPos -= 1.0) {
        for (float xPos = 3.5; xPos > -4; xPos -= 1.0) {
            float p = pixels[i];
            x += xPos * p;
            y += yPos * p;
            minVal = min(minVal, p);
            maxVal = max(maxVal, p);
            i++;
        }
    }
    x = - x / AMG88xx_PIXEL_ARRAY_SIZE / 5.0;
    y = y / AMG88xx_PIXEL_ARRAY_SIZE / 5.0;
    x = max(-1.0, min(1.0, x));
    y = max(-1.0, min(1.0, y));
    magnitude = max(0, min(50, maxVal - MIN_MAGNITUDE));

    rotate_coords(x, y);
    filter_coords(x, y);

    // Report.
#define SERIAL_OUT  3
#if SERIAL_OUT == 1
    // Print raw values
    Serial.print("[");
    for(int i=1; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
      Serial.print(pixels[i-1]);
      Serial.print(", ");
      if( i%8 == 0 ) Serial.println();
    }
    Serial.println("]");
    Serial.println();
#endif
#if SERIAL_OUT == 2
    // Print character-graphic array
    const char charPixels[] = " .-*o0#";
    Serial.println("========");
    for (int i = 1; i <= AMG88xx_PIXEL_ARRAY_SIZE; i++) {
      int val = min(5, round(max(0, pixels[i-1] - MIN_MAGNITUDE) / 2));
      Serial.print(charPixels[val]);
      if (i % 8 == 0) 
        Serial.println();
    }
    Serial.println();
#endif
#if SERIAL_OUT == 3 || SERIAL_OUT == 2
    // Print coordinates and brightness
    Serial.print(x);
    Serial.print(' ');
    Serial.print(y);
    Serial.print(' ');
    Serial.println(magnitude);
#endif
}

/*
void loop() {
    // Read all the pixels

    // Find the focal point.
    float x, y, magnitude;
    find_focus(x, y, magnitude);

    // Set diagnostic LEDs.
    analogWrite(CENTER_LED, round(max(0, magnitude / 30) * 255));
    analogWrite(RIGHT_LED, round(min(1.0, max(0.0, -x / 3)) * 255));
    analogWrite(LEFT_LED, round(min(1.0, max(0.0, x / 3)) * 255));
    analogWrite(UP_LED, round(min(1.0, max(0.0, y / 3)) * 255));
    analogWrite(DOWN_LED, round(min(1.0, max(0.0, -y / 3)) * 255));

    delay(200);
}
*/