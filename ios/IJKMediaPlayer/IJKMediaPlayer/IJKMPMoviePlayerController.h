/*
 * IJKMPMoviePlayerController.h
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import "IJKMediaPlayback.h"

#import <MediaPlayer/MediaPlayerDefines.h>
// -----------------------------------------------------------------------------
// Types

typedef NS_ENUM(NSInteger, MPMovieScalingMode) {
    MPMovieScalingModeNone,       // No scaling
    MPMovieScalingModeAspectFit,  // Uniform scale until one dimension fits
    MPMovieScalingModeAspectFill, // Uniform scale until the movie fills the visible bounds. One dimension may have clipped contents
    MPMovieScalingModeFill        // Non-uniform scale. Both render dimensions will exactly match the visible bounds
} ;

typedef NS_ENUM(NSInteger, MPMoviePlaybackState) {
    MPMoviePlaybackStateStopped,
    MPMoviePlaybackStatePlaying,
    MPMoviePlaybackStatePaused,
    MPMoviePlaybackStateInterrupted,
    MPMoviePlaybackStateSeekingForward,
    MPMoviePlaybackStateSeekingBackward
};

typedef NS_OPTIONS(NSUInteger, MPMovieLoadState) {
    MPMovieLoadStateUnknown        = 0,
    MPMovieLoadStatePlayable       = 1 << 0,
    MPMovieLoadStatePlaythroughOK  = 1 << 1, // Playback will be automatically started in this state when shouldAutoplay is YES
    MPMovieLoadStateStalled        = 1 << 2, // Playback will be automatically paused in this state, if started
} ;

typedef NS_ENUM(NSInteger, MPMovieRepeatMode) {
    MPMovieRepeatModeNone,
    MPMovieRepeatModeOne
} ;

typedef NS_ENUM(NSInteger, MPMovieControlStyle) {
    MPMovieControlStyleNone,       // No controls
    MPMovieControlStyleEmbedded,   // Controls for an embedded view
    MPMovieControlStyleFullscreen, // Controls for fullscreen playback
    
    MPMovieControlStyleDefault = MPMovieControlStyleEmbedded
};

typedef NS_ENUM(NSInteger, MPMovieFinishReason) {
    MPMovieFinishReasonPlaybackEnded,
    MPMovieFinishReasonPlaybackError,
    MPMovieFinishReasonUserExited
};

// -----------------------------------------------------------------------------
// Movie Property Types

typedef NS_OPTIONS(NSUInteger, MPMovieMediaTypeMask) {
    MPMovieMediaTypeMaskNone  = 0,
    MPMovieMediaTypeMaskVideo = 1 << 0,
    MPMovieMediaTypeMaskAudio = 1 << 1
} ;

typedef NS_ENUM(NSInteger, MPMovieSourceType) {
    MPMovieSourceTypeUnknown,
    MPMovieSourceTypeFile,     // Local or progressively downloaded network content
    MPMovieSourceTypeStreaming // Live or on-demand streaming content
} ;


// -----------------------------------------------------------------------------
// Movie Player Notifications

// Posted when the scaling mode changes.
MP_EXTERN NSString * const MPMoviePlayerScalingModeDidChangeNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(2.0, 9.0)) MP_PROHIBITED(tvos, watchos);

// Posted when movie playback ends or a user exits playback.
MP_EXTERN NSString * const MPMoviePlayerPlaybackDidFinishNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(2.0, 9.0)) MP_PROHIBITED(tvos, watchos);

MP_EXTERN NSString * const MPMoviePlayerPlaybackDidFinishReasonUserInfoKey MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos); // NSNumber (MPMovieFinishReason)

// Posted when the playback state changes, either programatically or by the user.
MP_EXTERN NSString * const MPMoviePlayerPlaybackStateDidChangeNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);

// Posted when the network load state changes.
MP_EXTERN NSString * const MPMoviePlayerLoadStateDidChangeNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);

// Posted when the currently playing movie changes.
MP_EXTERN NSString * const MPMoviePlayerNowPlayingMovieDidChangeNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);

// Posted when the movie player enters or exits fullscreen mode.
MP_EXTERN NSString * const MPMoviePlayerWillEnterFullscreenNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);
MP_EXTERN NSString * const MPMoviePlayerDidEnterFullscreenNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);
MP_EXTERN NSString * const MPMoviePlayerWillExitFullscreenNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);
MP_EXTERN NSString * const MPMoviePlayerDidExitFullscreenNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);
MP_EXTERN NSString * const MPMoviePlayerFullscreenAnimationDurationUserInfoKey MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos); // NSNumber of double (NSTimeInterval)
MP_EXTERN NSString * const MPMoviePlayerFullscreenAnimationCurveUserInfoKey MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);     // NSNumber of NSUInteger (UIViewAnimationCurve)

// Posted when the movie player begins or ends playing video via AirPlay.
MP_EXTERN NSString * const MPMoviePlayerIsAirPlayVideoActiveDidChangeNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(5.0, 9.0)) MP_PROHIBITED(tvos, watchos);

// Posted when the ready for display state changes.
MP_EXTERN NSString * const MPMoviePlayerReadyForDisplayDidChangeNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(6.0, 9.0)) MP_PROHIBITED(tvos, watchos);

// -----------------------------------------------------------------------------
// Movie Property Notifications

// Calling -prepareToPlay on the movie player will begin determining movie properties asynchronously.
// These notifications are posted when the associated movie property becomes available.
MP_EXTERN NSString * const MPMovieMediaTypesAvailableNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);
MP_EXTERN NSString * const MPMovieSourceTypeAvailableNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos); // Posted if the movieSourceType is MPMovieSourceTypeUnknown when preparing for playback.
MP_EXTERN NSString * const MPMovieDurationAvailableNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);
MP_EXTERN NSString * const MPMovieNaturalSizeAvailableNotification MP_DEPRECATED("Use AVPlayerViewController in AVKit.", ios(3.2, 9.0)) MP_PROHIBITED(tvos, watchos);



//
//#import <MediaPlayer/MediaPlayer.h>
//
//@interface IJKMPMoviePlayerController : MPMoviePlayerController <IJKMediaPlayback>
//
//- (id)initWithContentURL:(NSURL *)aUrl;
//- (id)initWithContentURLString:(NSString *)aUrl;
//
//@end

