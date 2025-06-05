#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// Constants
constexpr int SAMPLE_INTERVAL_US = 100;
constexpr float TEMP_WARNING_THRESHOLD = 85.0f;
constexpr float TEMP_CRITICAL_LOW = 5.0f;
constexpr float TEMP_CRITICAL_HIGH = 105.0f;

enum class LEDState { OFF, ON };

// EEPROM Configuration
class EEPROMConfig {
    public:
        enum Revision { REV_A = 0, REV_B = 1 };

        EEPROMConfig() {
            readMock();
        }

        Revision getRevision() const {
            return revision;
        }

        std::string getSerialNumber() const {
            return serial_number;
        }

    private:
        Revision revision;
        std::string serial_number;

        void readMock() {
            revision = REV_A;
            serial_number = "ABC1234";
        }
};

// LED Output
class LEDController {
    public:
        void update(float temperature) {
            green = yellow = red = LEDState::OFF;
            if (temperature < TEMP_CRITICAL_LOW || temperature >= TEMP_CRITICAL_HIGH) {
                red = LEDState::ON;
            } else if (temperature >= TEMP_WARNING_THRESHOLD) {
                yellow = LEDState::ON;
            } else {
                green = LEDState::ON;
            }
        }

        void print() const {
            std::cout << "LEDs: [G: " << (green == LEDState::ON ? "ON" : "OFF")
                    << ", Y: " << (yellow == LEDState::ON ? "ON" : "OFF")
                    << ", R: " << (red == LEDState::ON ? "ON" : "OFF") << "]\n";
        }

    private:
        LEDState green = LEDState::OFF;
        LEDState yellow = LEDState::OFF;
        LEDState red = LEDState::OFF;
};

// ADC Class
class ADC {
    public:
        int read() const {
            return std::rand() % 1500;
        }
};

// Temperature Converter
class TemperatureConverter {
    public:
        static float convert(int adcValue, EEPROMConfig::Revision rev) {
            return (rev == EEPROMConfig::REV_A) ? static_cast<float>(adcValue)
                                                : static_cast<float>(adcValue) / 10.0f;
        }
};

// Temperature Monitor
class TemperatureMonitor {
    public:
        TemperatureMonitor()
            : config(), adc(), leds() {}
        void run(int samples = 100) {
            std::cout << "Starting temperature monitor [Rev: "
                    << static_cast<int>(config.getRevision()) << ", Serial: "
                    << config.getSerialNumber() << "]\n";
            for (int i = 0; i < samples; ++i) {
                int adcVal = adc.read(); // reading should be done in the ISR
                float temp = TemperatureConverter::convert(adcVal, config.getRevision());
                leds.update(temp);
                printTemperature(temp);
                leds.print();
                usleep(SAMPLE_INTERVAL_US);
            }
        }

    private:
        EEPROMConfig config;
        ADC adc;
        LEDController leds;
        void printTemperature(float temp) const {
            if (config.getRevision() == EEPROMConfig::REV_A) {
                std::cout << "Temp: " << static_cast<int>(temp) << "C\t";
            } else {
                std::cout << "Temp: " << temp << "C\t";
            }
        }
};

int main() {
    std::srand(std::time(nullptr));
    TemperatureMonitor monitor;
    monitor.run();
    return 0;
}