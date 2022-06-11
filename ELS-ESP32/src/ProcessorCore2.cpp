#include <functions.h>


void setup2() {
}


void loop2() {
    delay(100);
}


void runOnProcessorCore2(void *parameter) {
  setup2();
  for (;;) {
    loop2();
  }
}
