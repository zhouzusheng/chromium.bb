/*
 * Copyright (C) 2011 Leo Yang <leoyang@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGAltGlyphDefElement_h
#define SVGAltGlyphDefElement_h

#if ENABLE(SVG_FONTS)
#include "core/svg/SVGElement.h"
#include "wtf/Vector.h"

namespace WebCore {

class SVGAltGlyphDefElement FINAL : public SVGElement {
public:
    static PassRefPtr<SVGAltGlyphDefElement> create(const QualifiedName&, Document&);

    bool hasValidGlyphElements(Vector<String>& glyphNames) const;

private:
    SVGAltGlyphDefElement(const QualifiedName&, Document&);

    virtual bool rendererIsNeeded(const RenderStyle&) OVERRIDE { return false; }
};

}

#endif
#endif