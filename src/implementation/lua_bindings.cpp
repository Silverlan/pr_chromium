// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.modules.chromium;

import :lua_bindings;
import pragma.shared;

static void register_wiweb_class(Lua::Interface &l)
{
	auto &modGUI = l.RegisterLibrary("gui");
	auto classDefWeb = luabind::class_<pragma::gui::types::WIWeb, luabind::bases<pragma::gui::types::WITexturedShape, pragma::gui::types::WIShape, pragma::gui::types::WIBase>>("Web");
	classDefWeb.def("LoadUrl", &pragma::gui::types::WIWeb::LoadURL);
	classDefWeb.def("GetUrl", &pragma::gui::types::WIWeb::GetUrl);
	classDefWeb.def("SetBrowserViewSize", &pragma::gui::types::WIWeb::SetBrowserViewSize);
	classDefWeb.def("GetBrowserViewSize", &pragma::gui::types::WIWeb::GetBrowserViewSize, luabind::copy_policy<0> {});

	classDefWeb.def("Close", &pragma::gui::types::WIWeb::Close);
	classDefWeb.def("CanGoBack", &pragma::gui::types::WIWeb::CanGoBack);
	classDefWeb.def("CanGoForward", &pragma::gui::types::WIWeb::CanGoForward);
	classDefWeb.def("GoBack", &pragma::gui::types::WIWeb::GoBack);
	classDefWeb.def("GoForward", &pragma::gui::types::WIWeb::GoForward);
	classDefWeb.def("HasDocument", &pragma::gui::types::WIWeb::HasDocument);
	classDefWeb.def("IsLoading", &pragma::gui::types::WIWeb::IsLoading);
	classDefWeb.def("Reload", &pragma::gui::types::WIWeb::Reload);
	classDefWeb.def("ReloadIgnoreCache", &pragma::gui::types::WIWeb::ReloadIgnoreCache);
	classDefWeb.def("StopLoad", &pragma::gui::types::WIWeb::StopLoad);
	classDefWeb.def("Copy", &pragma::gui::types::WIWeb::Copy);
	classDefWeb.def("Cut", &pragma::gui::types::WIWeb::Cut);
	classDefWeb.def("Delete", &pragma::gui::types::WIWeb::Delete);
	classDefWeb.def("Paste", &pragma::gui::types::WIWeb::Paste);
	classDefWeb.def("Redo", &pragma::gui::types::WIWeb::Redo);
	classDefWeb.def("SelectAll", &pragma::gui::types::WIWeb::SelectAll);
	classDefWeb.def("Undo", &pragma::gui::types::WIWeb::Undo);
	classDefWeb.def("SetInitialUrl", &pragma::gui::types::WIWeb::SetInitialUrl);
	classDefWeb.def("ExecuteJavaScript", &pragma::gui::types::WIWeb::ExecuteJavaScript);

	classDefWeb.def("SetZoomLevel", &pragma::gui::types::WIWeb::SetZoomLevel);
	classDefWeb.def("GetZoomLevel", &pragma::gui::types::WIWeb::GetZoomLevel);
	modGUI[classDefWeb];

	auto &modChromium = l.RegisterLibrary("chromium");
	modChromium[luabind::def(
	  "parse_url", +[](lua::State *l, const std::string &url) -> std::optional<luabind::map<std::string, std::string>> {
		  luabind::object t = luabind::newtable(l);
		  auto success = cef::get_wrapper().parse_url(
		    url.c_str(),
		    [](void *userData, const char *host, const char *fragment, const char *password, const char *origin, const char *path, const char *port, const char *query, const char *scheme, const char *spec, const char *username) {
			    auto &t = *static_cast<luabind::object *>(userData);
			    t["host"] = host;
			    t["fragment"] = fragment;
			    t["password"] = password;
			    t["origin"] = origin;
			    t["path"] = path;
			    t["port"] = port;
			    t["query"] = query;
			    t["scheme"] = scheme;
			    t["spec"] = spec;
			    t["username"] = username;
		    },
		    &t);
		  if(!success)
			  return Lua::nil;
		  return t;
	  })];
	Lua::RegisterLibraryEnums(l.GetState(), "chromium",
	  {{"DOWNLOAD_STATE_DOWNLOADING", pragma::math::to_integral(cef::IChromiumWrapper::DownloadState::Downloading)}, {"DOWNLOAD_STATE_CANCELLED", pragma::math::to_integral(cef::IChromiumWrapper::DownloadState::Cancelled)},
	    {"DOWNLOAD_STATE_COMPLETE", pragma::math::to_integral(cef::IChromiumWrapper::DownloadState::Complete)}, {"DOWNLOAD_STATE_INVALIDATED", pragma::math::to_integral(cef::IChromiumWrapper::DownloadState::Invalidated)}});
}

void Lua::chromium::register_library(Lua::Interface &l) { register_wiweb_class(l); }
