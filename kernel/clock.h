#ifndef __CLOCK_H__
#define __CLOCK_H__

#define CLOCK_MAX_WATCHERS 10

void clock_init(u32_t frequency);
void clock_add_watcher(void (*func)(void));

#endif
