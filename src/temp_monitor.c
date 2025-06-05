/* temp_monitor.c
Software for a bare-metal embedded system used in a temperature monitoring and
visualization device, with indicators for OK, Warning, and Critical temperature ranges.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // for usleep

// Constants
#define SAMPLE_INTERVAL_US 100
#define EEPROM_HARDWARE_REV_A 0
#define EEPROM_HARDWARE_REV_B 1
#define TEMP_WARNING_THRESHOLD 85.0
#define TEMP_CRITICAL_LOW 5.0
#define TEMP_CRITICAL_HIGH 105.0

// LED States
typedef enum {
    LED_OFF,
    LED_ON
} LEDState;

// EEPROM Mock via I2C
#define EEPROM_ADDRESS 0x50 // Mock I2C address

typedef struct {
    uint8_t hardware_revision; // 0 = Rev-A, 1 = Rev-B
    char serial_number[16];    // Placeholder
} EEPROMConfig;

// Simulate reading EEPROM memory via I2C
void i2c_read_eeprom(uint8_t address, uint8_t* buffer, size_t length) {
    if (address == EEPROM_ADDRESS && length >= sizeof(EEPROMConfig)) {
        EEPROMConfig mock_data = {
            .hardware_revision = EEPROM_HARDWARE_REV_A,
            .serial_number = "ABC1234"
        };
        memcpy(buffer, &mock_data, sizeof(EEPROMConfig));
    }
}

EEPROMConfig eeprom_read_config() {
    EEPROMConfig config;
    uint8_t buffer[sizeof(EEPROMConfig)];
    i2c_read_eeprom(EEPROM_ADDRESS, buffer, sizeof(buffer));
    memcpy(&config, buffer, sizeof(EEPROMConfig));
    return config;
}

// Mock GPIO
typedef struct {
    LEDState green;
    LEDState yellow;
    LEDState red;
} LEDOut;

void update_leds(LEDOut* leds, float temperature) {
    leds->green = LED_OFF;
    leds->yellow = LED_OFF;
    leds->red = LED_OFF;
    if (temperature < TEMP_CRITICAL_LOW || temperature >= TEMP_CRITICAL_HIGH) {
        leds->red = LED_ON;
    } else if (temperature >= TEMP_WARNING_THRESHOLD) {
        leds->yellow = LED_ON;
    } else {
        leds->green = LED_ON;
    }
}

void print_leds(const LEDOut* leds) {
    printf("LEDs: [G: %s, Y: %s, R: %s]\n",
        leds->green == LED_ON ? "ON" : "OFF",
        leds->yellow == LED_ON ? "ON" : "OFF",
        leds->red == LED_ON ? "ON" : "OFF");
}

// ADC Mock
int adc_read_mock() {
    return rand() % 1500; // Random value to simulate ADC
}

// Converts ADC value to temperature
float convert_adc_to_temp(int adc_value, uint8_t revision) {
    if (revision == EEPROM_HARDWARE_REV_A) {
        return (float)adc_value;
    } else {
        return ((float)adc_value) / 10.0f;
    }
}

// Update temperature and LEDs
void update_monitoring(const EEPROMConfig* config, int adc_val) {
    static LEDOut leds;
    float temp = convert_adc_to_temp(adc_val, config->hardware_revision);
    update_leds(&leds, temp);
    if (config->hardware_revision == EEPROM_HARDWARE_REV_A) {
        printf("Temp: %.0fC\t", temp);
    } else {
         printf("Temp: %.1fC\t", temp);
    }
    print_leds(&leds);
}


// Mock ISR
void timer_isr_mock(int* adc_val) {
    *adc_val = adc_read_mock();
}

int main() {
    srand(time(NULL));
    int static adc_val = 0;
    EEPROMConfig config = eeprom_read_config();
    printf("Starting temperature monitor [Rev: %d, Serial: %s]\n",
        config.hardware_revision, config.serial_number);
    for (int i = 0; i < 100; ++i) { // run 100 samples
        timer_isr_mock(&adc_val);
        update_monitoring(&config, adc_val);
        usleep(SAMPLE_INTERVAL_US);
    }
    return 0;
}
