// Serial Pass Through
// Copyright(c) 2024 washishi

#include <M5Unified.h>
#if defined(ARDUINO_M5STACK_CORES3)
  #include <gob_unifiedButton.hpp>
  goblib::UnifiedButton unifiedButton;
#endif

const uint32_t baud[]={9600,38400,57600,76800,115200,128000,250000,500000,1000000,2000000,3000000,4000000,4500000}; // 通信速度
uint8_t usb_baud = 8; 
uint8_t port_baud = 8;

// uart_gpio[0:USB 1:PortA 2:PortB 3:PortC][0:RX 1:TX]
#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
const uint8_t uart_gpio[3][2]={{21,22},{36,26},{16,17}}; 
#elif ARDUINO_M5STACK_Core2
const uint8_t uart_gpio[3][2]={{33,32},{36,26},{13,14}};
#elif ARDUINO_M5STACK_CORES3
const uint8_t uart_gpio[4][2]={{1,2},{8,9},{18,17}};
#endif
uint8_t port = 0;   // 0:PortA 1:PortB 2:PortC 
bool ec = true;     // Echo Cancel

// serial setup
void serial_setup(){
  Serial.end();
  Serial1.end();
  delay( 1000 );  // wait
  Serial.begin(baud[usb_baud]);   // USB-Serial for PC
  Serial1.begin(baud[port_baud], SERIAL_8N1, uart_gpio[port][0], uart_gpio[port][1]); // for Target
}

// Pass Through  without echo cancel
void start_pass_through(){
  serial_setup();
  while(true){
    // PC to Target
    if (Serial.available()) {
      Serial1.write(Serial.read());
      Serial1.flush();  // wait send
    }
    // Target to PC
    if (Serial1.available()) {
      Serial.write(Serial1.read());
      Serial.flush();   // wait send
    }
  }
}

// Pass Through  with echo cancel
void start_pass_through_ec(){
  serial_setup();
  while(true){
    // PC to Target
    if (Serial.available()) {
      Serial1.write(Serial.read());
      Serial1.flush();  // wait send
      Serial1.read();   // echo cancel
    }
    // Target to PC
    if (Serial1.available()) {
      Serial.write(Serial1.read());
      Serial.flush();   // wait send
    }
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
#ifdef ARDUINO_M5STACK_CORE   
  M5.In_I2C.release();
#endif 
#if defined( ARDUINO_M5STACK_CORES3 )
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif 
  M5.Display.setTextSize(2);
  M5.Display.print("Serial Pass Through ");
  M5.Display.setTextColor(TFT_GREEN,TFT_BLACK);
  M5.Display.print("Config\n\n");
  M5.Display.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Display.print("  USB    :         bps\n");
  M5.Display.print("     Echo Cancel :\n");
  M5.Display.print("  Port   :         bps\n\n");
  M5.Display.print(" Config Mode Usage\n\n");
  M5.Display.print("  A:Click: USB baudrate\n");
  M5.Display.print("  A:Hold : Echo Cansel\n\n");
  M5.Display.print("  B:Click: Port baudrate\n");
  M5.Display.print("  B:Hold : Port Select\n\n");
  M5.Display.print("  C:Hold : Execute\n");
}

void loop() {
  M5.update();
#if defined( ARDUINO_M5STACK_CORES3 )
  unifiedButton.update();
#endif

  // refresh screen
  M5.Display.setCursor(19*12,3*16);
  if (ec){
    M5.Display.setTextColor(TFT_GREEN,TFT_BLACK);
    M5.Display.print("Enable ");
  } else {
    M5.Display.setTextColor(TFT_DARKGREEN,TFT_BLACK);
    M5.Display.print("Disable");
  }
  M5.Display.setTextColor(TFT_GREEN,TFT_BLACK);
  M5.Display.setCursor(11*12,2*16);
  M5.Display.printf("%7d",baud[usb_baud]);
  M5.Display.setCursor(11*12,4*16);
  M5.Display.printf("%7d",baud[port_baud]);
  M5.Display.setCursor(6*12,4*16);
  switch(port){
    case 0:
      M5.Display.setTextColor(TFT_BLACK,TFT_RED);
    break;
    case 1:
      M5.Display.setTextColor(TFT_GREEN,TFT_BLACK);
    break;
    case 2:
      M5.Display.setTextColor(TFT_BLACK,TFT_CYAN);
    break;
    default:
      M5.Display.setTextColor(TFT_GREEN,TFT_BLACK);
  }
  M5.Display.printf(" %c ",0x41 + port, baud[port_baud]);

  // button action
  if (M5.BtnA.wasHold()){
    M5.Display.setCursor(11*12,2*16);
    if (ec){
      ec = false;
    } else {
      ec = true;
    }
  } else if (M5.BtnA.wasReleased() && !M5.BtnA.wasReleasedAfterHold()){
    usb_baud ++;
    if (usb_baud == sizeof(baud) / sizeof(uint32_t)){
      usb_baud = 0;
    }
  }
  if (M5.BtnB.wasHold()){
      port ++;
      if (port == sizeof(uart_gpio) / sizeof(uint8_t) /2) {
        port = 0;
      }
  } else if (M5.BtnB.wasReleased() && !M5.BtnB.wasReleasedAfterHold()){
    port_baud ++;
    if (port_baud == sizeof(baud) / sizeof(uint32_t)){
      port_baud = 0;
    }
  }
  if (M5.BtnC.wasHold()){
     M5.Display.setCursor(20*12,0*16);
     M5.Display.setTextColor(TFT_RED,TFT_BLACK);
     M5.Display.print(" Exec ");
    // start
    if(ec){
      start_pass_through_ec();
    } else {
      start_pass_through();
    }
  } 
}
