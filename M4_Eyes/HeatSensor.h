/* Read the IR sensor and try to figure out where the heat is located. 

    Orientation: Looking into the sensor, the window is on the bottom of the silver package,
    and the Adafruit star is at the upper left.
    X goes from about -1.0 on the left, facing out of the sensor, to +1.0 on the right.
    Y goes roughly from -1.0 (less?) on the bottom to +1.0 on the top.
    The sensor is oriented with its text upside down for the moment.
    Magnitude is currently the maximum temperature of any pixel, in degrees C.
*/

#ifndef __HEAT_SENSOR_H
#define __HEAT_SENSOR_H

class HeatSensor {
public:
    // The current focus position, each from -1.0 .. +1.0.
    float x, y;

    // The current magnitude estimate, in degrees C above 20.
    // 0 is the minimum number returned.
    float magnitude;

    // Set this to adjust the readings before they're reported.
    // If the board is rotated clockwise by the given angle from its home orientation,
    // described above, set the corresponding enum value.
    enum {
        ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
    } rotation;

    // Constructor.
    HeatSensor();

    // Must be called once to initialize the sensor.
    void setup();

    // Reads the sensor and updates x, y, and magnitude.
    void find_focus();

protected:
    // Modifies the given coordinates in-place according to the current rotation.
    void rotate_coords(float& x, float& y);

    // Prevents noise from causing too-rapid fluctuation of the focus point.
    // Modifies the given coordinates in-place.
    void filter_coords(float& x, float& y);
};

#endif