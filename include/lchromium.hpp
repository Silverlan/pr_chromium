// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LCHROMIUM_HPP__
#define __LCHROMIUM_HPP__

namespace Lua {
	class Interface;
	namespace chromium {
		void register_library(Lua::Interface &l);
	}
};

#endif
