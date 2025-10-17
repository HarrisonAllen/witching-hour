#include <pebble.h>

#define MAX_CHARS 40

// Enums
typedef enum weather {
    SUNNY,
    PARTLYCLOUDY,
    CLOUDY,
    RAINY,
    SNOWY,
    STORMY
} Weather;
  
// Define settings struct
typedef struct ClaySettings {
  // Relatime Data
  int TEMPERATURE;                   // Current temperature
  Weather CONDITIONS;                // Current weather conditions
  int MOON_FRACILLUM;              // Current moon visibility %
  // General settings
  bool UseCurrentLocation;           // use GPS for weather?
  char Latitude[MAX_CHARS];          // latitude when not using GPS
  char Longitude[MAX_CHARS];         // longitude when not using GPS
  int WeatherCheckRate;              // how often to check weather
  bool AmericanDate;                 // use American date format (Fri Oct 31)?
  bool VibrateOnDisc;                // vibrate on bluetooth disconnect?
  // Temperature Settings
  bool TemperatureMetric;          // Celsius or Fahrenheit?
  int Temperature0;                // Freezing temperature
  int Temperature1;                // Cold temperature
  int Temperature2;                // Chilly temperature
  int Temperature3;                // Warm temperature
  int Temperature4;                // Hot temperature
} ClaySettings;

static const uint32_t SUNNY_WITCHES[] = {
  RESOURCE_ID_IMAGE_WITCH_FREEZING,
  RESOURCE_ID_IMAGE_WITCH_COLD,
  RESOURCE_ID_IMAGE_WITCH_CHILLY,
  RESOURCE_ID_IMAGE_WITCH_WARM,
  RESOURCE_ID_IMAGE_WITCH_HOT,
};

static const uint32_t RAINY_WITCHES[] = {
  RESOURCE_ID_IMAGE_WITCH_FREEZING_UMBRELLA,
  RESOURCE_ID_IMAGE_WITCH_COLD_UMBRELLA,
  RESOURCE_ID_IMAGE_WITCH_CHILLY_UMBRELLA,
  RESOURCE_ID_IMAGE_WITCH_WARM_UMBRELLA,
  RESOURCE_ID_IMAGE_WITCH_HOT_UMBRELLA,
};

static const uint32_t CATS[] = {
  RESOURCE_ID_IMAGE_CAT_STANDING,
  RESOURCE_ID_IMAGE_CAT_SITTING,
  RESOURCE_ID_IMAGE_CAT_LOAFING,
  RESOURCE_ID_IMAGE_CAT_SLEEPING,
};