/*
 * Copyright (C) 2026 Igalia, S.L.
 * Copyright (C) 2026 Comcast
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
#include "SmoothSwitchOnGOP.h"

#include "Logging.h"

#include <cstdio>
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"

namespace WebCore {

bool SmoothSwitchOnGOP::isReadyForMoreSamples()
{
    if (std::holds_alternative<StateCollecting>(m_currentState)) {
        // When we're doing a smooth switch we want the SourceBuffer to provide samples ASAP,
        // since our decision on how to finish the smooth switch will depend on them.
        // We will still only feed the inner sink as much as it can handle.
        return true;
    }
    return m_innerSink->isReadyForMoreSamples();
}

void SmoothSwitchOnGOP::notifyWhenReadyForMoreSamples(std::function<void()>&& callback)
{
    m_innerSink->notifyWhenReadyForMoreSamples(std::forward<std::function<void()>>(callback));
}

void SmoothSwitchOnGOP::enqueueSample(Ref<MediaSample>&& sample)
{
    DEBUG_LOG(LOGIDENTIFIER, "Enqueue with DTS=", sample->decodeTime().toDouble());
    fprintf(stderr, "enqueueSample %s\n", sample->decodeTime().toString().utf8().data());

    DecodeOrderSampleMap::KeyType decodeKey { sample->decodeTime(), sample->presentationTime() };

    if (auto* current = std::get_if<StateCollecting>(&m_currentState)) {
        fprintf(stderr, "  (in collecting state, old samples: %lu, new gop: %lu)\n", current->oldSamples.size(), current->newGop.size());
        if (sample->isSync()) {
            current->newGop.clear();

            if ((m_lastDeliveredDecodeKey.first.isInvalid() || decodeKey > m_lastDeliveredDecodeKey)) {
                DEBUG_LOG(LOGIDENTIFIER, "Switching to new GOP with DTS=", sample->decodeTime().toDouble());
                fprintf(stderr, "  switching to new gop, dts: %s\n", sample->decodeTime().toString().utf8().data());
                current->oldSamples.erase(current->oldSamples.lower_bound(decodeKey), current->oldSamples.end());

                for (auto& remaining : current->oldSamples) {
                    fprintf(stderr, "  remaining old sample, dts = %s\n", remaining.second->decodeTime().toString().utf8().data());
                }

                current->oldSamples.insert({ decodeKey, WTF::move(sample) });
                m_currentState = StateVacating { WTF::move(current->oldSamples) };
            } else {
                // TODO: check if exceeded deadline
                DEBUG_LOG(LOGIDENTIFIER, "Not yet switching to GOP with DTS=", sample->decodeTime().toDouble());
                fprintf(stderr, "  not yet switching to new gop, dts: %s\n", sample->decodeTime().toString().utf8().data());
                current->newGop.erase(current->newGop.begin(), current->newGop.upper_bound(decodeKey)); // FIXME we clear above already

                if (sample->presentationTime() <= m_highestDeliveredPTS)
                    current->newGop.insert({ decodeKey, sample->createNonDisplayingCopy() });
                else
                    current->newGop.insert({ decodeKey, WTF::move(sample) });
                // TODO be careful to not use sample again when handling the Vacating state
            }
        } else {
            if (sample->presentationTime() <= m_highestDeliveredPTS)
                current->newGop.insert({ decodeKey, sample->createNonDisplayingCopy() });
            else
                current->newGop.insert({ decodeKey, WTF::move(sample) });

            bool needToRewind = true;
            for (auto oldSample : current->oldSamples) {
                if (decodeKey <= oldSample.first) {
                    needToRewind = false;
                    break;
                }
            }

            if (needToRewind) {
                DEBUG_LOG(LOGIDENTIFIER, "New GOP exceeds old samples, need to rewind with non-displaying samples");
                fprintf(stderr, "  rewind\n");

                m_currentState = StateVacating { WTF::move(current->newGop) };
            }
        }
    } else if (auto* current = std::get_if<StateVacating>(&m_currentState)) {
        fprintf(stderr, "  (in vacating state)\n");
        current->vacatingQueue.insert({ decodeKey, WTF::move(sample) });
    }

    tryFeedInnerSink();
}

void SmoothSwitchOnGOP::allSamplesEnqueued()
{
    DEBUG_LOG(LOGIDENTIFIER);

    m_allSamplesEnqueued = true;
    // TODO: do stuff with the flag
}

void SmoothSwitchOnGOP::flush()
{
    DEBUG_LOG(LOGIDENTIFIER);

    m_innerSink->flush();
}

void SmoothSwitchOnGOP::handleChangeInAlreadyEnqueuedContent(DecodeOrderSampleMap::MapType notYetEnqueuedSamples)
{
    // fprintf(stderr, "handleChangeInAlreadyEnqueuedContent, notYetEnqueued: %lu\n", notYetEnqueuedSamples.size());
    // for (auto& pair : notYetEnqueuedSamples) {
    //     DecodeOrderSampleMap::KeyType decodeKey = pair.first;
    //     m_oldSamples.insert_or_assign(decodeKey, protect(pair.second));
    // }
    fprintf(stderr, "handleChangeInAlreadyEnqueuedContent, not yet enqueued: %lu\n", notYetEnqueuedSamples.size());

    if (std::holds_alternative<StateCollecting>(m_currentState)) {
        DEBUG_LOG(LOGIDENTIFIER, "Ignoring content change due to already being in collecting state");
        return;
    }

    DEBUG_LOG(LOGIDENTIFIER, "Transferred not yet enqueued samples, will start collecting new frames");
    m_currentState = StateCollecting { WTF::move(notYetEnqueuedSamples), {} };
}

void SmoothSwitchOnGOP::deliverSample(Ref<MediaSample>&& sample) {
    if (m_highestDeliveredPTS.isInvalid() || sample->presentationTime() > m_highestDeliveredPTS)
        m_highestDeliveredPTS = sample->presentationTime();

    m_lastDeliveredDecodeKey = { sample->decodeTime(), sample->presentationTime() };
    fprintf(stderr, "Delivering sample with dts: %s\n", sample->decodeTime().toString().utf8().data());
    m_innerSink->enqueueSample(WTF::move(sample));
}

void SmoothSwitchOnGOP::tryFeedInnerSink()
{
    // Feed old samples
    // while (!m_oldSamples.empty()) {
    //     if (!m_innerSink->isReadyForMoreSamples())
    //         return;
    //     Ref<MediaSample> sample = m_oldSamples.begin()->second;
    //     m_oldSamples.erase(m_oldSamples.begin());
    //     deliverSample(WTF::move(sample));
    // }
    // TODO: feed new samples?

    // ASSERT(m_oldSamples.empty());
    // fprintf(stderr, "tryFeedInnerSink\n");

    // while (!m_newSamples.empty()) {
    //     if (!m_innerSink->isReadyForMoreSamples()) {
    //         fprintf(stderr, "  returning early, %lu new samples remaining\n", m_newSamples.size());
    //         return;
    //     }
    //     Ref<MediaSample> sample = m_newSamples.begin()->second;
    //     m_newSamples.erase(m_newSamples.begin());
    //     deliverSample(WTF::move(sample));
    // }

    // fprintf(stderr, "  fed all samples\n");

    if (auto* current = std::get_if<StateVacating>(&m_currentState)) {
        while (m_innerSink->isReadyForMoreSamples() && !current->vacatingQueue.empty()) {
            Ref<MediaSample> sample = current->vacatingQueue.begin()->second;
            current->vacatingQueue.erase(current->vacatingQueue.begin());
            deliverSample(WTF::move(sample));
        }
    }
}

#if !RELEASE_LOG_DISABLED
WTFLogChannel& SmoothSwitchOnGOP::logChannel() const
{
    return LogMediaSource;
}
#endif

} // namespace WebCore
