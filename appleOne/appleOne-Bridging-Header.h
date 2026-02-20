//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

void EmulatorInit(void);
void EmulatorFrame(void);
void EmulatorKeyPress(int ch);

void EmulatorHardReset(void);
void EmulatorLoadBasic(void);
void EmulatorLoadCore(void);

void EmulatorShowJobs(void);
void EmulatorShowWozniak(void);
void EmulatorShowBothSteves(void);

void EmulatorSkipSplash(void);
void EmulatorRefreshDisplay(void);

void GameSetModeEmulator(void);
void GameSetModeBreakout(void);
void GameBreakoutInput(int action);
void GameBreakoutInputRelease(void);
void GameBreakoutReset(void);
