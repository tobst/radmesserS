void FileWriter::listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void FileWriter::createDir(fs::FS &fs, const char * path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void FileWriter::removeDir(fs::FS &fs, const char * path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void FileWriter::readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void FileWriter::writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void FileWriter::appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void FileWriter::renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void FileWriter::deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void FileWriter::setFileName() {
  int fileSuffix = 0;
  String base_filename = "/sensorData";
  m_filename = base_filename + String(fileSuffix) + m_fileExtension;
  while (SD.exists(filename.c_str()))
  {
    fileSuffix++;
    m_filename = base_filename + String(fileSuffix) + m_fileExtension;
  }
}

void CSVFileWriter::writeHeader() {
  String headerString;
  headerString += "Date;Time;Latitude;Longitude";
  for (size_t idx = 0; idx < sensorNames.size(); ++idx)
  {
    headerString += ";" + sensorNames[idx];
  }
  this->appendFile(SD, m_filename.c_str(), headerString.c_str() );
}

void CSVFileWriter::writeData(DataSet* set) {
  //2019-07-16T09:12:46Z
  String dataString;

  char dateString[9];
  sprintf(dateString, "%02d%02d%04d;", set->date.day(), set->date.month(), set->date.year());
  dataString += dateString;

  char timeString[9];
  sprintf(timeString, "%02d:%02d:%02d;", set->time.hour(), set->time.minute(), set->time.second());
  dataString += timeString;

  String latitudeString = String(set->location.lat(), 6);
  dataString += latitudeString + ";";

  String longitudeString = String(set->location.lng(), 6);
  dataString += longitudeString;

  for (size_t idx = 0; idx < set->sensorValues.size(); ++idx)
  {
    dataString += ";" + String(set->sensorValues[idx]);
  }

  this->appendFile(SD, m_filename.c_str(), dataString.c_str() );
}


