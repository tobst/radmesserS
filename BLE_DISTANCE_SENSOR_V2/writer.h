struct GPSDateTime {
  int year;
  byte month,
       day,
       hour,
       minute,
       second,
       hundredths;
};

class FileWriter
{
  public:
    FileWriter() {}
    virtual ~FileWriter() {}
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    void createDir(fs::FS &fs, const char * path);
    void removeDir(fs::FS &fs, const char * path);
    void readFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void appendFile(fs::FS &fs, const char * path, const char * message);
    void renameFile(fs::FS &fs, const char * path1, const char * path2);
    void deleteFile(fs::FS &fs, const char * path);
    void setLatitude(double);
    void setLongitude(double);
    void setDateTime(GPSDateTime);
    virtual void init() = 0;

  protected:

  private:
    double m_latitude;
    double m_longitude;
};

class CSVFileWriter : public FileWriter
{
  public:
    CSVFileWriter() : FileWriter() {
    }
    ~CSVFileWriter() {}
    void init()
    {
      
    }
  protected:
};

class GPXFileWriter : public FileWriter
{
  public:
    GPXFileWriter() : FileWriter() {
    }
    ~GPXFileWriter() {}
  protected:
};
