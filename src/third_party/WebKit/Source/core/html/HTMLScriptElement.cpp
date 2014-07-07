/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "config.h"
#include "core/html/HTMLScriptElement.h"

#include "HTMLNames.h"
#include "bindings/v8/ExceptionStatePlaceholder.h"
#include "bindings/v8/ScriptEventListener.h"
#include "core/dom/Attribute.h"
#include "core/dom/Document.h"
#include "core/dom/Event.h"
#include "core/dom/EventNames.h"
#include "core/dom/ScriptLoader.h"
#include "core/dom/Text.h"

namespace WebCore {

using namespace HTMLNames;

inline HTMLScriptElement::HTMLScriptElement(const QualifiedName& tagName, Document& document, bool wasInsertedByParser, bool alreadyStarted)
    : HTMLElement(tagName, document)
    , m_loader(ScriptLoader::create(this, wasInsertedByParser, alreadyStarted))
{
    ASSERT(hasTagName(scriptTag));
    ScriptWrappable::init(this);
}

PassRefPtr<HTMLScriptElement> HTMLScriptElement::create(const QualifiedName& tagName, Document& document, bool wasInsertedByParser, bool alreadyStarted)
{
    return adoptRef(new HTMLScriptElement(tagName, document, wasInsertedByParser, alreadyStarted));
}

bool HTMLScriptElement::isURLAttribute(const Attribute& attribute) const
{
    return attribute.name() == srcAttr || HTMLElement::isURLAttribute(attribute);
}

void HTMLScriptElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    HTMLElement::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);
    m_loader->childrenChanged();
}

void HTMLScriptElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
{
    if (name == srcAttr)
        m_loader->handleSourceAttribute(value);
    else if (name == asyncAttr)
        m_loader->handleAsyncAttribute();
    else if (name == onbeforeloadAttr)
        setAttributeEventListener(eventNames().beforeloadEvent, createAttributeEventListener(this, name, value));
    else
        HTMLElement::parseAttribute(name, value);
}

Node::InsertionNotificationRequest HTMLScriptElement::insertedInto(ContainerNode* insertionPoint)
{
    HTMLElement::insertedInto(insertionPoint);
    return InsertionShouldCallDidNotifySubtreeInsertions;
}

void HTMLScriptElement::didNotifySubtreeInsertionsToDocument()
{
    m_loader->didNotifySubtreeInsertionsToDocument();
}

void HTMLScriptElement::setText(const String &value)
{
    RefPtr<Node> protectFromMutationEvents(this);

    if (hasOneTextChild()) {
        toText(firstChild())->setData(value);
        return;
    }

    removeChildren();
    appendChild(document().createTextNode(value.impl()), IGNORE_EXCEPTION);
}

void HTMLScriptElement::setAsync(bool async)
{
    setBooleanAttribute(asyncAttr, async);
    m_loader->handleAsyncAttribute();
}

bool HTMLScriptElement::async() const
{
    return fastHasAttribute(asyncAttr) || (m_loader->forceAsync());
}

KURL HTMLScriptElement::src() const
{
    return document().completeURL(sourceAttributeValue());
}

void HTMLScriptElement::addSubresourceAttributeURLs(ListHashSet<KURL>& urls) const
{
    HTMLElement::addSubresourceAttributeURLs(urls);

    addSubresourceURL(urls, src());
}

String HTMLScriptElement::sourceAttributeValue() const
{
    return getAttribute(srcAttr).string();
}

String HTMLScriptElement::charsetAttributeValue() const
{
    return getAttribute(charsetAttr).string();
}

String HTMLScriptElement::typeAttributeValue() const
{
    return getAttribute(typeAttr).string();
}

String HTMLScriptElement::languageAttributeValue() const
{
    return getAttribute(languageAttr).string();
}

String HTMLScriptElement::forAttributeValue() const
{
    return getAttribute(forAttr).string();
}

String HTMLScriptElement::eventAttributeValue() const
{
    return getAttribute(eventAttr).string();
}

bool HTMLScriptElement::asyncAttributeValue() const
{
    return fastHasAttribute(asyncAttr);
}

bool HTMLScriptElement::deferAttributeValue() const
{
    return fastHasAttribute(deferAttr);
}

bool HTMLScriptElement::hasSourceAttribute() const
{
    return fastHasAttribute(srcAttr);
}

void HTMLScriptElement::dispatchLoadEvent()
{
    ASSERT(!m_loader->haveFiredLoadEvent());
    dispatchEvent(Event::create(eventNames().loadEvent));
}

PassRefPtr<Element> HTMLScriptElement::cloneElementWithoutAttributesAndChildren()
{
    return adoptRef(new HTMLScriptElement(tagQName(), document(), false, m_loader->alreadyStarted()));
}

}