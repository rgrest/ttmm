#include <iomanip>

#include "FileWriter.h"

ttmm::FileWriter ttmm::FileWriter::instance(LOGFILE_NAME);

ttmm::FileWriter::FileWriter(std::string filename)
{
    rotate(filename);
    // appending to file.
    out.open(filename, std::ios::app | std::ios::ate | std::ios::out);
}

void ttmm::FileWriter::rotate(std::string const& filename)
{
    auto f = std::ifstream(filename);
    auto couldOpen = f.good(); // not sure that not existing else
    f.close();

    if (couldOpen)
    {
        auto lastName = filename + ".last";
        std::remove(lastName.c_str());
        std::rename(filename.c_str(), lastName.c_str());
        std::remove(filename.c_str());
    }
}

ttmm::FileWriter::~FileWriter() { out.close(); }

void ttmm::FileWriter::write(std::string text) { out << text << std::endl; }

void ttmm::FileWriter::write(std::string text, double number)
{
    out << text << " "
        << std::setprecision(std::numeric_limits<double>::digits10)
        << number << std::endl;
}

void ttmm::FileWriter::write(std::string text, int number)
{
    out << text << " " << number << std::endl;
}

void ttmm::FileWriter::write(std::string text, int nr1, int nr2)
{
    out << text << "\t" << nr1 << "\t" << nr2 << std::endl;
}

void ttmm::FileWriter::write(std::string text, int nr1, int nr2, int nr3)
{
    out << text << "\t" << nr1 << "\t" << nr2 << "\t" << nr3 << std::endl;
}

void ttmm::FileWriter::write(std::string text, int nr1, int nr2, double time, int nr3)
{
    out << text << " " << nr1 << " " << nr2 << " --- Timestamp:" << time << " --- Sample position: " << nr3
        << std::endl;
}

void ttmm::FileWriter::write(std::string text, int nr1, int nr2, int nr3,
    double time, int nr4)
{
    out << text << " " << nr1 << " " << nr2 << " " << nr3 << " --- Timestamp:" << time
        << " --- Sample position: " << nr4 << std::endl;
}
