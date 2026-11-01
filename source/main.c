#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <citro2d.h>
#include <math.h>

#include "audioPlay.h"

#define CATS 3
#define INTROCATS 20
C2D_TextBuf g_staticBuf, g_dynamicBuf;
C2D_Text g_staticText[CATS+4];
struct introCat {
	int cat;
	float x;
	float y;
	float rot;
	float rotS;
	float xS;
	float scale;
};
struct introCat introCats[INTROCATS];

int main(int argc, char* argv[])
{
	int mode = 0;
	double x = 240;double y = 0;
	int refPos[2] = {0,0};
	double velocity[2] = {0,0};
	float touchVel[4] = {0,0,0,0};
	int cat = 0;
	bool isPicked = 0;
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);
	C2D_SpriteSheet backgroundSheet = C2D_SpriteSheetLoad("romfs:/gfx/backgrounds.t3x");
	if (!backgroundSheet) svcBreak(USERBREAK_PANIC);
	if (C2D_SpriteSheetCount(spriteSheet) < CATS) svcBreak(USERBREAK_PANIC);
	if (C2D_SpriteSheetCount(backgroundSheet) < 1) svcBreak(USERBREAK_PANIC);
    C2D_Image uni = C2D_SpriteSheetGetImage(spriteSheet, 1);
    C2D_Image hiCat = C2D_SpriteSheetGetImage(spriteSheet, 0);
    C2D_Image marker = C2D_SpriteSheetGetImage(spriteSheet, 2);
    C2D_Image images[CATS] = {hiCat, uni, marker};
    C2D_Image backgrounds[1] = {C2D_SpriteSheetGetImage(backgroundSheet, 0)};
	u32 color1  = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
	u32 color2  = C2D_Color32(0xFF, 0x0, 0x0, 0xFF);
	u32 color3  = C2D_Color32(0x0, 0x0, 0xFF, 0xFF);
	g_staticBuf  = C2D_TextBufNew(4096); // support up to 4096 glyphs in the buffer
	g_dynamicBuf = C2D_TextBufNew(4096);
	C2D_TextParse(&g_staticText[0], g_staticBuf, "he said hi :skull:");
	C2D_TextOptimize(&g_staticText[0]);
	C2D_TextParse(&g_staticText[1], g_staticBuf, "HI UNI!!!!!");
	C2D_TextOptimize(&g_staticText[1]);
	C2D_TextParse(&g_staticText[2], g_staticBuf, "hello everybody I'm a fish");
	C2D_TextOptimize(&g_staticText[2]);
	C2D_TextParse(&g_staticText[CATS], g_staticBuf, "Press A to switch cats.");
	C2D_TextOptimize(&g_staticText[CATS]);
	C2D_TextParse(&g_staticText[1+CATS], g_staticBuf, "Press START to exit.");
	C2D_TextOptimize(&g_staticText[1+CATS]);
	C2D_TextParse(&g_staticText[2+CATS], g_staticBuf, "Press A to continue.");
	C2D_TextOptimize(&g_staticText[2+CATS]);
	C2D_TextParse(&g_staticText[3+CATS], g_staticBuf, "idk if I'll use ts");
	C2D_TextOptimize(&g_staticText[3+CATS]);
	audio_init();
	audio_play("romfs:/sfx/bgm.pcm");
	for (int i=0;i<INTROCATS;i++){introCats[i].x = 800.0;}
	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();
		touchPosition touch;
		hidTouchRead(&touch);
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		if (mode == 67) {break;}
		switch (mode) {
			case 0:
				C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
				C2D_TargetClear(bottom, color1);
				C2D_SceneBegin(bottom);
				for (int i=0;i<INTROCATS;i++){
					if (introCats[i].x > 599) {
						introCats[i].cat = rand()%CATS;
						introCats[i].x = -200;
						introCats[i].y = rand()%280+20;
						introCats[i].rot = 0;
						introCats[i].rotS = (float)(rand()%10)*0.01745329;
						introCats[i].xS = (float)(rand() % (75 - 20 + 10) + 20)/10.0;
						introCats[i].scale = (float)(rand() % (50 - 25 + 50) + 25)/100.0;
					}
					introCats[i].x += introCats[i].xS;
					introCats[i].rot += introCats[i].rotS;
					C2D_DrawImageAtRotated(images[introCats[i].cat], introCats[i].x, introCats[i].y, 0, introCats[i].rot, NULL, introCats[i].scale, introCats[i].scale);
				}
				C2D_DrawRectSolid(85, 190, 0, 150, 50, color1);
				C2D_DrawText(&g_staticText[CATS+2], C2D_AlignCenter, 160, 200, 0, 0.5, 0.5);
				C2D_DrawText(&g_staticText[CATS+1], C2D_AlignCenter, 160, 220, 0, 0.5, 0.5);
				C2D_TargetClear(top, color1);
				C2D_SceneBegin(top);
				C2D_DrawImageAt(backgrounds[0], 0, 0, 0, NULL, 1, 1);
				C3D_FrameEnd(0);
				if (kDown & KEY_A) {mode = 1;audio_stop();}
				if (kDown & KEY_START){mode = 67;audio_stop();}
				break;
			case 1:
				if (isPicked) {
					x = touch.px-refPos[0]+40;
					y = touch.py-refPos[1]+240;
					velocity[1] = 0;
				} else {
					if (y+160>479) {
						if (velocity[1]<3) velocity[1] = 0;
						velocity[1] = velocity[1]*-0.4;
						y=320;
						velocity[0]*=0.9;
					} else {
						velocity[1] += 0.1;
					}
				}
				if (x<0) {
					velocity[0] = abs(velocity[0])*0.75;
					x = 0;
				}
				if (x>240) {
					velocity[0] = abs(velocity[0])*-0.75;
					x = 240;
				}
				y += velocity[1];
				x += velocity[0];
				if (y<-160) velocity[1]+=1;
				if (kDown & KEY_START){
					mode = 67; // break in order to return to hbmenu
				}
				if (kDown & KEY_A) {
					cat++;
					if (cat >= CATS) {cat = 0;} 
				}				
				C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
				
				C2D_TargetClear(bottom, color1);
				C2D_SceneBegin(bottom);
				C2D_DrawText(&g_staticText[cat], C2D_AlignCenter, 160, 10, 0, 1, 1);
				C2D_DrawText(&g_staticText[CATS], C2D_AlignCenter, 160, 200, 0, 0.5, 0.5);
				C2D_DrawText(&g_staticText[CATS+1], C2D_AlignCenter, 160, 220, 0, 0.5, 0.5);
				C2D_DrawImageAt(images[cat], (int)x-40, (int)y-240, 0, NULL, 1, 1);
				
				if (kDown & KEY_TOUCH) {
					if (touch.px > x-40 && touch.px < x+120 && touch.py > y-240 && touch.py < y-80) {
						isPicked = 1;
						refPos[0] = touch.px-x+40;
						refPos[1] = touch.py-(y-240);
					}
				}
				if (kHeld & KEY_TOUCH) {
					C2D_DrawCircleSolid(touch.px, touch.py, 0, 5, color2);
				} else {
					if (touchVel[3] != 0 && touchVel[2] != 0) {
						if (isPicked) {
							velocity[0] = touchVel[0];
							velocity[1] = touchVel[1];
						}
						isPicked = 0;
					}
				}
				if (isPicked) {
					C2D_DrawCircleSolid(touch.px, touch.py, 0, 10, color3);
				}

				touchVel[0] = touch.px - touchVel[2];
				touchVel[1] = touch.py - touchVel[3];
				touchVel[2] = touch.px;
				touchVel[3] = touch.py;

				C2D_TargetClear(top, color1);
				C2D_SceneBegin(top);
				C2D_TextBufClear(g_dynamicBuf);
				char buf[160];
				C2D_Text dynText;
				
				snprintf(buf, sizeof(buf), "velX: %f velY: %f", velocity[0], velocity[1]);
				C2D_TextParse(&dynText, g_dynamicBuf, buf);
				
				C2D_TextOptimize(&dynText);
				C2D_DrawText(&dynText, C2D_AlignCenter, 200, 0, 0, 0.5, 0.5);
				
				C2D_DrawImageAt(images[cat], (int)x, (int)y, 0, NULL, 1, 1);
				C3D_FrameEnd(0);
				break;
			default:
				mode = 0;
				break;
		}
	}
	audio_stop();
    ndspExit();
	C2D_TextBufDelete(g_staticBuf);
	C2D_TextBufDelete(g_dynamicBuf);
	C2D_SpriteSheetFree(spriteSheet);
	C2D_SpriteSheetFree(bakgroundSheet);
	C2D_Fini();
	C3D_Fini();
	romfsExit();
	gfxExit();
	return 0;
}
