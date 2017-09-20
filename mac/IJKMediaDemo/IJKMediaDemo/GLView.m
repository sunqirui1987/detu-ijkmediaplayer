/*
     File: GLView.mm
 Abstract: The OpenGL view. It delegates to the renderer class for drawing.
  Version: 1.3
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
 */

#import "GLView.h"
#import "GlRender.h"
#import "GlFlatRender.h"

#define kSpacebarKeyCode 49

static CVReturn Heartbeat (		CVDisplayLinkRef displayLink,
                           const CVTimeStamp *inNow,
                           const CVTimeStamp *inOutputTime,
                           CVOptionFlags flagsIn,
                           CVOptionFlags *flagsOut,
                           void *displayLinkContext)
{
	GLView* view = (__bridge GLView*) displayLinkContext;
    // this method is called on a background thread via the display link
    // make sure we have the right context
	CGLSetCurrentContext((CGLContextObj) [view.ctx CGLContextObj]);
    // and lock it to avoid thread conflicts
    CGLLockContext((CGLContextObj) [view.ctx CGLContextObj]);
	[view.renderer render];
	CGLFlushDrawable((CGLContextObj) [view.ctx CGLContextObj]);
    CGLUnlockContext((CGLContextObj) [view.ctx CGLContextObj]);
    
	return kCVReturnSuccess;
}

@implementation GLView

- (id)initWithFrame:(NSRect)frame{
	NSOpenGLPixelFormatAttribute attrs[] = {
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core, //Core Profile
        0
	};
		
    NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    self = [super initWithFrame:frame pixelFormat: format];
    if (self)
	{
        self.pf = format;
		self.ctx = [self openGLContext];
		[self.ctx makeCurrentContext];
        
        // create the renderer object
		self.renderer = [[GlFlatRender alloc]init];;
		NSRect bounds = [self bounds];
		NSRect pixels = [self convertRectToBacking:bounds];
		[self.renderer resizeWithWidth:pixels.size.width AndHeight:pixels.size.height];
    }
    return self;
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    [self.renderer initWithDefaultFBO:0];
	
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, (CGLContextObj) [self.ctx CGLContextObj], (CGLPixelFormatObj) [self.pf CGLPixelFormatObj]);
	CVDisplayLinkSetOutputCallback(displayLink, &Heartbeat, (__bridge void*)self);
	CVDisplayLinkStart(displayLink);
    
    [[self window] setAcceptsMouseMovedEvents:YES];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	return YES;
}

- (void)dealloc
{
	CVDisplayLinkStop(displayLink);
	CVDisplayLinkRelease(displayLink);
	//[self.pf release];
	//[self.ctx release];
	//[self.renderer release];
	//free(cbCtx);
	//[super dealloc];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint p = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	// eat the first event so the mouse doesn't go crazy
	if(lastPoint.x + lastPoint.y <= 0.01)
	{
		lastPoint = p;
	}
    // if lastPoint is inside the view
    if ((lastPoint.x >= 0 && lastPoint.x < self.bounds.size.width) && (lastPoint.y >= 0 & lastPoint.y < self.bounds.size.height)) {
        float dx = lastPoint.x - p.x;
        float dy = lastPoint.y - p.y;
        //[cbCtx->renderer applyCameraMovementWdx:dx dy:dy];
    }
    lastPoint = p;
}

@end
