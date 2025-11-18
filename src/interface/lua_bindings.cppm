// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.modules.chromium:lua_bindings;

export import pragma.lua;

export namespace Lua {
	namespace chromium {
		void register_library(Lua::Interface &l);
	}
};
