#ifndef TTMM_TIME_H
#define TTMM_TIME_H

#include <iostream>
#include <ratio>
#include <chrono>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <iomanip>

#include "Types.h"
#include "ExceptionTypes.h"

namespace std {
    /**
     * Overload of @code min to compare some duration with a duration in seconds.
     * @param[in] seconds A duration given in seconds.
     * @param[in] dur1 A duration of arbitrary unit.
     * @return The minimum of @code seconds and @code dur1, expressed in units of @code Dur.
     */
    template<typename Dur>
    typename Dur min(std::chrono::duration<double> const& seconds, typename Dur const & dur1) {
        return std::min(dur1, duration_cast<Dur>(seconds));
    }

}

namespace ttmm {

using namespace std::chrono;

static_assert(Duration::period::den < std::pico::den,
              "The clock used for this project is not supported currently.");

namespace {

using UnitMap = std::map<std::string, intmax_t>;
static const UnitMap knownUnits{{"picoseconds", std::pico::den},
                                {"nanoseconds", std::nano::den},
                                {"microsecnods", std::micro::den},
                                {"milliseconds", std::milli::den},
                                {"centiseconds", std::centi::den},
                                {"deciseconds", std::deci::den},
                                {"seconds", 1}};
}

// The following is general code which works with any clock and duration,
// independent of
// the clock defined in Types.h and the clock provided to timeInfo_t (see
// below).
// This is necessary because the underlying units of the clocks and durations
// change
// while calculating with their values.

//
// Output routines
//

template <typename Clock>
inline intmax_t timestampAsNum(std::chrono::time_point<Clock> const &t) {
  return t.time_since_epoch().count();
}

template <class Clock>
inline std::string toString(std::chrono::time_point<Clock> const &timestamp) {
  return std::to_string(timestampAsNum(timestamp));
}

inline std::ostream &operator<<(std::ostream &os,
                                std::chrono::duration<double> const &dur) {
  os << dur;
  return os;
}

template <class Clock>
inline std::ostream &
operator<<(std::ostream &os, std::chrono::time_point<Clock> const &timestamp) {
  os << std::to_string(timestampAsNum(timestamp));
  return os;
}

inline const std::string unitNameFromPeriod(intmax_t num, intmax_t den) {
  auto unit = std::find_if(
      std::begin(knownUnits), std::end(knownUnits),
      [&](UnitMap::value_type const &it) { return it.second == den; });

  if ((num != 1) || unit == std::end(knownUnits)) {
    return "[(" + std::to_string(num) + "s)/" + std::to_string(den) + "]";
  }

  return unit->first;
}

//
// Arithmetics and conversions
//

template <typename Clock, typename Duration>
inline typename Clock::time_point
timeAfter(std::chrono::time_point<Clock> const &tp, Duration dur) {
  //return time_point_cast<typename Clock::duration>(tp + dur);
    return time_point<Clock>(tp + duration_cast<typename Clock::duration>(dur));
}


template <typename Clock>
inline typename Clock::time_point
timeAfter(std::chrono::time_point<Clock> const &tp, double secondsLater) {
    //return time_point_cast<typename Clock::duration>(tp + dur);
    return time_point<Clock>(tp + duration_cast<typename Clock::duration>(duration<double>(secondsLater)));
}

template <typename Clock, typename Duration>
inline typename Clock::time_point
timeBefore(std::chrono::time_point<Clock> const &tp, Duration dur) {
  //return time_point_cast<typename Clock::duration>(tp - dur);
    return time_point<Clock>(tp - duration_cast<typename Clock::duration>(dur));
}

template <typename Clock>
inline typename Clock::time_point
timeBefore(std::chrono::time_point<Clock> const &tp, double secondsBefore) {
    //return time_point_cast<typename Clock::duration>(tp - dur);
    return time_point<Clock>(tp - duration_cast<typename Clock::duration>(duration<double>(secondsBefore)));
}

inline std::chrono::duration<long double> noteLengthAtBPMInSeconds(double bpm,
                                                                   uint8_t noteUnit) {
  const auto lengthOfBar = double{60.0 / bpm * 4};
  return std::chrono::duration<long double>(lengthOfBar / noteUnit);
}

template <typename Clock>
typename Clock::duration inline durationBetweenTimepoints(
    std::chrono::time_point<Clock> const &time1,
    std::chrono::time_point<Clock> const &time2) {

    return (time1 > time2? time1 - time2 : time2 - time1);
}

/**
 * Provides information and utilities for Clock.
 * The singleton instance holds a timestamp @code timeZero which defaults to the
 * clocks
 * epoch-value (1.1.1970 for the @code system_clock, unspecified for the @code
 * high_resolution_clock).
 * @code resetToNow() resets the @code timeZero to @code now (as provided by the
 * configured clock).
 * The @code ms and @code s methods return the time passed since timeZero in
 * milliseconds
 * and seconds, respectively.
 * @see TimeInfo, which is a specialization for the Clock configured in Types.h.
 */
template <class Clock> class timeInfo_t {

public:
    using Timestamp = typename Clock::time_point;
    using Dur = typename Clock::duration;

  timeInfo_t(timeInfo_t const &) = delete;
  timeInfo_t &operator=(timeInfo_t const &) = delete;
  ~timeInfo_t() = default;

  /**
   * Reset the timeZero to @code now as provided by @code Clock.
   * @deprecated @code timeZero is set in the constructor.
   */
  void resetToNow() { timeZero = Clock::now(); }


  /**
  * Resets the zero timepoint to the provided timestamp
  * @param tp A @code time_point of the clock @Clock.
  */
  void setTimeZero(typename Clock::time_point tp) {
      timeZero = tp;
  }

  /**
  *
  */
  void setTimeZeroFromSeconds(double secondsLater) {
      timeZero = timeAfter(time_point<Clock>(), secondsLater);
  }

  typename Clock::time_point getTimeZero() const {
      return timeZero;
  }

  /**
   * Realtime of the internal @code Clock
   */
  typename Clock::time_point realtime() const { 
      return Clock::now();
  }

  /**
  * Realtime of the internal @code Clock truncated to milliseconds.
  */
  long long realtimeInNanoseconds() const {
      return static_cast<long long>(
          duration_cast<nanoseconds>(Clock::now().time_since_epoch()).count());
  }

  /**
  * Time passed since @code timeZero in units of @code Clock
  */
  typename Clock::time_point now() const {
      return Clock::time_point(Clock::now() - timeZero);
  }

  /**
   * Get the unit of the configured clocks tick.
   * @return A unit between seconds and picoseconds, depending on the clock.
   */
  const std::string unitName() const {
    return find_if(
               begin(knownUnits), end(knownUnits),
               [&](UnitMap::value_type const &it) { return it.second == den; })
        ->first;
  }

  /**
  * Compute the @code Clock::duration passed since @code timeZero until @code
  * now.
  * @return The @code Duration passed from @code timeZero until @code timestamp.
  * @sa @code ms and @code s
  */
  const Dur timeSinceInit() const { return Clock::now() - timeZero; }

  /**
  * Compute the @code Clock::duration passed since @code timeZero until @code
  * timestamp.
  * @param[in] timestamp The reference timepoint
  * @return The @code Duration passed from @code timeZero until @code timestamp.
  * @sa @code ms and @code s
  */
  const Dur
  timeSinceInitUntil(typename Clock::time_point const &timestamp) const {
    return timestamp - timeZero;
  }

  /**
   * Give the value of a timestamp in milliseconds.
   * @param[in] timestamp The timepoint
   * @return The timestamp represented in ms.
   */
  const double ms(typename Clock::time_point const &timestamp) const {
      return static_cast<double>(duration_cast<milliseconds>(timestamp.time_since_epoch()).count());
  }

  const double ns(typename Clock::time_point const &timestamp) const {
      return static_cast<double>(duration_cast<nanoseconds>(timestamp.time_since_epoch()).count());
  }
  /**
  * Compute the time passed since @code timeZero until @code Clock::now, in
  * milliseconds.
  * @param[in] timestamp The reference timepoint
  * @return The milliseconds passed from @code timeZero until @code timestamp.
  */
  const double ms() const { return ms(now()); }

  /**
   * Access the singleton instance in constant context.
   */
  static timeInfo_t<Clock> const &ctimeInfo() {
    return timeInfo_t<Clock>::instance;
  }

  /**
  * Access the singleton instance.
  */
  static timeInfo_t<Clock> &timeInfo() { return timeInfo_t<Clock>::instance; }

  private:
      intmax_t num = Dur::period::num;
      intmax_t den = Dur::period::den;

      static timeInfo_t<Clock> instance;
      typename Clock::time_point timeZero{};

      timeInfo_t() {
          timeZero = Clock::now();
      }

};

///< Instantiation of @code timeInfo_t with the Clock as defined in types.h.
///< All time operations must refer to the same clock, so it's crucial that
///< only the Clock, Timestamp and Duration as defined in types.h are used
///< explicitly.
using TimeInfo = timeInfo_t<Clock>;

// just the instantiation of the static member
template <typename Clock> timeInfo_t<Clock> timeInfo_t<Clock>::instance;



#ifdef RUN_TIMER

/**
* Utility class to measure code runtime.
*/
class Timer {
  using Clock = std::chrono::steady_clock;

public:
  Timer(Timer const &) = delete;
  ~Timer();

  /**
  * Auxiliary class providing a block-scoped Timer run.
  */
  class ScopedTimerRunner {
    char const *id;

  public:
    ScopedTimerRunner(char const *id) : id(id) { Timer::start(id); }
    ~ScopedTimerRunner() { Timer::stop(id); }
  };

  /**
   * Auxiliary class which instance automatically finishes a Timer when
   * destroyed.
   */
  class TimerWrapper {
  public:
    TimerWrapper(char const *filename) { Timer::setFilename(filename); }
    ~TimerWrapper() { Timer::done(); }
  };

private:
  using Map = std::unordered_map<char const *, Timer *>;
  static Map timer;
  static std::string filename;
  static std::ofstream outputStream;

  bool running = false;
  std::string id;
  Clock::time_point lastStartPoint;
  std::vector<Clock::duration> runtimes;

  Timer(char const *id) : id(id) {}

  static void start(char const *id);
  static void stop(char const *id);
  static void setFilename(char const *name);
  static void done();
};

#define TIMED_BLOCK(id) Timer::ScopedTimerRunner __timer(id);
#define TIMER_REGISTER(name) Timer::TimerWrapper __timerWrapper{name};
#else
#define TIMED_BLOCK(id)
#define TIMER_REGISTER(name)
#endif
}
#endif