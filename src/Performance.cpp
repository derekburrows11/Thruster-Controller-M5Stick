#include "config.h"
#include "Performance.h"



void Performance::init() {
  msTimeStart = millis();
}

void Performance::print() {
  Serial.printf("%d in %ld ms, average M5.update() took %.2f microseconds\n",
      loops, millis() - msTimeStart, (float)((millis() - msTimeStart) * 1000) / loops);
  msTimeStart = millis();
  loops = 0;
}

void Performance::memory_logStats() {
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Min Free heap: %d\n", ESP.getMinFreeHeap());

    Serial.printf("Free PSRAM heap: %d\n", ESP.getFreePsram());
    Serial.printf("uptime: %ld\n", millis() / 1000);

    log_i("Free heap: %d", ESP.getFreeHeap());
    log_i("Free PSRAM heap: %d", ESP.getFreePsram());
    log_i("uptime: %d", millis() / 1000 );
}



