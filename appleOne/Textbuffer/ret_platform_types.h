
#ifndef ret_platform_types_h
#define ret_platform_types_h

// Datatypes
#ifndef byte
	typedef unsigned char byte;
#endif

#ifndef boolean
	#define boolean int
	#define true 1
	#define false 0
#endif

enum KEY_ACTION{
	RET_KEY_NULL = 0,
	RET_KEY_CTRL_C = 3,
	RET_KEY_CTRL_D = 4,
	RET_KEY_CTRL_E = 5,
	RET_KEY_CTRL_F = 6,
	RET_KEY_CTRL_H = 8,
	RET_KEY_TAB = 9,
	RET_KEY_CTRL_L = 12,
	RET_KEY_ENTER = 13,
	RET_KEY_CTRL_R = 16,
	RET_KEY_CTRL_Q = 17,
	RET_KEY_CTRL_S = 19,
	RET_KEY_CTRL_U = 21,
	RET_KEY_ESC = 27,
	RET_KEY_BACKSPACE = 127, 
	
	// The following are just soft codes, not really assigned to ascii codes
	RET_KEY_ARROW_LEFT = 200,
	RET_KEY_ARROW_RIGHT,
	RET_KEY_ARROW_UP,
	RET_KEY_ARROW_DOWN,
    
    // Not implemented assigned yet
	RET_KEY_DEL_KEY,
	RET_KEY_HOME_KEY,
	RET_KEY_END_KEY,
	RET_KEY_PAGE_UP,
	RET_KEY_PAGE_DOWN
};

unsigned long ret_text_create_rgb(int r, int g, int b);

#endif
