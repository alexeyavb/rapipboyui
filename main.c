#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <raylib.h>

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include "text_utils.h"

#include "global_variables.h"
#include "loader_thread.h"
#include "simple_thread.h"
#include "linux_evthread.h"
#include "linux_i2c_thread.h"
#include "local_timers.h"
#include "main.h"


int main(void){
	extern int worked_pip_main (void);
	worked_pip_main();
	return 0;
}

int worked_pip_main (void)
{	
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCRREN_WIDTH, SCREEN_HEIGHT, "Pipboy OS");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("images/Pip-Boy-StartScreen.png");
	
	// Simple thread step 1
	simple_thread_init();
	
	// Run Event thread immediatelly
	// EventThread_Init();
	
	I2C_Thread_Init();



	// Font ru_font = LoadFont("fonts/dejavu.fnt");
	Font ru_font = LoadCyrillicTTF("fonts/Microsoft Sans Serif.ttf", 12); 	
	const char* text = "ЯЮЭЖЗЦЙФЫЛДЯБ";
	// const char *text = "\xD0\xAF\x20\xD0\xBB\xD1\x8E\xD0\xB1\xD0\xBB\xD1\x8E\x20\x72\x61\x79\x6C\x69\x62\x21";
	float fontSize = 16.0f;
	float spacing = 1.34f;
	bool wordWrap = false; 
	Color tint = {0x0f, 0xee, 0x03, 0xaf} ;	// #0fee03af
	Rectangle rec = {4, 4, 740, 22};
	
	// extern void initCamera(void); initCamera();
	// extern void renderCameraProc(void);
	// extern void RotateModelProc(void);
	// game loop
    // int framesCount = {0};

	// Simple thread step 2 run
	simple_thread_run();

	int staytime = 6000ul;
	int startTick = GetTickCountMs() + staytime + 500;
	bool lShowLogo = true;

	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();
		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);		
		if(lShowLogo){
			DrawTexture(wabbit, 0, 0, GREEN);			
		}

		DrawTextBoxed(ru_font, text, rec, fontSize, spacing, wordWrap, tint);        	

		// progress bar
		checkThreadState();
		drawRuller();
		// end pb

		// bender model rotation
		// RotateModelProc();
		// renderCameraProc();
		// end bmr

		DrawFPS(FPS_X_POS, FPS_Y_POS);
		
		if(GetTickCountMs()  >= startTick )
			lShowLogo = false;
		
		EndDrawing();
		
	}

	// cleanup
	// stop poll devices event thread
	i2c_thread_running = false;
	event_thread_running = false;	
	simple_thread_running = false;

	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);
	// destroy the window and cleanup the OpenGL context
	sleep(2);
	CloseWindow();
	return 0;
}
