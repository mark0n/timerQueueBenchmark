#include <benchmark/benchmark.h>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>
#include <condition_variable>

#include <epicsTimer.h>

using namespace std;

class handler : public epicsTimerNotify
{
public:
  handler(const char* nameIn, epicsTimerQueueActive& queue) : name(nameIn), timer(queue.createTimer()) {}
  ~handler() { timer.destroy(); }
  void start(double delay) { timer.start(*this, delay); }
  void cancel() { timer.cancel(); }
  expireStatus expire(const epicsTime& currentTime) {
    cout << name << "\n";
    return expireStatus(restart, 0.1);
//     return expireStatus(noRestart);
  }
private:
  const string name;
  epicsTimer &timer;
};

// class timerQueueFixture : public benchmark::Fixture {
// public:
//   epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
// };

static epicsTimerQueueActive& createEpicsTimerQueueActive() {
  static epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
  return tq;
}

// class myEpicsTimerQueueActive {
//   epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
// public:
//   myEpicsTimerQueueActive() {}
//   ~myEpicsTimerQueueActive() { tq.release(); }
//   epicsTimerQueueActive& get() { return tq; }
// };
// 
class epicsTimerQueueActiveCntr {
public:
  static size_t refCnt;
  static epicsTimerQueueActive& tq;
  static bool valid;
  static epicsTimerQueueActive& get() {
    cerr << "get()";
    if (!valid) {
      cerr << " - invalid, allocating";
      tq = epicsTimerQueueActive::allocate(false);
      valid = true;
    }
    refCnt++;
    cerr << " - new ref cnt is " << refCnt << "\n";
    return tq;
  }
  static void release() {
    cerr << "release()";
    refCnt--;
    cerr << " - new ref cnt is " << refCnt;
    if (refCnt == 0 && valid) {
      cerr << " - releasing";
      tq.release();
      valid = false;
    }
    cerr << "\n";
  }
};

size_t epicsTimerQueueActiveCntr::refCnt = 0;
epicsTimerQueueActive& epicsTimerQueueActiveCntr::tq = epicsTimerQueueActive::allocate(false);
bool epicsTimerQueueActiveCntr::valid = true;


// class epicsTimerQueueActiveFactory {
// private:
//   bool valid = true;
//   epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
// public:
//   static epicsTimerQueueActive& get() {
//     if (!valid) {
//       tq = epicsTimerQueueActive::allocate(false);
//     }
//     return tq;
//   }
//   static void release() {
//     tq.release();
//     valid = false;
//   }
// };

static void withPassiveTimersCreateTimer(benchmark::State& state) {
  epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
  {
    vector<handler> hv;
    hv.reserve(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
      hv.emplace_back("arbitrary name", tq);
    }

    for (auto _ : state) {
      // create and immediately destroy a timer
      handler h = handler("arbitrary name", tq);
    }
  }
  tq.release();
}
BENCHMARK(withPassiveTimersCreateTimer)->Arg(0)->Arg(30000);

static void withActiveTimersCreateTimer(benchmark::State& state) {
  epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
  {
    vector<handler> hv;
    hv.reserve(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
      hv.emplace_back("arbitrary name", tq);
    }
    for (auto& h : hv) {
      h.start(60.0f);
    }

    for (auto _ : state) {
      // create and immediately destroy a timer
      handler h = handler("arbitrary name", tq);
    }
  }

  tq.release();
}
BENCHMARK(withActiveTimersCreateTimer)->Arg(0)->Arg(30000);

static void withActiveTimersCreateAndStartTimer(benchmark::State& state) {
  epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
  {
    vector<handler> hv;
    hv.reserve(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
      hv.emplace_back("arbitrary name", tq);
    }
    for (auto& h : hv) {
      h.start(60.0f);
    }

    for (auto _ : state) {
      handler h = handler("arbitrary name", tq);
      h.start(state.range(1));
    }
  }

  tq.release();
}
BENCHMARK(withActiveTimersCreateAndStartTimer)
  ->Args({0,      30})  // start timer while timer queue is empty, new timer has smallest expiration time
  ->Args({30000,  30})  // start timer while timer queue has 30,000 entries, new timer has smallest expiration time
  ->Args({0,     120})  // start timer while timer queue is empty, new timer has largest expiration time
  ->Args({30000, 120}); // start timer while timer queue has 30,000 entries, new timer has largest expiration time


static void withActiveTimersCreateAndStartTimerMultiThreaded(benchmark::State& state) {
  epicsTimerQueueActive& tq = createEpicsTimerQueueActive(); // share timer queue between threads
//   epicsTimerQueueActive& tq = epicsTimerQueueActiveCntr::get();
  {
    vector<handler> hv;
    if (state.thread_index == 0) {
      hv.reserve(state.range(0));
      for (int i = 0; i < state.range(0); i++) {
        hv.emplace_back("arbitrary name", tq);
      }
      for (auto& h : hv) {
        h.start(60.0f);
      }
    }

    for (auto _ : state) {
      handler h = handler("arbitrary name", tq);
      h.start(state.range(1));
    }
  }

//   static bool released = false;
//   if (!released) {
//     cerr << "RELEASING\n";
//     tq.release();
//   }
//   if (state.thread_index == 0) {
//     cerr << "RELEASING...";
//     tq.release();
//     cerr << "DONE\n";
//   }
//   epicsTimerQueueActiveCntr::release();
}
BENCHMARK(withActiveTimersCreateAndStartTimerMultiThreaded)
  ->Args({0,      30}) // start timer while timer queue is empty, new timer has smallest expiration time
  ->Args({30000,  30}) // start timer while timer queue has 30,000 entries, new timer has smallest expiration time
  ->Args({0,     120}) // start timer while timer queue is empty, new timer has largest expiration time
  ->Args({30000, 120}) // start timer while timer queue has 30,000 entries, new timer has largest expiration time
  ->Threads(1)
  ->Threads(100);

static void withActiveTimersCancelTimer(benchmark::State& state) {
  epicsTimerQueueActive& tq = epicsTimerQueueActive::allocate(false);
  {
    vector<handler> hv;
    hv.reserve(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
      hv.emplace_back("arbitrary name", tq);
    }
    for (auto& h : hv) {
      h.start(60.0f);
    }

    for (auto _ : state) {
      state.PauseTiming();
      handler h = handler("arbitrary name", tq);
      h.start(state.range(1));
      state.ResumeTiming();
      h.cancel();
    }
  }

  tq.release();
}
BENCHMARK(withActiveTimersCancelTimer)
  ->Args({0,      30})  // start timer while timer queue is empty, new timer has smallest expiration time
  ->Args({30000,  30})  // start timer while timer queue has 30,000 entries, new timer has smallest expiration time
  ->Args({0,     120})  // start timer while timer queue is empty, new timer has largest expiration time
  ->Args({30000, 120}); // start timer while timer queue has 30,000 entries, new timer has largest expiration time

// static void epicsThreadSleep(benchmark::State& state) {
// //   cout << "epicsThreadSleepQuantum: " << epicsThreadSleepQuantum() << "\n";
//   auto t = 1.23456789 / state.range(0);
//   for (auto _ : state) {
//     epicsThreadSleep(t);
//   }
// }
// BENCHMARK(epicsThreadSleep)->RangeMultiplier(10)->Range(10, 1000000);
// 
// static void stdThisThreadSleepFor(benchmark::State& state) {
//   auto t = 1.23456789s / state.range(0);
//   for (auto _ : state) {
//     std::this_thread::sleep_for(t);
//   }
// }
// BENCHMARK(stdThisThreadSleepFor)->RangeMultiplier(10)->Range(10, 1000000);

BENCHMARK_MAIN();
