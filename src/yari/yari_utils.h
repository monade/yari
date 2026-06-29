#ifndef YR_UTILS_H
#define YR_UTILS_H

#include <stdbool.h>
#include "yari.h"

typedef struct {
    float ends_at;
} YrTimer;

static inline YrTimer yr_timer_start(float duration) {
    YrTimer timer;
    timer.ends_at = yr_get_time() + duration;
    return timer;
}

static inline bool yr_timer_is_done(YrTimer *timer) {
    return yr_get_time() >= timer->ends_at;
}

static inline bool yr_timer_loop(YrTimer *loop, float duration) {
    if (yr_get_time() >= loop->ends_at) {
        loop->ends_at = yr_get_time() + duration;
        return true;
    }
    return false;
}

static inline bool yr_timer_is_started(YrTimer *timer) {
    return timer->ends_at != 0.0f;
}

#ifdef YARI_NO_PREFIX
#define Timer YrTimer
#define timer_start yr_timer_start
#define timer_is_done yr_timer_is_done
#define timer_loop yr_timer_loop
#define timer_is_started yr_timer_is_started
#endif

#endif // YR_UTILS_H
