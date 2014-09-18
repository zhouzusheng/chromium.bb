/*
 * Copyright (C) 2013 Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <blpwtk2_prefstore.h>

#include <base/values.h>

namespace blpwtk2 {

PrefStore::PrefStore() {

}

PrefStore::~PrefStore() {

}

void PrefStore::AddObserver(PrefStore::Observer* observer)
{
    d_observers.AddObserver(observer);
}

void PrefStore::RemoveObserver(PrefStore::Observer* observer)
{
    d_observers.RemoveObserver(observer);
}

bool PrefStore::HasObservers() const
{
    return !d_observers.might_have_observers();
}

bool PrefStore::IsInitializationComplete() const
{
    return true;
}

bool PrefStore::GetValue(const std::string& key, const Value** result) const
{
    return d_prefs.GetValue(key, result);
}

bool PrefStore::GetMutableValue(const std::string& key, Value** result)
{
    return d_prefs.GetValue(key, result);
}

void PrefStore::SetValue(const std::string& key, Value* value)
{
    if (d_prefs.SetValue(key, value))
        ReportValueChanged(key);
}

void PrefStore::SetValueSilently(const std::string& key, Value* value)
{
    d_prefs.SetValue(key, value);
}

void PrefStore::RemoveValue(const std::string& key)
{
    if (d_prefs.RemoveValue(key))
        ReportValueChanged(key);
}

void PrefStore::MarkNeedsEmptyValue(const std::string& key)
{
    // Do nothing
}

bool PrefStore::ReadOnly() const
{
    return false;
}

PersistentPrefStore::PrefReadError PrefStore::GetReadError() const
{
    return PersistentPrefStore::PREF_READ_ERROR_NONE;
}

PersistentPrefStore::PrefReadError PrefStore::ReadPrefs()
{
    OnInitializationCompleted();
    return PersistentPrefStore::PREF_READ_ERROR_NONE;
}

void PrefStore::ReadPrefsAsync(ReadErrorDelegate* error_delegate_raw)
{
    scoped_ptr<ReadErrorDelegate> error_delegate(error_delegate_raw);
    OnInitializationCompleted();
}

void PrefStore::CommitPendingWrite()
{
    // Do nothing
}

void PrefStore::ReportValueChanged(const std::string& key)
{
    FOR_EACH_OBSERVER(PrefStore::Observer, d_observers, OnPrefValueChanged(key));
}

void PrefStore::OnInitializationCompleted()
{
    FOR_EACH_OBSERVER(PrefStore::Observer, d_observers, OnInitializationCompleted(true));
}

}  // close namespace blpwtk2
