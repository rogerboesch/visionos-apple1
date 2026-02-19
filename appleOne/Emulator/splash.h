#ifndef SPLASH_H
#define SPLASH_H

// Initialize the splash screen state
void splash_init(void);

// Update one frame of the splash animation.
// Returns 1 while splash is active, 0 when done.
int splash_frame(void);

#endif
