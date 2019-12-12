void setHandleBarWidth(int width) {
  handleBarWidth = width;
  timeout = 15000 + (int)(handleBarWidth * 29.1 * 2);
  EEPROM.write(0, handleBarWidth);
  EEPROM.commit();
}

