/*
 * Copyright (C) 2020 Igalia S.L.
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

#include "config.h"
#include "UserMediaCaptureManager.h"

#if USE(GLIB) && ENABLE(MEDIA_STREAM)

#include "UserMediaCaptureManagerMessages.h"
#include "WebProcess.h"
#include <WebCore/CaptureDeviceWithCapabilities.h>
#include <WebCore/MediaDeviceHashSalts.h>
#include <WebCore/MediaStreamRequest.h>
#include <wtf/TZoneMallocInlines.h>

namespace WebKit {
using namespace WebCore;

WTF_MAKE_TZONE_ALLOCATED_IMPL(UserMediaCaptureManager);

UserMediaCaptureManager::UserMediaCaptureManager(WebProcess& process)
    : m_process(process)
{
    process.addMessageReceiver(Messages::UserMediaCaptureManager::messageReceiverName(), *this);
}

UserMediaCaptureManager::~UserMediaCaptureManager()
{
    m_process->removeMessageReceiver(Messages::UserMediaCaptureManager::messageReceiverName());
}

void UserMediaCaptureManager::ref() const
{
    m_process->ref();
}

void UserMediaCaptureManager::deref() const
{
    m_process->deref();
}

void UserMediaCaptureManager::validateUserMediaRequestConstraints(const WebCore::MediaStreamRequest& request, WebCore::MediaDeviceHashSalts&& deviceIdentifierHashSalts, WebCore::RealtimeMediaSourceCenter::ValidateHandler&& validateHandler)
{
    RealtimeMediaSourceCenter::singleton().validateRequestConstraints(WTFMove(validateHandler), request, WTFMove(deviceIdentifierHashSalts));
}

void UserMediaCaptureManager::getMediaStreamDevices(bool revealIdsAndLabels, GetMediaStreamDevicesCallback&& completionHandler)
{
    RealtimeMediaSourceCenter::singleton().getMediaStreamDevices([completionHandler = WTFMove(completionHandler), revealIdsAndLabels](auto&& devices) mutable {
        auto devicesWithCapabilities = WTF::compactMap(devices, [&](auto& device) -> std::optional<CaptureDeviceWithCapabilities> {
            RealtimeMediaSourceCapabilities deviceCapabilities;

            if (device.isInputDevice()) {
                auto capabilities = RealtimeMediaSourceCenter::singleton().getCapabilities(device);
                if (!capabilities)
                    return std::nullopt;

                if (revealIdsAndLabels)
                    deviceCapabilities = *capabilities;
            }

            return CaptureDeviceWithCapabilities { WTFMove(device), WTFMove(deviceCapabilities) };
        });

        completionHandler(WTFMove(devicesWithCapabilities));
    });
}

} // namespace WebKit

#endif
