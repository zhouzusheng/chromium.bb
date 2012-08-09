// Copyright 2011 the v8-i18n authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "include/extension.h"

#include "src/break-iterator.h"
#include "src/collator.h"
#include "src/datetime-format.h"
#include "src/locale.h"
#include "src/natives.h"
#include "src/number-format.h"

namespace v8_i18n {

Extension* Extension::extension_ = NULL;

Extension::Extension()
    : v8::Extension("v8/i18n", Natives::GetScriptSource()) {
}

v8::Handle<v8::FunctionTemplate> Extension::GetNativeFunction(
    v8::Handle<v8::String> name) {
  if (name->Equals(v8::String::New("NativeJSLocale"))) {
    return v8::FunctionTemplate::New(Locale::JSLocale);
  } else if (name->Equals(v8::String::New("NativeJSBreakIterator"))) {
    return v8::FunctionTemplate::New(BreakIterator::JSBreakIterator);
  } else if (name->Equals(v8::String::New("NativeJSCollator"))) {
    return v8::FunctionTemplate::New(Collator::JSCollator);
  } else if (name->Equals(v8::String::New("NativeJSDateTimeFormat"))) {
    return v8::FunctionTemplate::New(DateTimeFormat::JSDateTimeFormat);
  } else if (name->Equals(v8::String::New("NativeJSNumberFormat"))) {
    return v8::FunctionTemplate::New(NumberFormat::JSNumberFormat);
  }

  return v8::Handle<v8::FunctionTemplate>();
}

Extension* Extension::get() {
  if (!extension_) {
    extension_ = new Extension();
  }
  return extension_;
}

void Extension::Register() {
  static v8::DeclareExtension extension_declaration(Extension::get());
}

}  // namespace v8_i18n
