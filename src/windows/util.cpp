#include "util.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wil/result_macros.h>

#include <format>
#include <stdexcept>

namespace coverpp::windows::detail
{
template<typename TChar>
    requires std::same_as<TChar, char> || std::same_as<TChar, wchar_t>
std::basic_string<TChar> read_remote_c_string(HANDLE process, std::uintptr_t address, std::size_t max_length)
{
	if (!address)
	{
		throw std::invalid_argument("Attempting to read from null address");
	}
	if (max_length == 0)
	{
		throw std::invalid_argument("max_length must be > 0");
	}

	static constexpr std::size_t max_chunk_size_bytes = 256;
	static constexpr std::size_t max_chunk_size_chars = max_chunk_size_bytes / sizeof(TChar);

	auto const ideal_chunk_size_chars = std::min(max_chunk_size_chars, max_length);

	std::basic_string<TChar> result;
	while (true)
	{
		TChar       chunk[max_chunk_size_chars];
		std::size_t chunk_size_chars{ideal_chunk_size_chars};
		std::size_t num_read_bytes{};
		while (!ReadProcessMemory(
		    process, reinterpret_cast<LPCVOID>(address), chunk, chunk_size_chars * sizeof(TChar), &num_read_bytes))
		{
			// Chunk size too big?
			chunk_size_chars /= 2;
			if (chunk_size_chars == 0)
			{
				THROW_LAST_ERROR();
			}
		}
		if (num_read_bytes < sizeof(TChar) || num_read_bytes % sizeof(TChar) != 0)
		{
			// This should never happen
			throw std::runtime_error("ReadProcessMemory read incorrect number of bytes but was successful");
		}

		// We have successfully read a chunk
		// Note that it might contain the nullterm followed by garbage
		auto chunk_view = std::basic_string_view<TChar>{chunk, num_read_bytes / sizeof(TChar)};

		if (auto const nullterm = chunk_view.find(TChar{}); nullterm != std::basic_string_view<TChar>::npos)
		{
			result.append(chunk_view.substr(0, nullterm));

			if (result.size() > max_length)
			{
				throw std::runtime_error(std::format("Remote string is too long at over {} chars", max_length));
			}

			return result;
		}

		result.append(chunk_view);

		if (result.size() > max_length)
		{
			throw std::runtime_error(std::format("Remote string is too long at over {} chars", max_length));
		}

		if (num_read_bytes > std::numeric_limits<std::uintptr_t>::max() - address)
		{
			throw std::runtime_error("Remote address overflow");
		}
		address += num_read_bytes;
	}
}

template<typename TChar>
    requires std::same_as<TChar, char> || std::same_as<TChar, wchar_t>
std::basic_string<TChar> read_remote_c_string(HANDLE process, TChar const* address, std::size_t max_length)
{
	return read_remote_c_string<TChar>(process, reinterpret_cast<std::uintptr_t>(address), max_length);
}

std::wstring read_remote_c_wstring(void* process, std::uintptr_t address, std::size_t max_length)
{
	return read_remote_c_string<wchar_t>(process, address, max_length);
}

std::wstring read_remote_c_wstring(void* process, wchar_t const* address, std::size_t max_length)
{
	return read_remote_c_string<wchar_t>(process, address, max_length);
}

// Instantiations
template std::string  read_remote_c_string<char>(void* process, std::uintptr_t address, std::size_t max_length);
template std::wstring read_remote_c_string<wchar_t>(void* process, std::uintptr_t address, std::size_t max_length);
template std::string  read_remote_c_string<char>(void* process, char const* address, std::size_t max_length);
template std::wstring read_remote_c_string<wchar_t>(void* process, wchar_t const* address, std::size_t max_length);

} // namespace coverpp::windows::detail
