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

#include <blpwtk2_utility.h>
#include <blpwtk2_webviewproxy.h>

#include <base/command_line.h>
#include <base/json/json_writer.h>
#include <base/sys_info.h>
#include <content/browser/gpu/compositor_util.h>
#include <content/browser/gpu/gpu_data_manager_impl.h>
#include <content/browser/gpu/gpu_internals_ui.h>
#include <third_party/angle/src/common/version.h>
#include <fstream>

namespace blpwtk2 {

void DumpGpuInfo(std::string path) {
    base::DictionaryValue gpuInfo;
    gpuInfo.Set("featureStatus", content::GetFeatureStatus());
    gpuInfo.Set("problems", content::GetProblems());

    base::ListValue* workarounds = new base::ListValue();
    for (const auto& workaround : content::GetDriverBugWorkarounds()) {
        workarounds->AppendString(workaround);
    }

    gpuInfo.Set("workarounds", workarounds);
    gpuInfo.Set("memoryBufferInfo", content::GpuInternalsUI::GetGpuMemoryBufferInfo());
    gpuInfo.Set("LogMessage", content::GpuDataManagerImpl::GetInstance()->GetLogMessages());

    gpuInfo.SetString("command_line",
        base::CommandLine::ForCurrentProcess()->GetCommandLineString());

    gpuInfo.SetString("operating_system",
        base::SysInfo::OperatingSystemName() + " " +
        base::SysInfo::OperatingSystemVersion());

    gpuInfo.SetString("angle_commit_id", ANGLE_COMMIT_HASH);
    gpuInfo.SetString("graphics_backend", "Skia");
    gpuInfo.SetString("blacklist_version",
        content::GpuDataManagerImpl::GetInstance()->GetBlacklistVersion());

    gpuInfo.SetString("driver_bug_list_version",
        content::GpuDataManagerImpl::GetInstance()->GetDriverBugListVersion());

    std::string json;
    base::JSONWriter::Write(gpuInfo, &json);

    std::ofstream file(path, std::ios::binary);
    file << json;
    file.close();
}
}  // close namespace blpwtk2
