/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 OpenGL view subclass.
 */


#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import "GlRender.h"

@interface GLEssentialsGLView : NSOpenGLView {
	CVDisplayLinkRef displayLink;
}

-(instancetype)initWithFrame:(NSRect)frameRect render:(GlRender*)render;

@end
