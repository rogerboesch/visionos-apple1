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

#include "ret_renderer.h"
#include "terminal.h"

@implementation UIImage (Buffer)

// -----------------------------------------------------------------------------
#pragma mark - Image handling

+ (UIImage *)fromBuffer:(unsigned char *)buffer width:(int)width height:(int)height {
    int length = width*height*4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, length, NULL);
    
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

// External functions
int emulator_init(void);
int emulator_frame(void);

// Helper functions to access platfiorm stuff
char* platform_file_path(char *name) {
    NSString *filename = [NSString stringWithCString:name encoding:NSASCIIStringEncoding];
    NSString *path = [[NSBundle mainBundle] pathForResource:filename ofType:@"ROM"];

    if (path == nil) {
        return nil;
    }
    
    sprintf(filepath, "%s", path.UTF8String);
    
    return filepath;
}

// Called from C to Swift
void ret_render_frame(unsigned char *data, int index) {
    UIImage* image = [UIImage fromBuffer:data width:RET_PIXEL_WIDTH height:RET_PIXEL_HEIGHT];
    [Renderer render:image];
}

// Called from Swift to C

NSMutableArray *queue = nil;

void EmulatorInit(void) {
    emulator_init();

    queue = [[NSMutableArray alloc] init];
}

void EmulatorFrame(void) {
    if (terminal_testch() == TERMINAL_ERR) {
        NSNumber* ascii = [queue dequeue];

        if (ascii != nil) {
            terminal_setch(ascii.intValue);
        }
    }

    emulator_frame();
}

void EmulatorKeyPress(int ch) {
    [queue enqueue:[NSNumber numberWithInt:ch]];
}
