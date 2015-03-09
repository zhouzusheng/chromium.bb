// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/spellchecker/spellcheck.h"

#include "base/bind.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/common/render_messages.h"
#include "chrome/common/spellcheck_common.h"
#include "chrome/common/spellcheck_messages.h"
#include "chrome/common/spellcheck_result.h"
#include "chrome/renderer/spellchecker/spellcheck_language.h"
#include "chrome/renderer/spellchecker/spellcheck_provider.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_visitor.h"
#include "third_party/WebKit/public/web/WebTextCheckingCompletion.h"
#include "third_party/WebKit/public/web/WebTextCheckingResult.h"
#include "third_party/WebKit/public/web/WebTextDecorationType.h"
#include "third_party/WebKit/public/web/WebView.h"

using blink::WebVector;
using blink::WebString;
using blink::WebTextCheckingResult;
using blink::WebTextDecorationType;
using chrome::spellcheck_common::FileLanguagePair;

namespace {

class UpdateSpellcheckEnabled : public content::RenderViewVisitor {
 public:
  explicit UpdateSpellcheckEnabled(bool enabled) : enabled_(enabled) {}
  bool Visit(content::RenderView* render_view) override;

 private:
  bool enabled_;  // New spellcheck-enabled state.
  DISALLOW_COPY_AND_ASSIGN(UpdateSpellcheckEnabled);
};

bool UpdateSpellcheckEnabled::Visit(content::RenderView* render_view) {
  SpellCheckProvider* provider = SpellCheckProvider::Get(render_view);
  DCHECK(provider);
  provider->EnableSpellcheck(enabled_);
  return true;
}

class RequestSpellcheckForView : public content::RenderViewVisitor {
 public:
  RequestSpellcheckForView() {}
  virtual bool Visit(content::RenderView* render_view) OVERRIDE;
 private:
  DISALLOW_COPY_AND_ASSIGN(RequestSpellcheckForView);
};

bool RequestSpellcheckForView::Visit(content::RenderView* render_view) {
  SpellCheckProvider* provider = SpellCheckProvider::Get(render_view);
  DCHECK(provider);
  provider->RequestSpellcheck();
  return true;
}

class DocumentMarkersCollector : public content::RenderViewVisitor {
 public:
  DocumentMarkersCollector() {}
  ~DocumentMarkersCollector() override {}
  const std::vector<uint32>& markers() const { return markers_; }
  bool Visit(content::RenderView* render_view) override;

 private:
  std::vector<uint32> markers_;
  DISALLOW_COPY_AND_ASSIGN(DocumentMarkersCollector);
};

bool DocumentMarkersCollector::Visit(content::RenderView* render_view) {
  if (!render_view || !render_view->GetWebView())
    return true;
  WebVector<uint32> markers;
  render_view->GetWebView()->spellingMarkers(&markers);
  for (size_t i = 0; i < markers.size(); ++i)
    markers_.push_back(markers[i]);
  // Visit all render views.
  return true;
}

class DocumentMarkersRemover : public content::RenderViewVisitor {
 public:
  explicit DocumentMarkersRemover(const std::vector<std::string>& words);
  ~DocumentMarkersRemover() override {}
  bool Visit(content::RenderView* render_view) override;

 private:
  WebVector<WebString> words_;
  DISALLOW_COPY_AND_ASSIGN(DocumentMarkersRemover);
};

DocumentMarkersRemover::DocumentMarkersRemover(
    const std::vector<std::string>& words)
    : words_(words.size()) {
  for (size_t i = 0; i < words.size(); ++i)
    words_[i] = WebString::fromUTF8(words[i]);
}

bool DocumentMarkersRemover::Visit(content::RenderView* render_view) {
  if (render_view && render_view->GetWebView())
    render_view->GetWebView()->removeSpellingMarkersUnderWords(words_);
  return true;
}

}  // namespace

class SpellCheck::SpellcheckRequest {
 public:
  SpellcheckRequest(const base::string16& text,
                    blink::WebTextCheckingCompletion* completion)
      : text_(text), completion_(completion) {
    DCHECK(completion);
  }
  ~SpellcheckRequest() {}

  base::string16 text() { return text_; }
  blink::WebTextCheckingCompletion* completion() { return completion_; }

 private:
  base::string16 text_;  // Text to be checked in this task.

  // The interface to send the misspelled ranges to WebKit.
  blink::WebTextCheckingCompletion* completion_;

  DISALLOW_COPY_AND_ASSIGN(SpellcheckRequest);
};


// Initializes SpellCheck object.
// spellcheck_enabled_ currently MUST be set to true, due to peculiarities of
// the initialization sequence.
// Since it defaults to true, newly created SpellCheckProviders will enable
// spellchecking. After the first word is typed, the provider requests a check,
// which in turn triggers the delayed initialization sequence in SpellCheck.
// This does send a message to the browser side, which triggers the creation
// of the SpellcheckService. That does create the observer for the preference
// responsible for enabling/disabling checking, which allows subsequent changes
// to that preference to be sent to all SpellCheckProviders.
// Setting |spellcheck_enabled_| to false by default prevents that mechanism,
// and as such the SpellCheckProviders will never be notified of different
// values.
// TODO(groby): Simplify this.
SpellCheck::SpellCheck()
    : auto_spell_correct_behavior_(chrome::spellcheck_common::AUTOCORRECT_NONE),
      spellcheck_enabled_(true) {
}

SpellCheck::~SpellCheck() {
}

bool SpellCheck::OnControlMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(SpellCheck, message)
    IPC_MESSAGE_HANDLER(SpellCheckMsg_Init, OnInit)
    IPC_MESSAGE_HANDLER(SpellCheckMsg_CustomDictionaryChanged,
                        OnCustomDictionaryChanged)
    IPC_MESSAGE_HANDLER(SpellCheckMsg_AutocorrectWordsChanged,
                        OnAutocorrectWordsChanged)
    IPC_MESSAGE_HANDLER(SpellCheckMsg_SetAutoSpellCorrectBehavior,
                        OnSetAutoSpellCorrectBehavior)
    IPC_MESSAGE_HANDLER(SpellCheckMsg_EnableSpellCheck, OnEnableSpellCheck)
    IPC_MESSAGE_HANDLER(SpellCheckMsg_RequestDocumentMarkers,
                        OnRequestDocumentMarkers)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void SpellCheck::OnInit(const std::vector<FileLanguagePair>& languages,
                        const std::set<std::string>& custom_words,
                        const std::map<std::string, std::string>& autocorrect_words,
                        int auto_spell_correct_behavior) {
  Init(languages, custom_words, autocorrect_words);
  auto_spell_correct_behavior_ = auto_spell_correct_behavior;
#if !defined(OS_MACOSX)
  PostDelayedSpellCheckTask(pending_request_param_.release());
#endif
}

void SpellCheck::OnCustomDictionaryChanged(
    const std::vector<std::string>& words_added,
    const std::vector<std::string>& words_removed) {
  custom_dictionary_.OnCustomDictionaryChanged(words_added, words_removed);
  if (spellcheck_enabled_) {
    RequestSpellcheckForView requestor;
    content::RenderView::ForEach(&requestor);
  }
}

void SpellCheck::OnAutocorrectWordsChanged(
    const std::map<std::string, std::string>& words_added,
    const std::vector<std::string>& words_removed) {
  typedef std::map<std::string, std::string>::const_iterator SrcIterator;
  typedef std::map<base::string16, base::string16>::iterator DestIterator;
  for (SrcIterator it = words_added.begin(); it != words_added.end(); ++it) {
    base::string16 badWord = base::UTF8ToUTF16(it->first);
    autocorrect_words_[badWord] = base::UTF8ToUTF16(it->second);
  }
  for (size_t i = 0; i < words_removed.size(); ++i) {
    DestIterator it = autocorrect_words_.find(base::UTF8ToUTF16(words_removed[i]));
    if (it != autocorrect_words_.end()) {
      autocorrect_words_.erase(it);
    }
  }
}

void SpellCheck::OnSetAutoSpellCorrectBehavior(int flags) {
  auto_spell_correct_behavior_ = flags;
}

void SpellCheck::OnEnableSpellCheck(bool enable) {
  spellcheck_enabled_ = enable;
  UpdateSpellcheckEnabled updater(enable);
  content::RenderView::ForEach(&updater);
}

void SpellCheck::OnRequestDocumentMarkers() {
  DocumentMarkersCollector collector;
  content::RenderView::ForEach(&collector);
  content::RenderThread::Get()->Send(
      new SpellCheckHostMsg_RespondDocumentMarkers(collector.markers()));
}

// TODO(groby): Make sure we always have a spelling engine, even before Init()
// is called.
void SpellCheck::Init(const std::vector<FileLanguagePair>& languages,
                      const std::set<std::string>& custom_words,
                      const std::map<std::string, std::string>& autocorrect_words) {
  size_t langCount = languages.size();
  spellcheck_.clear();

  for (size_t langIndex = 0; langIndex < langCount; langIndex++) {
    SpellcheckLanguage *language = new SpellcheckLanguage();
    const FileLanguagePair& flp = languages[langIndex];
    language->Init(IPC::PlatformFileForTransitToFile(flp.first),
                   flp.second);

    spellcheck_[language->GetScriptCode()].push_back(language);
  }

  custom_dictionary_.Init(custom_words);

  typedef std::map<std::string, std::string>::const_iterator MapIterator;
  autocorrect_words_.clear();
  for (MapIterator it = autocorrect_words.begin();
                   it != autocorrect_words.end(); ++it) {
    base::string16 badWord = base::UTF8ToUTF16(it->first);
    autocorrect_words_[badWord] = base::UTF8ToUTF16(it->second);
  }
}

bool SpellCheck::SpellCheckWordInScript(
    const ScopedVector<SpellcheckLanguage>& languages,
    const base::char16* in_word,
    int in_word_len,
    int tag,
    int* misspelling_start,
    int* misspelling_len,
    bool checkForContractions,
    std::set<base::string16>* suggestions_set) {
  DCHECK(in_word_len >= 0);
  DCHECK(misspelling_start && misspelling_len) << "Out vars must be given.";

  *misspelling_start = *misspelling_len = 0;

  int checked_offset = 0;
  while (checked_offset < in_word_len) {
    // Find the first misspelled word in the first language.

    SpellcheckLanguage *firstLang = languages[0];
    std::vector<base::string16> suggestions_vector;
    int segment_misspelling_start;
    int segment_misspelling_len;

    if (firstLang->SpellCheckWord(
        in_word + checked_offset,
        in_word_len - checked_offset,
        tag,
        &segment_misspelling_start,
        &segment_misspelling_len,
        checkForContractions,
        suggestions_set ? &suggestions_vector : 0)) {
      // Everything is OK in the first language, no need to check the other
      // languages.
      return true;
    }

    if (suggestions_set) {
      for (std::size_t i = 0; i < suggestions_vector.size(); ++i) {
        suggestions_set->insert(suggestions_vector[i]);
      }
    }

    // We have a misspelling in the first language!  See if this word is
    // recognized in the custom dictionary or any of the other languages.

    bool alternativeFound =
        custom_dictionary_.SpellCheckWord(in_word + checked_offset,
                                          segment_misspelling_start,
                                          segment_misspelling_len);

    for (size_t langIndex = 1;
        langIndex < languages.size() && !alternativeFound;
        ++langIndex) {
      SpellcheckLanguage *language = languages[langIndex];
      suggestions_vector.clear();

      int tmpStart, tmpLen;
      alternativeFound = language->SpellCheckWord(
          in_word + checked_offset + segment_misspelling_start,
          segment_misspelling_len,
          tag,
          &tmpStart, &tmpLen,
          checkForContractions,
          suggestions_set ? &suggestions_vector : 0);

      if (suggestions_set) {
        for (std::size_t i = 0; i < suggestions_vector.size(); ++i) {
          suggestions_set->insert(suggestions_vector[i]);
        }
      }
    }

    if (!alternativeFound) {
      *misspelling_start = checked_offset + segment_misspelling_start;
      *misspelling_len = segment_misspelling_len;
      return false;
    }
    else {
      checked_offset += segment_misspelling_start + segment_misspelling_len;
    }
  }

  return true;
}

bool SpellCheck::SpellCheckWord(
    const base::char16* in_word,
    int in_word_len,
    int tag,
    int* misspelling_start,
    int* misspelling_len,
    bool checkForContractions,
    std::vector<base::string16>* optional_suggestions) {
  DCHECK(in_word_len >= 0);
  DCHECK(misspelling_start && misspelling_len) << "Out vars must be given.";

  // Do nothing if we need to delay initialization. (Rather than blocking,
  // report the word as correctly spelled.)
  if (InitializeIfNeeded())
    return true;

  *misspelling_start = *misspelling_len = 0;
  if (spellcheck_.empty()) {
    return true;
  }

  std::set<base::string16> suggestions_set;
  std::map<UScriptCode, ScopedVector<SpellcheckLanguage> >::iterator it;

  for (it = spellcheck_.begin(); it != spellcheck_.end(); ++it) {
    int tmp_misspelling_start, tmp_misspelling_len;

    bool found_misspelling = !SpellCheckWordInScript(
          it->second,
          in_word,
          in_word_len,
          tag,
          &tmp_misspelling_start,
          &tmp_misspelling_len,
          checkForContractions,
          optional_suggestions ? &suggestions_set : 0);

    if (!found_misspelling)
      continue;

    if (!*misspelling_len || tmp_misspelling_start < *misspelling_start) {
      *misspelling_start = tmp_misspelling_start;
      *misspelling_len = tmp_misspelling_len;
    }
  }

  if (optional_suggestions) {
    for (std::set<base::string16>::const_iterator it = suggestions_set.begin();
         it != suggestions_set.end();
         ++it)
    {
      optional_suggestions->push_back(*it);
    }
  }

  return !*misspelling_len;
}

bool SpellCheck::SpellCheckParagraph(
    const base::string16& text,
    WebVector<WebTextCheckingResult>* results) {
#if !defined(OS_MACOSX)
  // Mac has its own spell checker, so this method will not be used.
  DCHECK(results);
  std::vector<WebTextCheckingResult> textcheck_results;
  size_t length = text.length();
  size_t offset = 0;

  // Spellcheck::SpellCheckWord() automatically breaks text into words and
  // checks the spellings of the extracted words. This function sets the
  // position and length of the first misspelled word and returns false when
  // the text includes misspelled words. Therefore, we just repeat calling the
  // function until it returns true to check the whole text.
  int misspelling_start = 0;
  int misspelling_length = 0;
  while (offset <= length) {
    if (SpellCheckWord(&text[offset],
                       length - offset,
                       0,
                       &misspelling_start,
                       &misspelling_length,
                       true,
                       NULL)) {
      results->assign(textcheck_results);
      return true;
    }

    if (!custom_dictionary_.SpellCheckWord(
            text, misspelling_start + offset, misspelling_length)) {
      base::string16 replacement;
      textcheck_results.push_back(WebTextCheckingResult(
          blink::WebTextDecorationTypeSpelling,
          misspelling_start + offset,
          misspelling_length,
          replacement));
    }
    offset += misspelling_start + misspelling_length;
  }
  results->assign(textcheck_results);
  return false;
#else
  // This function is only invoked for spell checker functionality that runs
  // on the render thread. OSX builds don't have that.
  NOTREACHED();
  return true;
#endif
}

base::string16 SpellCheck::GetAutoCorrectionWord(const base::string16& word,
                                                 int tag) {
  if (auto_spell_correct_behavior_ & chrome::spellcheck_common::AUTOCORRECT_WORD_MAP) {
    typedef std::map<base::string16, base::string16> WordMap;
    WordMap::const_iterator it = autocorrect_words_.find(word);
    if (it != autocorrect_words_.end()) {
      return it->second;  // Return the configured correction for 'word'.
    }
  }

  base::string16 autocorrect_word;
  if (!(auto_spell_correct_behavior_ & chrome::spellcheck_common::AUTOCORRECT_SWAP_ADJACENT_CHARS))
    return autocorrect_word;  // Return the empty string.

  int word_length = static_cast<int>(word.size());
  if (word_length < 2 ||
      word_length > chrome::spellcheck_common::kMaxAutoCorrectWordSize)
    return autocorrect_word;

  if (InitializeIfNeeded())
    return autocorrect_word;

  base::char16 misspelled_word[
      chrome::spellcheck_common::kMaxAutoCorrectWordSize + 1];
  const base::char16* word_char = word.c_str();
  for (int i = 0; i <= chrome::spellcheck_common::kMaxAutoCorrectWordSize;
       ++i) {
    if (i >= word_length)
      misspelled_word[i] = 0;
    else
      misspelled_word[i] = word_char[i];
  }

  // Swap adjacent characters and spellcheck.
  int misspelling_start, misspelling_len;
  for (int i = 0; i < word_length - 1; i++) {
    // Swap.
    std::swap(misspelled_word[i], misspelled_word[i + 1]);

    // Check spelling.
    misspelling_start = misspelling_len = 0;
    SpellCheckWord(misspelled_word, word_length, tag, &misspelling_start,
                   &misspelling_len, false, NULL);

    // Make decision: if only one swap produced a valid word, then we want to
    // return it. If we found two or more, we don't do autocorrection.
    if (misspelling_len == 0) {
      if (autocorrect_word.empty()) {
        autocorrect_word.assign(misspelled_word);
      } else {
        autocorrect_word.clear();
        break;
      }
    }

    // Restore the swapped characters.
    std::swap(misspelled_word[i], misspelled_word[i + 1]);
  }
  return autocorrect_word;
}

#if !defined(OS_MACOSX)  // OSX uses its own spell checker
void SpellCheck::RequestTextChecking(
    const base::string16& text,
    blink::WebTextCheckingCompletion* completion) {
  // Clean up the previous request before starting a new request.
  if (pending_request_param_.get())
    pending_request_param_->completion()->didCancelCheckingText();

  pending_request_param_.reset(new SpellcheckRequest(
      text, completion));
  // We will check this text after we finish loading the hunspell dictionary.
  if (InitializeIfNeeded())
    return;

  PostDelayedSpellCheckTask(pending_request_param_.release());
}
#endif

bool SpellCheck::InitializeIfNeeded() {
  bool inited = true;
  std::map<UScriptCode, ScopedVector<SpellcheckLanguage> >::iterator it1;

  for (it1 = spellcheck_.begin(); it1 != spellcheck_.end(); ++it1) {
    ScopedVector<SpellcheckLanguage>::iterator it2;

    for (it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
      inited &= (*it2)->InitializeIfNeeded();
    }
  }

  return inited;
}

#if !defined(OS_MACOSX) // OSX doesn't have |pending_request_param_|
void SpellCheck::PostDelayedSpellCheckTask(SpellcheckRequest* request) {
  if (!request)
    return;

  base::MessageLoopProxy::current()->PostTask(FROM_HERE,
      base::Bind(&SpellCheck::PerformSpellCheck,
                 AsWeakPtr(),
                 base::Owned(request)));
}
#endif

#if !defined(OS_MACOSX)  // Mac uses its native engine instead.
void SpellCheck::PerformSpellCheck(SpellcheckRequest* param) {
  DCHECK(param);

  bool allEnabled = !spellcheck_.empty();
  std::map<UScriptCode, ScopedVector<SpellcheckLanguage> >::iterator it1;
  for (it1 = spellcheck_.begin(); it1 != spellcheck_.end(); ++it1) {
    ScopedVector<SpellcheckLanguage>::iterator it2;

    for (it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
      allEnabled &= (*it2)->IsEnabled();
    }
  }

  if (!allEnabled) {
    param->completion()->didCancelCheckingText();
  } else {
    WebVector<blink::WebTextCheckingResult> results;
    SpellCheckParagraph(param->text(), &results);
    param->completion()->didFinishCheckingText(results);
  }
}
#endif

void SpellCheck::CreateTextCheckingResults(
    ResultFilter filter,
    int line_offset,
    const base::string16& line_text,
    const std::vector<SpellCheckResult>& spellcheck_results,
    WebVector<WebTextCheckingResult>* textcheck_results) {
  // Double-check misspelled words with our spellchecker and attach grammar
  // markers to them if our spellchecker tells they are correct words, i.e. they
  // are probably contextually-misspelled words.
  const base::char16* text = line_text.c_str();
  std::vector<WebTextCheckingResult> list;
  for (size_t i = 0; i < spellcheck_results.size(); ++i) {
    SpellCheckResult::Decoration decoration = spellcheck_results[i].decoration;
    int word_location = spellcheck_results[i].location;
    int word_length = spellcheck_results[i].length;
    int misspelling_start = 0;
    int misspelling_length = 0;
    if (decoration == SpellCheckResult::SPELLING &&
        filter == USE_NATIVE_CHECKER) {
      if (SpellCheckWord(text + word_location, word_length, 0,
                         &misspelling_start, &misspelling_length, true, NULL)) {
        decoration = SpellCheckResult::GRAMMAR;
      }
    }
    if (!custom_dictionary_.SpellCheckWord(
            line_text, word_location, word_length)) {
      list.push_back(WebTextCheckingResult(
          static_cast<WebTextDecorationType>(decoration),
          word_location + line_offset,
          word_length,
          spellcheck_results[i].replacement,
          spellcheck_results[i].hash));
    }
  }
  textcheck_results->assign(list);
}
