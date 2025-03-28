/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#if ENABLE(MEDIA_STREAM)
#import <WebKit/WKNavigationDelegate.h>
#import <WebKit/WKUIDelegate.h>
#import <WebKit/WKUIDelegatePrivate.h>

@interface UserMediaCaptureUIDelegate : NSObject<WKNavigationDelegate, WKUIDelegate> {
    bool _wasPrompted;
    int _numberOfPrompts;
    WKPermissionDecision _audioDecision;
    WKPermissionDecision _videoDecision;
    WKDisplayCapturePermissionDecision _getDisplayMediaDecision;
}

@property (readonly) BOOL wasPrompted;
@property int numberOfPrompts;
@property WKPermissionDecision decision;

-(void)waitUntilPrompted;
-(void)resetWasPrompted;

-(void)setAudioDecision:(WKPermissionDecision)decision;
-(void)setVideoDecision:(WKPermissionDecision)decision;
-(void)setGetDisplayMediaDecision:(WKDisplayCapturePermissionDecision)decision;

@property (nonatomic, copy) WKWebView* (^createWebViewWithConfiguration)(WKWebViewConfiguration *, WKNavigationAction *, WKWindowFeatures *);

@end

@interface UserMediaCaptureUIDelegateWithDeviceChange : NSObject<WKNavigationDelegate, WKUIDelegate> {
    bool _wasPrompted;
}
-(void)addDefaultCamera:(WKWebViewConfiguration*)configuration;
-(void)addDefaultMicrophone:(WKWebViewConfiguration*)configuration;

@end
#endif // ENABLE(MEDIA_STREAM)
