struct DataSet {
  TinyGPSLocation location;
  TinyGPSAltitude altitude;
  TinyGPSDate date;
  TinyGPSTime time;
  Vector<uint8_t> sensorValues;
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
    void setFileName();
    virtual void init() = 0;
    virtual void writeHeader() = 0;
    virtual void writeData(DataSet*) = 0;

  protected:
    String m_fileExtension;
    String m_filename;

  private:

};

class CSVFileWriter : public FileWriter
{
  public:
    CSVFileWriter() : FileWriter() {
      m_fileExtension = ".csv";
    }
    ~CSVFileWriter() {}
    void init()
    {
    }
    void writeHeader();
    void writeData(DataSet*);
  protected:
};

class GPXFileWriter : public FileWriter
{
  public:
    GPXFileWriter() : FileWriter() {
      m_fileExtension = ".gpx";
    }
    ~GPXFileWriter() {}
  protected:
  private:
};
