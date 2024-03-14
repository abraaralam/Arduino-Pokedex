// Include Libraries
#include <SPI.h> // f.k. for Arduino-1.5.2
#include <SD.h>
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include <Adafruit_TFTLCD.h>  // Hardware-specific library
#include <Adafruit_ILI9341.h> // Hardware-specific library

MCUFRIEND_kbv tft;

#define YP A3 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 9  // can be a digital pin
#define XP 8  // can be a digital pin

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define SD_CS 10

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define WHITE 0xFFFF

#define MINPRESSURE 5
#define MAXPRESSURE 1000

#define TS_MINX 100
#define TS_MAXX 920

#define TS_MINY 70
#define TS_MAXY 900

int current_pokemon;

// First time
void setup()
{
  // Setting up display
  uint16_t ID;
  ID = tft.readID();
  Serial.println(ID, HEX);
  if (ID == 0x0D3D3)
    ID = 0x9481;
  tft.begin(ID);
  tft.fillScreen(WHITE);

  SD.begin(SD_CS); // turning on SD card

  tft.setRotation(1);  // horizontal
  current_pokemon = 0; // starts with bulbasaur

  // Display bulbasaur name and image
  pokeDesc(current_pokemon);
  pokeImage(current_pokemon);
}

void loop()
{
  // Finding if screen was touched, storing touched value
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT); // output pins to change screen
  pinMode(YP, OUTPUT);

  // if screen touch registered
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {

    // scale from 0->1023 to tft.width
    int16_t nTmpX = p.x;
    p.x = map(p.y, TS_MINY, TS_MAXY, 0, 320);
    p.y = map(nTmpX, TS_MINX, TS_MAXX, 0, 240);

    if (p.y > 200)
    { // if bottom of screen touched
      if (p.x > 220)
      { // if right screen touched

        if (current_pokemon < 150)
        { // loop if reach the end
          current_pokemon++;
        }
        else
        {
          current_pokemon = 0;
        }
      }
      else if (p.x < 100)
      {
      }
      else if (current_pokemon > 0)
      { // loop if reach the beginning
        current_pokemon--;
      }
      else
      {
        current_pokemon = 150;
      }
    }

    // display current pokemon
    pokeDesc(current_pokemon);
    pokeImage(current_pokemon);
  }
}
delay(100);
}

// 0) Normal: 0xa54f | Normal*: 0x6b69
// 1) Fire: 0xec06 | Fire*: 0x6b69
// 2) Fighting: 0xb985 | Fighting*: 0x7903
// 3) Water: 0x6c9d | Water*: 0x42f2
// 4) Flying: 0xa49d | Flying*: 0x6af3
// 5) Grass: 0x7e2a | Grass*: 0x4c06
// 6) Poison: 0x9a13 | Poison*: 0x694d
// 7) Electric: 0xf666 | Electric*: 0x694d
// 8) Ground: 0xf666 | Ground*: 0x93e8
// 9) Psychic: 0xf2d1 | Psychic*: 0x99cb
// 10) Rock: 0xf2d1 | Rock*: 0x7324
// 11) Ice: 0xf2d1 | Ice*: 0x6471
// 12) Bug: 0xa5a4 | Bug*: 0x6ba2
// 13) Dragon: 0x71de | Dragon*: 0x4935
// 14) Ghost: 0x71de | Ghost*: 0x49cc
// 15) Dark: 0x72c9 | Dark*: 0x49c5
// 16) Steel: 0x72c9 | Steel*: 0x7bd1

// Color scheme, reads number from sd card text file, returns corresponding light or dark colour scheme
const uint16_t light[] = {0xa54f, 0xec06, 0xb985, 0x6c9d, 0xa49d, 0x7e2a, 0x9a13, 0xf666, 0xf666, 0xf2d1, 0xf2d1, 0xf2d1, 0xa5a4, 0x71de, 0x71de, 0x72c9, 0x72c9};
const uint16_t dark[] = {0x6b69, 0x6b69, 0x7903, 0x42f2, 0x6af3, 0x4c06, 0x694d, 0x694d, 0x93e8, 0x99cb, 0x7324, 0x6471, 0x6ba2, 0x4935, 0x49cc, 0x49c5, 0x7bd1};

// Color scheme function, takes number from text file and gives corresponding hex color
uint16_t color(int current_pokemon, int lightdark)
{
  if (lightdark == 1)
  {
    return light[SD_Data(current_pokemon, 7).toInt()];
  }
  else if (lightdark == 2)
  {
    return dark[SD_Data(current_pokemon, 7).toInt()];
  }
}

// draw the image and border
void pokeImage(int current_pokemon)
{
  tft.fillRoundRect(170, 50, 140, 140, 12, WHITE);
  tft.drawRoundRect(170, 50, 140, 140, 12, color(current_pokemon + 1, 2));
  bmpDraw(current_pokemon + 1, 175, 55);
}

// Define the array to store the variables for each line
String SD_Data(int current_pokemon, int infoNum)
{
  File dataFile = SD.open(String(current_pokemon) + "_" + String(infoNum) + ".txt");
  String word = ""; // Variable to store the word

  if (dataFile)
  {
    while (dataFile.available())
    {
      char c = dataFile.read(); // Read each character

      // Check if the character is a letter or a space
      if (isAlpha(c) || c == ' ' || isdigit(c) || c == '\'' || c == '\"')
      {
        word += c; // Append the character to the word
      }
      dataFile.close();
    }
    return word;
  }

  // Description, shows number, title, description, etc.
  void pokeDesc(int current_pokemon)
  {
    tft.fillRect(0, 0, 320, 45, color(current_pokemon + 1, 2));
    tft.setTextColor(WHITE);
    tft.setTextSize(2);

    tft.setCursor(10, 10);
    tft.print(String(current_pokemon + 1));
    tft.print(". ");
    tft.print(SD_Data(current_pokemon + 1, 1));
    tft.println("");
    tft.setTextSize(1);
    tft.print(SD_Data(current_pokemon + 1, 2));

    String descInfo = SD_Data(current_pokemon + 1, 3);

    int wrapWidth = 25;
    int startIdx = 0;
    int endIdx = wrapWidth;
    int pixelY = 50;
    tft.setTextColor(color(current_pokemon + 1, 2));
    tft.setTextSize(1);
    tft.fillRect(0, 45, 320, 155, color(current_pokemon + 1, 1));

    while (startIdx < descInfo.length())
    {
      // Find the last space within the wrap width
      while (endIdx < descInfo.length() && descInfo[endIdx] != ' ')
      {
        endIdx--;
      }

      // If no space found, wrap at the wrap width
      if (endIdx == startIdx + wrapWidth)
      {
        endIdx = startIdx + wrapWidth - 1;
      }

      // Print the wrapped line
      tft.setCursor(10, pixelY);

      for (int i = startIdx; i <= endIdx; i++)
      {
        tft.print(descInfo[i]);
      }
      pixelY += 14;

      // Update start and end indices for the next line
      startIdx = endIdx + 1;
      endIdx = startIdx + wrapWidth;
      if (endIdx > descInfo.length())
      {
        endIdx = descInfo.length();
      }
    }
    tft.fillRect(0, 200, 320, 40, color(current_pokemon + 1, 2));
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(10, 215);
    tft.print("Prev");
    tft.setCursor(265, 215);
    tft.print("Next");
  }

// SD Card select
#define BUFFPIXEL 20
  File root;
  char namebuf[32];
  int pathlen;
  uint8_t spi_save;

  void bmpDraw(int number, int x, int y)
  {
    String fileName = String(number) + ".bmp";
    const char *filename = fileName.c_str();
    File bmpFile;
    int bmpWidth, bmpHeight;            // W+H in pixels
    uint8_t bmpDepth;                   // Bit depth (currently must be 24)
    uint32_t bmpImageoffset;            // Start of image data in file
    uint32_t rowSize;                   // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[BUFFPIXEL];      // pixel out buffer (16-bit per pixel)
    uint8_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
    boolean goodBmp = false;            // Set to true on valid header parse
    boolean flip = true;                // BMP is stored bottom-to-top
    int w, h, row, col;
    uint8_t r, g, b;
    uint32_t pos = 0, startTime = millis();
    uint8_t lcdidx = 0;
    boolean first = true;

    SPCR = spi_save;
    bmpFile = SD.open(filename);

    // Parse BMP header
    if (read16(bmpFile) == 0x4D42)
    { // BMP signature
      Serial.print(F("File size: "));
      Serial.println(read32(bmpFile));
      (void)read32(bmpFile);            // Read & ignore creator bytes
      bmpImageoffset = read32(bmpFile); // Start of image data
      Serial.print(F("Image Offset: "));
      Serial.println(bmpImageoffset, DEC);
      // Read DIB header
      Serial.print(F("Header size: "));
      Serial.println(read32(bmpFile));
      bmpWidth = read32(bmpFile);
      bmpHeight = read32(bmpFile);
      if (read16(bmpFile) == 1)
      {                             // # planes -- must be '1'
        bmpDepth = read16(bmpFile); // bits per pixel
        Serial.print(F("Bit Depth: "));
        Serial.println(bmpDepth);
        if ((bmpDepth == 24) && (read32(bmpFile) == 0))
        { // 0 = uncompressed

          goodBmp = true; // Supported BMP format -- proceed!
          Serial.print(F("Image size: "));
          Serial.print(bmpWidth);
          Serial.print('x');
          Serial.println(bmpHeight);

          // BMP rows are padded (if needed) to 4-byte boundary
          rowSize = (bmpWidth * 3 + 3) & ~3;

          // If bmpHeight is negative, image is in top-down order.
          // This is not canon but has been observed in the wild.
          if (bmpHeight < 0)
          {
            bmpHeight = -bmpHeight;
            flip = false;
          }

          // Crop area to be loaded
          w = bmpWidth;
          h = bmpHeight;
          if ((x + w - 1) >= tft.width())
            w = tft.width() - x;
          if ((y + h - 1) >= tft.height())
            h = tft.height() - y;

          // Set TFT address window to clipped image bounds
          SPCR = 0;
          tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

          for (row = 0; row < h; row++)
          {

            if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + row * rowSize;
            SPCR = spi_save;
            if (bmpFile.position() != pos)
            { // Need seek?
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; col++)
            { // For each column...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer))
              { // Indeed
                // Push LCD buffer to the display first
                if (lcdidx > 0)
                {
                  SPCR = 0;
                  tft.pushColors(lcdbuffer, lcdidx, first);
                  lcdidx = 0;
                  first = false;
                }
                SPCR = spi_save;
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
              }

              // Convert pixel from BMP to TFT format
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              lcdbuffer[lcdidx++] = tft.color565(r, g, b);
            } // end pixel
          }   // end scanline
          // Write any remaining data to LCD
          if (lcdidx > 0)
          {
            SPCR = 0;
            tft.pushColors(lcdbuffer, lcdidx, first);
          }
          Serial.print(F("Loaded in "));
          Serial.print(millis() - startTime);
          Serial.println(" ms");
        } // end goodBmp
      }
    }

    bmpFile.close();
    if (!goodBmp)
      Serial.println("BMP format not recognized.");
  }

  // These read 16- and 32-bit types from the SD card file.
  // BMP data is stored little-endian, Arduino is little-endian too.
  // May need to reverse subscript order if porting elsewhere.

  uint16_t read16(File f)
  {
    Serial.println("read16");
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
  }

  uint32_t read32(File f)
  {
    Serial.println("read32");
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
  }
