#include "FileDlg.h"
#include <shlobj.h>
#include <memory>

class RegKey {
public:
	RegKey(HKEY hkey, LPCWSTR path, DWORD flag) {
		m_hKey = NULL;
		RegOpenKeyExW(hkey, path, 0, flag, &m_hKey);
	}
	~RegKey() {
		if (m_hKey) {
			RegCloseKey(m_hKey);
		}
	}

	DWORD ReadValue(LPCWSTR key, std::wstring* value) {
		long len = 0;
		DWORD status= ::RegQueryValueW(m_hKey, key, NULL, &len);
		if (len > 0) {
			value->resize(len);
			RegQueryValueW(m_hKey, key, &(*value)[0], &len);
			len = wcslen(value->c_str());
			value->resize(len);
		}
		else {
			*value = L"";
		}
		return status;
	}
private:
	HKEY m_hKey;
};
static bool GetRegistryDescriptionFromExtension(const std::wstring& file_ext,
	std::wstring* reg_description) {
	RegKey reg_ext(HKEY_CLASSES_ROOT, file_ext.c_str(), KEY_READ);
	std::wstring reg_app;
	if (reg_ext.ReadValue(NULL, &reg_app) == ERROR_SUCCESS && !reg_app.empty()) {
		RegKey reg_link(HKEY_CLASSES_ROOT, reg_app.c_str(), KEY_READ);
		if (reg_link.ReadValue(NULL, reg_description) == ERROR_SUCCESS)
			return true;
	}
	return false;
}

std::wstring FormatFilterForExtensions(
	const std::vector<std::wstring>& file_ext,
	const std::vector<std::wstring>& ext_desc,
	bool include_all_files) {
	const std::wstring all_ext = L"*.*";
	const std::wstring all_desc = std::wstring(L"所有文件(") + all_ext + L")";

	if (file_ext.empty())
		include_all_files = true;

	std::wstring result;

	for (size_t i = 0; i < file_ext.size(); ++i) {
		std::wstring ext = file_ext[i];
		std::wstring desc;
		if (i < ext_desc.size())
			desc = ext_desc[i];

		if (ext.empty()) {
			// Force something reasonable to appear in the dialog box if there is no
			// extension provided.
			include_all_files = true;
			continue;
		}

		if (desc.empty()) {
			std::wstring first_extension = ext.substr(ext.find(L'.'));
			size_t first_separator_index = first_extension.find(L';');
			if (first_separator_index != std::wstring::npos)
				first_extension = first_extension.substr(0, first_separator_index);

			// Find the extension name without the preceeding '.' character.
			std::wstring ext_name = first_extension;
			size_t ext_index = ext_name.find_first_not_of(L'.');
			if (ext_index != std::wstring::npos)
				ext_name = ext_name.substr(ext_index);

			if (!GetRegistryDescriptionFromExtension(first_extension, &desc)) {
				// The extension doesn't exist in the registry.
				include_all_files = true;
			}
		}

		if (!desc.empty())
			desc += L" (" + ext + L")";
		else
			desc = ext;

		result.append(desc.c_str(), desc.size() + 1);  // Append NULL too.
		result.append(ext.c_str(), ext.size() + 1);
	}

	if (include_all_files) {
		result.append(all_desc.c_str(), all_desc.size() + 1);
		result.append(all_ext.c_str(), all_ext.size() + 1);
	}

	result.append(1, '\0');  // Double NULL required.
	return result;
}

std::wstring GetDescriptionFromMimeType(const std::string& mime_type) {
	// Check for wild card mime types and return an appropriate description.
	static const struct {
		const char* mime_type;
		std::wstring value;
	} kWildCardMimeTypes[] = {
		{ "audio", L"音频文件" },
		{ "image", L"图像文件" },
		{ "text",  L"文本文件" },
		{ "video", L"视频文件" },
	};

	for (size_t i = 0; i < 4; ++i) {
		if (mime_type == std::string(kWildCardMimeTypes[i].mime_type) + "/*")
			return kWildCardMimeTypes[i].value;
	}
	return std::wstring();
}

void SplitString(const std::wstring& src, wchar_t ch, std::vector<std::wstring>& result) {
	size_t last = 0;
	size_t t = src.find(L';', last);
	while (t != std::wstring::npos) {
		if (t > last) {
			result.push_back(src.substr(last, t - last));
		}
		last = t + 1;
	}

	if (last < src.length()) {
		result.push_back(src.substr(last));
	}
}

std::wstring GetFilterString(
	const std::vector<std::wstring>& accept_filters) {
	std::vector<std::wstring> extensions;
	std::vector<std::wstring> descriptions;

	for (size_t i = 0; i < accept_filters.size(); ++i) {
		const std::wstring& filter = accept_filters[i];
		if (filter.empty())
			continue;

		size_t sep_index = filter.find(L'|');
		if (sep_index != std::wstring::npos) {
			// Treat as a filter of the form "Filter Name|.ext1;.ext2;.ext3".
			const std::wstring& desc = filter.substr(0, sep_index);
			const std::wstring& exts = filter.substr(sep_index + 1);

			std::vector<std::wstring> ext;

			SplitString(exts, L';', ext);
			std::wstring ext_str;
			for (size_t x = 0; x < ext.size(); ++x) {
				const std::wstring& file_ext = ext[x];
				if (!file_ext.empty() && file_ext[0] == '.') {
					if (!ext_str.empty())
						ext_str += L";";
					ext_str += L"*" + file_ext;
				}
			}
			if (!ext_str.empty()) {
				extensions.push_back(ext_str);
				descriptions.push_back(desc);
			}
		}
		else if (filter[0] == L'.') {
			// Treat as an extension beginning with the '.' character.
			extensions.push_back(L"*" + filter);
			descriptions.push_back(std::wstring());
		}
		else {
			//TODO
		}
	}

	return FormatFilterForExtensions(extensions, descriptions, true);
}

void UTF8ToUTF16(const std::string& src, std::wstring* dest) {
	int size = MultiByteToWideChar(CP_UTF8, 0, src.data(), src.length(), NULL, 0);
	dest->resize(size);
	MultiByteToWideChar(CP_UTF8, 0, src.data(), src.length(), &(*dest)[0], size);
}

void UTF16ToUTF8(const std::wstring& src, std::string* dest) {
	int size = WideCharToMultiByte(CP_UTF8, 0, src.data(), src.length(), NULL, 0, NULL, NULL);
	dest->resize(size);
	WideCharToMultiByte(CP_UTF8, 0, src.data(), src.length(), &(*dest)[0], size, NULL, NULL);
}

bool RunOpenFileDialog(
	const minikit::FileChooserParams&  params,
	HWND owner,
	int* filter_index,
	std::wstring* path) {

	OPENFILENAME ofn;

	// We must do this otherwise the ofn's FlagsEx may be initialized to random
	// junk in release builds which can cause the Places Bar not to show up!
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner;

	wchar_t filename[MAX_PATH] = { 0 };

	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;

	std::wstring directory;
	
	if (!params.defaultFileName().isEmpty()) {
		minikit::StringRef defaultFileName = params.defaultFileName();
		if (defaultFileName.data()[defaultFileName.length() - 1] == L'/' 
			|| defaultFileName.data()[defaultFileName.length() - 1] == L'\\') {
			// The value is only a directory.
			UTF8ToUTF16(defaultFileName.toStdString(), &directory);
		}
		else {
			// The value is a file name and possibly a directory.
			std::wstring name;
			UTF8ToUTF16(defaultFileName.toStdString(), &name);
			wcscpy_s(filename, MAX_PATH, name.c_str());
			size_t pos = name.rfind(L'/');
			if (pos == std::wstring::npos)
				pos = name.rfind(L'\\');
			if (pos != std::wstring::npos)
				directory = name.substr(0, pos);
		}
	}
	if (!directory.empty())
		ofn.lpstrInitialDir = directory.c_str();

	std::wstring title;
	if (!params.title().isEmpty())
		UTF8ToUTF16(params.title().toStdString(), &title);
	else
		title = L"打开文件";
	if (!title.empty())
		ofn.lpstrTitle = title.c_str();

	// We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
	// without having to close Chrome first.
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER |
		OFN_ENABLESIZING;
	
	int numAcceptTypes = params.numAcceptTypes();
	std::vector<std::wstring> filters;
	for (int i = 0; i < numAcceptTypes; i++) {
		minikit::StringRef f = params.acceptTypeAt(i);
		std::wstring wf;
		UTF8ToUTF16(f.toStdString(), &wf);
		filters.push_back(wf);
	}

	const std::wstring& filter = GetFilterString(filters);
	if (!filter.empty()) {
		ofn.lpstrFilter = filter.c_str();
		// Indices into |lpstrFilter| start at 1.
		ofn.nFilterIndex = *filter_index + 1;
	}

	bool success = !!GetOpenFileName(&ofn);
	if (success) {
		*filter_index = ofn.nFilterIndex == 0 ? 0 : ofn.nFilterIndex - 1;
		*path = filename;
	}
	return success;
}

bool RunOpenMultiFileDialog(
	const minikit::FileChooserParams&  params,
	HWND owner,
	int* filter_index,
	std::vector<std::wstring>* paths) {
	OPENFILENAME ofn;

	// We must do this otherwise the ofn's FlagsEx may be initialized to random
	// junk in release builds which can cause the Places Bar not to show up!
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner;

	std::unique_ptr<wchar_t[]> filename(new wchar_t[UNICODE_STRING_MAX_CHARS]);
	filename[0] = 0;

	ofn.lpstrFile = filename.get();
	ofn.nMaxFile = UNICODE_STRING_MAX_CHARS;

	std::wstring directory;
	if (!params.defaultFileName().isEmpty()) {
		minikit::StringRef defaultFileName = params.defaultFileName();
		if (defaultFileName.data()[defaultFileName.length() - 1] == L'/'
			|| defaultFileName.data()[defaultFileName.length() - 1] == L'\\') {
			// The value is only a directory.
			UTF8ToUTF16(defaultFileName.toStdString(), &directory);
		}
		else {
			// The value is a file name and possibly a directory.
			std::wstring name;
			UTF8ToUTF16(defaultFileName.toStdString(), &name);
			wcscpy_s(filename.get(), MAX_PATH, name.c_str());
			size_t pos = name.rfind(L'/');
			if (pos == std::wstring::npos)
				pos = name.rfind(L'\\');
			if (pos != std::wstring::npos)
				directory = name.substr(0, pos);
		}
	}
	if (!directory.empty())
		ofn.lpstrInitialDir = directory.c_str();

	std::wstring title;
	if (!params.title().isEmpty())
		UTF8ToUTF16(params.title().toStdString(), &title);
	else
		title = L"打开文件";
	if (!title.empty())
		ofn.lpstrTitle = title.c_str();

	// We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
	// without having to close Chrome first.
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER |
		OFN_ALLOWMULTISELECT | OFN_ENABLESIZING;
	

	int numAcceptTypes = params.numAcceptTypes();
	std::vector<std::wstring> filters;
	for (int i = 0; i < numAcceptTypes; i++) {
		minikit::StringRef f = params.acceptTypeAt(i);
		std::wstring wf;
		UTF8ToUTF16(f.toStdString(), &wf);
		filters.push_back(wf);
	}

	const std::wstring& filter = GetFilterString(filters);
	if (!filter.empty()) {
		ofn.lpstrFilter = filter.c_str();
		// Indices into |lpstrFilter| start at 1.
		ofn.nFilterIndex = *filter_index + 1;
	}

	bool success = !!GetOpenFileName(&ofn);

	if (success) {
		std::vector<std::wstring> files;
		const wchar_t* selection = ofn.lpstrFile;
		while (*selection) {  // Empty string indicates end of list.
			files.push_back(selection);
			// Skip over filename and null-terminator.
			selection += files.back().length() + 1;
		}
		if (files.empty()) {
			success = false;
		}
		else if (files.size() == 1) {
			// When there is one file, it contains the path and filename.
			paths->swap(files);
		}
		else {
			// Otherwise, the first string is the path, and the remainder are
			// filenames.
			std::vector<std::wstring>::iterator path = files.begin();
			for (std::vector<std::wstring>::iterator file = path + 1;
				file != files.end(); ++file) {
				paths->push_back(*path + L"\\" + *file);
			}
		}
	}

	if (success)
		*filter_index = ofn.nFilterIndex == 0 ? 0 : ofn.nFilterIndex - 1;

	return success;
}

// The callback function for when the select folder dialog is opened.
int CALLBACK BrowseCallbackProc(HWND window,
UINT message,
LPARAM parameter,
LPARAM data)
{
	if (message == BFFM_INITIALIZED) {
		// WParam is TRUE since passing a path.
		// data lParam member of the BROWSEINFO structure.
		SendMessage(window, BFFM_SETSELECTION, TRUE, (LPARAM)data);
	}
	return 0;
}

bool RunOpenFolderDialog(
	const minikit::FileChooserParams& params,
	HWND owner,
	std::wstring* path) {
	wchar_t dir_buffer[MAX_PATH + 1] = { 0 };

	bool result = false;
	BROWSEINFO browse_info = { 0 };
	browse_info.hwndOwner = owner;
	browse_info.pszDisplayName = dir_buffer;
	browse_info.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;

	std::wstring title;
	if (!params.title().isEmpty())
		UTF8ToUTF16(params.title().toStdString(), &title);
	else
		title = L"打开文件夹";
	if (!title.empty())
		browse_info.lpszTitle = title.c_str();

	std::wstring file_path;
	UTF8ToUTF16(params.defaultFileName().toStdString(), &file_path);

	if (!file_path.empty()) {
		// Highlight the current value.
		browse_info.lParam = (LPARAM)file_path.c_str();
		browse_info.lpfn = &BrowseCallbackProc;
	}

	LPITEMIDLIST list = SHBrowseForFolder(&browse_info);
	if (list) {
		STRRET out_dir_buffer;
		ZeroMemory(&out_dir_buffer, sizeof(out_dir_buffer));
		out_dir_buffer.uType = STRRET_WSTR;
		IShellFolder* shell_folder = NULL;
		if (SHGetDesktopFolder(&shell_folder) == NOERROR) {
			HRESULT hr = shell_folder->GetDisplayNameOf(list, SHGDN_FORPARSING,
				&out_dir_buffer);
			if (SUCCEEDED(hr) && out_dir_buffer.uType == STRRET_WSTR) {
				*path = out_dir_buffer.pOleStr;
				CoTaskMemFree(out_dir_buffer.pOleStr);
				result = true;
			}
			else {
				// Use old way if we don't get what we want.
				wchar_t old_out_dir_buffer[MAX_PATH + 1];
				if (SHGetPathFromIDList(list, old_out_dir_buffer)) {
					*path = old_out_dir_buffer;
					result = true;
				}
			}
			shell_folder->Release();
		}
		CoTaskMemFree(list);
	}

	return result;
}

bool RunSaveFileDialog(
	const minikit::FileChooserParams& params,
	HWND owner,
	int* filter_index,
	std::wstring* path) {
	OPENFILENAME ofn;

	// We must do this otherwise the ofn's FlagsEx may be initialized to random
	// junk in release builds which can cause the Places Bar not to show up!
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner;

	wchar_t filename[MAX_PATH] = { 0 };

	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;

	std::wstring directory;

	if (!params.defaultFileName().isEmpty()) {
		minikit::StringRef defaultFileName = params.defaultFileName();
		if (defaultFileName.data()[defaultFileName.length() - 1] == L'/'
			|| defaultFileName.data()[defaultFileName.length() - 1] == L'\\') {
			// The value is only a directory.
			UTF8ToUTF16(defaultFileName.toStdString(), &directory);
		}
		else {
			// The value is a file name and possibly a directory.
			std::wstring name;
			UTF8ToUTF16(defaultFileName.toStdString(), &name);
			wcscpy_s(filename, MAX_PATH, name.c_str());
			size_t pos = name.rfind(L'/');
			if (pos == std::wstring::npos)
				pos = name.rfind(L'\\');
			if (pos != std::wstring::npos)
				directory = name.substr(0, pos);
		}
	}
	if (!directory.empty())
		ofn.lpstrInitialDir = directory.c_str();

	std::wstring title;
	if (!params.title().isEmpty())
		UTF8ToUTF16(params.title().toStdString(), &title);
	else
		title = L"保存文件";
		
	if (!title.empty())
		ofn.lpstrTitle = title.c_str();

	// We use OFN_NOCHANGEDIR so that the user can rename or delete the directory
	// without having to close Chrome first.
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_NOCHANGEDIR |
		OFN_PATHMUSTEXIST;
	
	int numAcceptTypes = params.numAcceptTypes();
	std::vector<std::wstring> filters;
	for (int i = 0; i < numAcceptTypes; i++) {
		minikit::StringRef f = params.acceptTypeAt(i);
		std::wstring wf;
		UTF8ToUTF16(f.toStdString(), &wf);
		filters.push_back(wf);
	}

	const std::wstring& filter = GetFilterString(filters);
	if (!filter.empty()) {
		ofn.lpstrFilter = filter.c_str();
		// Indices into |lpstrFilter| start at 1.
		ofn.nFilterIndex = *filter_index + 1;
	}

	bool success = !!GetSaveFileName(&ofn);
	if (success) {
		*filter_index = ofn.nFilterIndex == 0 ? 0 : ofn.nFilterIndex - 1;
		*path = std::wstring(filename);
	}
	return success;
}

void FileDialogRunner::Run(minikit::WebView* source, const minikit::FileChooserParams& params)
{
	int filter_index = 0;
	std::vector<std::wstring> files;

	HWND owner = source->getView();
	if (params.mode() == minikit::FileChooserParams::OPEN) {
		std::wstring file;
		if (RunOpenFileDialog(params, owner, &filter_index, &file)){
			files.push_back(file);
		}
	}
	else if (params.mode() == minikit::FileChooserParams::OPEN_MULTIPLE) {
		RunOpenMultiFileDialog(params, owner, &filter_index, &files);
	}
	else if (params.mode() == minikit::FileChooserParams::UPLOAD_FOLDER) {
		std::wstring file;
		if (RunOpenFolderDialog(params, owner, &file))
			files.push_back(file);
	}
	else if (params.mode() == minikit::FileChooserParams::SAVE) {
		std::wstring file;
		if (RunSaveFileDialog(params, owner, &filter_index, &file))
			files.push_back(file);
	}

	std::vector<std::string> utf8files;
	std::vector<minikit::StringRef> ret;
	for (std::vector<std::wstring>::const_iterator it = files.begin(); it != files.end(); ++it) {
		std::string item;
		UTF16ToUTF8(*it, &item);
		utf8files.push_back(item);
		ret.push_back(utf8files.back());
	}
	source->fileChooserCompleted(ret.empty() ? NULL : &ret[0], ret.size());
}
