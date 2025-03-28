/*
 * Copyright (C) 2004, 2005, 2009 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "RenderTreeAsText.h"
#include <wtf/text/TextStream.h>

namespace WebCore {

class LegacyRenderSVGContainer;
class LegacyRenderSVGImage;
class LegacyRenderSVGResourceContainer;
class LegacyRenderSVGRoot;
class LegacyRenderSVGShape;
class RenderElement;
class RenderObject;
class RenderSVGGradientStop;
class RenderSVGInlineText;
class RenderSVGText;
class SVGGraphicsElement;

// functions used by the main RenderTreeAsText code
void write(WTF::TextStream&, const LegacyRenderSVGRoot&, OptionSet<RenderAsTextFlag>);
void write(WTF::TextStream&, const LegacyRenderSVGShape&, OptionSet<RenderAsTextFlag>);
void writeSVGGradientStop(WTF::TextStream&, const RenderSVGGradientStop&, OptionSet<RenderAsTextFlag>);
void writeSVGResourceContainer(WTF::TextStream&, const LegacyRenderSVGResourceContainer&, OptionSet<RenderAsTextFlag>);
void writeSVGContainer(WTF::TextStream&, const LegacyRenderSVGContainer&, OptionSet<RenderAsTextFlag>);
void writeSVGGraphicsElement(WTF::TextStream&, const SVGGraphicsElement&);
void writeSVGImage(WTF::TextStream&, const LegacyRenderSVGImage&, OptionSet<RenderAsTextFlag>);
void writeSVGInlineText(WTF::TextStream&, const RenderSVGInlineText&, OptionSet<RenderAsTextFlag>);
void writeSVGPaintingFeatures(TextStream&, const RenderElement&, OptionSet<RenderAsTextFlag>);
void writeSVGText(WTF::TextStream&, const RenderSVGText&, OptionSet<RenderAsTextFlag>);
void writeResources(WTF::TextStream&, const RenderObject&, OptionSet<RenderAsTextFlag>);

// helper operators specific to dumping the render tree. these are used in various classes to dump the render tree
// these could be defined in separate namespace to avoid matching these generic signatures unintentionally.

template<typename Item>
WTF::TextStream& operator<<(WTF::TextStream& ts, const Vector<Item*>& v)
{
    ts << '[';

    for (unsigned i = 0; i < v.size(); i++) {
        ts << *v[i];
        if (i < v.size() - 1)
            ts << ", "_s;
    }

    ts << ']';
    return ts;
}

} // namespace WebCore
