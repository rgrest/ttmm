#include "TimeTools.h"

#ifdef RUN_TIMER
ttmm::Timer::Map ttmm::Timer::timer;
std::string ttmm::Timer::filename;
std::ofstream ttmm::Timer::outputStream;

ttmm::Timer::~Timer() {

  if (Timer::outputStream.good() && runtimes.size() > 0) {

    auto den = Clock::duration::period::den;
    auto unit = unitNameFromPeriod(Clock::duration::period::num, den);
    auto total =
        double(std::accumulate(std::begin(runtimes), std::end(runtimes),
                               std::chrono::high_resolution_clock::duration(0))
                   .count());
    auto total_ms = total / den * knownUnits.at("milliseconds");
    auto average = total / runtimes.size();
    auto average_ms = average / den * knownUnits.at("milliseconds");

    Timer::outputStream << std::setfill('-') << std::setw(60) << "-"
                        << std::endl
                        << id << std::endl
                        << std::setfill('-') << std::setw(60) << "-"
                        << std::endl;
    Timer::outputStream << std::setw(15) << std::setfill(' ') << std::left
                        << " - Total" << total << " " << unit << " ("
                        << total_ms << "ms)" << std::endl
                        << std::setw(15) << std::setfill(' ') << std::left
                        << " - Runs" << runtimes.size() << std::endl
                        << std::setw(15) << std::setfill(' ') << std::left
                        << " - Average" << average << " " << unit << " ("
                        << average_ms << "ms)" << std::endl
                        << std::endl;
  }
}

void ttmm::Timer::start(char const *id) {
  auto t = Timer::timer.find(id);
  if (t == std::end(Timer::timer)) {
    Timer::timer[id] = new Timer(id);
  }
  if (Timer::timer[id]->running) {
    throw GeneralException(std::string("Timer ") + id + " is already running.");
  }
  Timer::timer[id]->lastStartPoint = std::chrono::high_resolution_clock::now();
  Timer::timer[id]->running = true;
}

void ttmm::Timer::stop(char const *id) {
  Timer::timer[id]->runtimes.emplace_back(Clock::now() -
                                          Timer::timer[id]->lastStartPoint);

  Timer::timer[id]->running = false;
}

void ttmm::Timer::setFilename(char const *name) {
  Timer::filename = name;
  Timer::outputStream = std::ofstream((Timer::filename + ".txt").c_str(),
                                      std::ios::out | std::ios::trunc);
}

void ttmm::Timer::done() {

  for (auto& it : Timer::timer) {
    delete it.second;
  }
  Timer::outputStream.close();
}

#endif
