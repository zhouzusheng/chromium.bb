/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
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

#ifndef SVGPolylineElement_h
#define SVGPolylineElement_h

#include "SVGNames.h"
#include "core/svg/SVGPolyElement.h"

namespace WebCore {

class SVGPolylineElement FINAL : public SVGPolyElement {
public:
    static PassRefPtr<SVGPolylineElement> create(const QualifiedName&, Document&);

private:
    SVGPolylineElement(const QualifiedName&, Document&);
};

inline SVGPolylineElement* toSVGPolylineElement(Node* node)
{
    ASSERT_WITH_SECURITY_IMPLICATION(!node || node->hasTagName(SVGNames::polylineTag));
    return static_cast<SVGPolylineElement*>(node);
}

} // namespace WebCore

#endif