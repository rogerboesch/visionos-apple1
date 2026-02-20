//
//  ObjCBridge.swift
//  Bridging class to use teh C code
//
//  Created by Roger Boesch on 30/09/15.
//  Copyright © 2016 Roger Boesch. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "appleOne-Swift.h"

#include "rb_display.h"
#include "terminal.h"
#include "Emulator.h"
#include "Effects/effect_ascii_art.h"
#include "Games/game_breakout.h"

/* 0 = emulator, 1 = breakout */
static int app_mode = 0;

@implementation UIImage (Buffer)

// -----------------------------------------------------------------------------
#pragma mark - Image handling

+ (UIImage *)fromBuffer:(unsigned char *)buffer width:(int)width height:(int)height {
    int length = width*height*4;
    CFDataRef dataCopy = CFDataCreate(NULL, buffer, length);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(dataCopy);
    CFRelease(dataCopy);
    
    int bitsPerComponent = 8;
    int bitsPerPixel = 4 * bitsPerComponent;
    int bytesPerRow = 4*width;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    CGDataProviderRelease(provider);
    
    UIImage* myImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    
    return myImage;
}

@end

@implementation NSMutableArray (QueueAdditions)

- (id)dequeue {
    if ([self count] == 0) return nil;
    id headObject = [self objectAtIndex:0];
    
    if (headObject != nil) {
        [self removeObjectAtIndex:0];
    }
    
    return headObject;
}

- (void)enqueue:(id)anObject {
    [self addObject:anObject];
}

@end

char filepath[1024];

// Helper functions to access platfiorm stuff
char* platform_file_path(char *name, char *extension) {
    NSString *filename = [NSString stringWithCString:name encoding:NSASCIIStringEncoding];
    NSString *ext = [NSString stringWithCString:extension encoding:NSASCIIStringEncoding];
    NSString *path = [[NSBundle mainBundle] pathForResource:filename ofType:ext];

    if (path == nil) {
        return nil;
    }
    
    sprintf(filepath, "%s", path.UTF8String);

    return filepath;
}

// Called from C to Swift
void rb_render_frame(unsigned char *data, int width, int height) {
    UIImage* image = [UIImage fromBuffer:data width:width height:height];
    [Renderer render:image];
}

// Called from C to Swift — single portrait for circle displays
void rb_render_portrait(unsigned char *data, int width, int height) {
    UIImage* image = [UIImage fromBuffer:data width:width height:height];
    [Renderer renderPortrait:image];
}

// Called from C to Swift — portrait pair for alternating displays
void rb_render_portrait_pair(unsigned char *dataA, int widthA, int heightA,
                              unsigned char *dataB, int widthB, int heightB) {
    UIImage* imageA = [UIImage fromBuffer:dataA width:widthA height:heightA];
    UIImage* imageB = [UIImage fromBuffer:dataB width:widthB height:heightB];
    [Renderer renderPortraitPair:imageA imageB:imageB];
}

// Called from Swift to C

NSMutableArray *queue = nil;

void EmulatorInit(void) {
    emulator_init();

    queue = [[NSMutableArray alloc] init];
}

void EmulatorFrame(void) {
    if (app_mode == 0) {
        if (terminal_testch() == TERMINAL_ERR) {
            NSNumber* ascii = [queue dequeue];

            if (ascii != nil) {
                terminal_setch(ascii.intValue);
            }
        }

        emulator_frame();
    }
    else {
        game_breakout_frame();
    }

    effect_ascii_art_frame();
}

void EmulatorKeyPress(int ch) {
    [queue enqueue:[NSNumber numberWithInt:ch]];
}

void EmulatorLoadBasic(void) {
    emulator_task(1);
}

void EmulatorHardReset(void) {
    effect_ascii_art_stop();
    emulator_task(5);
}

void EmulatorLoadCore(void) {
    emulator_task(2);
}

void EmulatorShowJobs(void) {
    effect_ascii_art_show_portrait(0, "steve-jobs");
}

void EmulatorShowWozniak(void) {
    effect_ascii_art_show_portrait(0, "steve-wozniak");
}

void EmulatorSkipSplash(void) {
    emulator_task(9);
}

void EmulatorShowBothSteves(void) {
    effect_ascii_art_show_portrait_pair("steve-jobs", "steve-wozniak");
}

void EmulatorRefreshDisplay(void) {
    effect_ascii_art_show_portrait(0, "steve-wozniak");
    terminal_refresh();
}

/* -------------------------------------------------------------------------- */
/*  Game mode switching                                                       */
/* -------------------------------------------------------------------------- */

void GameSetModeEmulator(void) {
    app_mode = 0;
    terminal_refresh();
}

void GameSetModeBreakout(void) {
    app_mode = 1;
    game_breakout_init();
}

void GameBreakoutInput(int action) {
    game_breakout_input(action);
}

void GameBreakoutInputRelease(void) {
    game_breakout_input_release();
}

void GameBreakoutReset(void) {
    game_breakout_reset();
}

