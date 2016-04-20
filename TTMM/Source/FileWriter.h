#pragma once

#include <string>
#include <fstream>

namespace ttmm {

#if !defined LOGFILE_NAME
#error "To use the filewriter, define a LOGFILE_NAME"
#endif

class FileWriter {
public:
  static FileWriter instance;

  FileWriter() = delete;
  FileWriter(FileWriter const&) = delete;
  FileWriter& operator=(FileWriter const&) = delete;

  ~FileWriter();

  void write(std::string text);
  // write a string with the double parameter
  void write(std::string text, double number);
  // write a string with the int parameter
  void write(std::string text, int number);
  // write a NoteOn of MidiEvent
  void write(std::string text, int nr1, int nr2, int nr3, double time, int nr4);
  // write a NoteOff of MidiEvent
  void write(std::string text, int nr1, int nr2, double time, int nr3);
  // write a xxxSignature Event or a string with two integer parameters
  void write(std::string text, int nr1, int nr2);

  void write(std::string text, int nr1, int nr2, int nr3);
private:
  explicit FileWriter(std::string filename);

  std::ofstream out;

  // create a backup of last file if it can be opened
  void rotate(std::string const &filename);
};

static FileWriter &logger = FileWriter::instance; ///< globally accessible ref
/// to the singleton logger instance

//==================================================================================
// The following are provided to keep the usages of a previous filewriter
// working
static FileWriter *logfileMusic =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
static FileWriter *logfileMetronom =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
static FileWriter *logfileMidiReader =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
static FileWriter *logfileMidiHandler =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
static FileWriter *logfileDrum =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
static FileWriter *logfileKinect =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
static FileWriter *logfileGeneral =
    &FileWriter::instance; ///< @deprecated Just use ttmm::logger instead
}
