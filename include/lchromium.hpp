#ifndef __LCHROMIUM_HPP__
#define __LCHROMIUM_HPP__

namespace Lua {
	class Interface;
	namespace chromium {
		void register_library(Lua::Interface &l);
	}
};

#endif
