#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

// For the display
U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 8);
int displayWidth = 128;
int displayHeight = 64;

const uint8_t* numberFonts[] = {
  u8g2_font_spleen16x32_mn,
  u8g2_font_helvR24_tn,
  u8g2_font_maniac_tn,
  u8g2_font_logisoso22_tn,
  u8g2_font_mystery_quest_28_tn,
  u8g2_font_luBS19_tn 
};
const char* numberFontNames[] = {
  "Spleen",
  "Helvetica",
  "Maniac",
  "Logisoso",
  "Mystery Quest",
  "Lucida"
};
const int MaxNumberFonts = sizeof(numberFonts) / sizeof(numberFonts[0]);

const uint8_t* fullCharFonts[] = {
  u8g2_font_helvR14_tf,
  u8g2_font_6x10_mr,
  u8g2_font_logisoso16_tr,
  u8g2_font_ncenB14_tr,
  u8g2_font_fur20_tr,
  u8g2_font_10x20_tr,
  u8g2_font_mystery_quest_28_tr,
  u8g2_font_maniac_tr
};
const char* fullCharFontNames[] = {
  "Helvetica",
  "6x10",
  "Logisoso",
  "New Century Schoolbook",
  "Free Universal",
  "10x20",
  "Mystery Quest",
  "Maniac"
};
const int MaxFullCharFonts = sizeof(fullCharFonts) / sizeof(fullCharFonts[0]);

const uint8_t* binaryFont = u8g2_font_6x10_mr;
const uint8_t* settingsFont = u8g2_font_6x10_mr;

void drawSplashScreen();

void setupDisplay() {
  u8g2.begin();

}

void drawDisplay() {

}

void displaySplashScreen() {
  // List of status messages to display
  std::vector<String> statusMessages = {
    "Initializing...",
    "Loading modules...",
    "Starting services..."
  };

  u8g2.setFont(u8g2_font_nine_by_five_nbp_tr);
 
  // Display each status message
  for (size_t i = 0; i < statusMessages.size(); ++i) {
    u8g2.firstPage();
    do {
      for (size_t j = 0; j <= i; ++j) {
        u8g2.drawStr(0, u8g2.getMaxCharHeight() * (j + 1) + 4 * j, statusMessages[j].c_str());
      }
    } while (u8g2.nextPage());

    delay(500);
  }

  delay(500);
}