#pragma once

namespace coverpp
{
template<typename F>
struct Guard
{
	F f;
	~Guard()
	{
		f();
	}
};
} // namespace coverpp
