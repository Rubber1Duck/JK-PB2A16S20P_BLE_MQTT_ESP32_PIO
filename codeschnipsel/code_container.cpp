// hier werden codeschnippsel abgelegt die ich in meinem Projekt eventuell verwenden möchte


// CODESTOPPER wird nirgendwo definiert und dieser Code wird daher nie kompiliert
#ifdef CODESTOPPER
// Dieser Code steuert eine einzelne NeoPixel-LED und zeigt verschiedene Farben an.
#include <Adafruit_NeoPixel.h>

// Define the pin where the built-in RGB LED is connected
#define LED_PIN 48

// Define the number of LEDs in the strip (usually 1 for built-in LED)
#define NUM_LEDS 1

// Create an instance of the Adafruit_NeoPixel class
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
// Initialize the NeoPixel library
strip.begin();
strip.show(); // Initialize all pixels to 'off'
}

void loop() {
// Cycle through some colors

// Red
strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
strip.show();
delay(1000);

// Green
strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
strip.show();
delay(1000);

// Blue
strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
strip.show();
delay(1000);

// Yellow
strip.setPixelColor(0, strip.Color(255, 255, 0)); // Yellow
strip.show();
delay(1000);

// Cyan
strip.setPixelColor(0, strip.Color(0, 255, 255)); // Cyan
strip.show();
delay(1000);

// Magenta
strip.setPixelColor(0, strip.Color(255, 0, 255)); // Magenta
strip.show();
delay(1000);

// White
strip.setPixelColor(0, strip.Color(255, 255, 255)); // White
strip.show();
delay(1000);

// Turn off
strip.setPixelColor(0, strip.Color(0, 0, 0)); // Off
strip.show();
delay(1000);
}
#endif
