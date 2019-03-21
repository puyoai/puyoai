#ifndef CAPTURE_MONITOR_H_
#define CAPTURE_MONITOR_H_

// Size of captured AND normalized monitor.
#ifdef WIDE_MONITOR_GAME
static const int kMonitorWidth = 640;
static const int kMonitorHeight = 360;
#else
static const int kMonitorWidth = 640;
static const int kMonitorHeight = 448;
#endif

#endif // CAPTURE_MONITOR_H_
