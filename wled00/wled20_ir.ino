/*
 * Infrared sensor support for generic 24 key RGB remote
 */

#if defined(WLED_DISABLE_INFRARED) || defined(ARDUINO_ARCH_ESP32)
void handleIR(){}
#else

IRrecv* irrecv;
//change pin in NpbWrapper.h

decode_results results;

unsigned long irCheckedTime = 0;
uint32_t lastValidCode = 0;
uint16_t irTimesRepeated = 0;

//relatively change brightness, minumum A=5
void relativeChange(byte* property, int8_t amount, byte lowerBoundary =0)
{
  int16_t new_val = (int16_t) *property + amount;
  if (new_val > 0xFF) new_val = 0xFF;
  else if (new_val < lowerBoundary) new_val = lowerBoundary;
  *property = new_val;
}


void decodeIR(uint32_t code)
{
  if (code == 0xFFFFFFFF) //repeated code, continue brightness up/down
  {
    irTimesRepeated++;
    if (lastValidCode == IR24_BRIGHTER || lastValidCode == IR17_BRIGHTER)
    { 
      relativeChange(&bri, 10); colorUpdated(2);
    }
    else if (lastValidCode == IR24_DARKER || lastValidCode == IR17_DARKER)
    {
      relativeChange(&bri, -10, 5); colorUpdated(2);
    }
    else if (lastValidCode == IR17_QUICK)
    {
      relativeChange(&effectSpeed, 10); colorUpdated(2);
    }
    else if (lastValidCode == IR17_SLOW)
    {
      relativeChange(&effectSpeed, -10, 5); colorUpdated(2);
    }
    else if ((lastValidCode == IR24_ON || lastValidCode == IR17_ONOFF) && irTimesRepeated > 7)
    {
      nightlightActive = true;
      nightlightStartTime = millis();
      colorUpdated(2);
    }
    else if (lastValidCode == IR17_FAV1 && irTimesRepeated > 7)   savePreset(1);
    else if (lastValidCode == IR17_FAV2 && irTimesRepeated > 7)   savePreset(2);
    else if (lastValidCode == IR17_FAV3 && irTimesRepeated > 7)   savePreset(3);
    else if (lastValidCode == IR17_FAV4 && irTimesRepeated > 7)   savePreset(4);
    else if (lastValidCode == IR17_FAV5 && irTimesRepeated > 7)   savePreset(5);
    else if (lastValidCode == IR17_FAV6 && irTimesRepeated > 7)   savePreset(6);
      
   
    return;
  }
  lastValidCode = 0; irTimesRepeated = 0;

  if      (code > 0xFFFFFF) return; //invalid code
  else if (code > 0xFF0000) decodeIR17(code); //is in 17-key remote range
  else if (code > 0xF70000 && code < 0xF80000) decodeIR24(code); //is in 24-key remote range
  //code <= 0xF70000 also invalid
}

void decodeIR17(uint32_t code)
{
  switch (code) {

    case IR17_SW_ONOFF  : toggleSwitch();                   break;
    case IR17_ONOFF     : toggleOnOff(); colorUpdated(1);   break;
    case IR17_BRIGHTER  : relativeChange(&bri, 10);         break;
    case IR17_DARKER    : relativeChange(&bri, -10, 5);     break;
    case IR17_RNDCOLLOR : _setRandomColor(false);           break;
    case IR17_FAV1      : if (!applyPreset(1)) colorFromUint32(COLOR_RED);        break;
    case IR17_FAV2      : if (!applyPreset(2)) colorFromUint32(COLOR_GREEN);      break;
    case IR17_FAV3      : if (!applyPreset(3)) colorFromUint32(COLOR_BLUE);       break;
    case IR17_FAV4      : if (!applyPreset(4)) colorFromUint32(COLOR_MAGENTA);    break;
    case IR17_FAV5      : if (!applyPreset(5)) colorFromUint32(COLOR_WHITE);      break;
    case IR17_FAV6      : if (!applyPreset(6)) colorFromUint32(COLOR_YELLOWISH);  break;
    case IR17_QUICK     : relativeChange(&effectSpeed, 10);                       break;
    case IR17_SLOW      : relativeChange(&effectSpeed, -10, 5);                   break;
    case IR17_FX_PREV   : effectCurrent -= 1; if (effectCurrent < 0) effectCurrent = MODE_COUNT-1;  break;
    case IR17_FX_NEXT   : effectCurrent += 1; if (effectCurrent >= MODE_COUNT) effectCurrent = 0;   break;
    case IR17_PAL_NEXT  : effectPalette += 1; if (effectPalette >= strip.getPaletteCount()) effectPalette = 0;   break;
    case IR17_PAL_PREV  : effectPalette -= 1; if (effectPalette < 0) effectPalette = strip.getPaletteCount()-1;  break;
    default: return;
  }
  lastValidCode = code;
  colorUpdated(2); //for notifier, IR is considered a button input
}

void decodeIR24(uint32_t code)
{
  switch (code) {
    case IR24_BRIGHTER  : relativeChange(&bri, 10);         break;
    case IR24_DARKER    : relativeChange(&bri, -10, 5);     break;
    case IR24_OFF       : briLast = bri; bri = 0;           break;
    case IR24_ON        : bri = briLast;                    break;
    case IR24_RED       : colorFromUint32(COLOR_RED);       break;
    case IR24_REDDISH   : colorFromUint32(COLOR_REDDISH);   break;
    case IR24_ORANGE    : colorFromUint32(COLOR_ORANGE);    break;
    case IR24_YELLOWISH : colorFromUint32(COLOR_YELLOWISH); break;
    case IR24_YELLOW    : colorFromUint32(COLOR_YELLOW);    break;
    case IR24_GREEN     : colorFromUint32(COLOR_GREEN);     break;
    case IR24_GREENISH  : colorFromUint32(COLOR_GREENISH);  break;
    case IR24_TURQUOISE : colorFromUint32(COLOR_TURQUOISE); break;
    case IR24_CYAN      : colorFromUint32(COLOR_CYAN);      break;
    case IR24_AQUA      : colorFromUint32(COLOR_AQUA);      break;
    case IR24_BLUE      : colorFromUint32(COLOR_BLUE);      break;
    case IR24_DEEPBLUE  : colorFromUint32(COLOR_DEEPBLUE);  break;
    case IR24_PURPLE    : colorFromUint32(COLOR_PURPLE);    break;
    case IR24_MAGENTA   : colorFromUint32(COLOR_MAGENTA);   break;
    case IR24_PINK      : colorFromUint32(COLOR_PINK);      break;
    case IR24_WHITE     : colorFromUint32(COLOR_WHITE);           effectCurrent = 0;  break;
    case IR24_FLASH     : if (!applyPreset(1)) effectCurrent = FX_MODE_COLORTWINKLE;  break;
    case IR24_STROBE    : if (!applyPreset(2)) effectCurrent = FX_MODE_RAINBOW_CYCLE; break;
    case IR24_FADE      : if (!applyPreset(3)) effectCurrent = FX_MODE_BREATH;        break;
    case IR24_SMOOTH    : if (!applyPreset(4)) effectCurrent = FX_MODE_RAINBOW;       break;
    default: return;
  }
  lastValidCode = code;
  colorUpdated(2); //for notifier, IR is considered a button input
}


void decodeIR44(uint32_t code)
{
  //not implemented for now
}


void initIR()
{
  if (irEnabled)
  {
    irrecv = new IRrecv(IR_PIN);
    irrecv->enableIRIn();
  }
}


void handleIR()
{
  if (irEnabled && millis() - irCheckedTime > 120)
  {
    irCheckedTime = millis();
    if (irEnabled)
    {
      if (irrecv == NULL)
      { 
        initIR(); return;
      }
      
      if (irrecv->decode(&results))
      {
        Serial.print("IR recv\r\n0x");
        Serial.println((uint32_t)results.value, HEX);
        Serial.println();
        decodeIR(results.value);
        irrecv->resume();
      }
    } else if (irrecv != NULL)
    {
      irrecv->disableIRIn();
      delete irrecv; irrecv = NULL;
    }
  }
}

#endif
