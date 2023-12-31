//
// This file is part of the course materials for AMATH483/583 at the University of Washington,
// Spring 2020
//
// Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#ifndef AMATH_583_TIMER_HPP
#define AMATH_583_TIMER_HPP

#include <iostream>
#include <chrono>
#include <string>

class Timer {
private:
  typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_t;

public:
  Timer() = default;

  time_t start()   { return (startTime = std::chrono::high_resolution_clock::now()); }
  time_t stop()    { return (stopTime  = std::chrono::high_resolution_clock::now()); }
  double elapsed() { return std::chrono::duration_cast<std::chrono::microseconds>(stopTime-startTime).count(); }

private:
  time_t startTime, stopTime;
};


class life_timer : public Timer {

public:
  life_timer(const std::string& msg = "") : msg_(msg) { Timer::start(); }
  ~life_timer() {
    Timer::stop();
    if (msg_ != "") {
      std::cout << "# [ " + msg_ + " ]: ";
    }
    std::cout << Timer::elapsed() / 1.e3 << " mu s" << std::endl;
  }
private:
  std::string msg_;
};


#endif //  AMATH_583_TIMER_HPP