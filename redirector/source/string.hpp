#ifndef REDIRECTOR_STRING_HPP_
#define REDIRECTOR_STRING_HPP_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline std::string UnicodeToUtf8(_In_ std::wstring const& source) 
{
  if (source.empty()) return {};

  auto size   = WideCharToMultiByte(CP_UTF8, 0, &source[0], (int)source.size(), nullptr, 0, nullptr, nullptr);
  auto result = std::string(size, 0);

  WideCharToMultiByte(CP_UTF8, 0, &source[0], (int)source.size(), &result[0], size, nullptr, nullptr);
  return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline std::wstring Utf8ToUnicode(_In_ std::string const& source)
{
  if (source.empty()) return {};

  auto size   = MultiByteToWideChar(CP_UTF8, 0, &source[0], (int)source.size(), nullptr, 0);
  auto result = std::wstring(size, 0);

  MultiByteToWideChar(CP_UTF8, 0, &source[0], (int)source.size(), &result[0], size);
  return result;
}

#endif // !REDIRECTOR_STRING_HPP_

