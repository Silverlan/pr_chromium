// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.modules.chromium;

import :lua_bindings;
import pragma.shared;

static void register_wiweb_class(Lua::Interface &l)
{
	auto &modGUI = l.RegisterLibrary("gui");
	auto classDefWeb = luabind::class_<::WIWeb, luabind::bases<WITexturedShape, WIShape, ::WIBase>>("Web");
	classDefWeb.def("LoadUrl", &WIWeb::LoadURL);
	classDefWeb.def("GetUrl", &WIWeb::GetUrl);
	classDefWeb.def("SetBrowserViewSize", &WIWeb::SetBrowserViewSize);
	classDefWeb.def("GetBrowserViewSize", &WIWeb::GetBrowserViewSize, luabind::copy_policy<0> {});

	classDefWeb.def("Close", &WIWeb::Close);
	classDefWeb.def("CanGoBack", &WIWeb::CanGoBack);
	classDefWeb.def("CanGoForward", &WIWeb::CanGoForward);
	classDefWeb.def("GoBack", &WIWeb::GoBack);
	classDefWeb.def("GoForward", &WIWeb::GoForward);
	classDefWeb.def("HasDocument", &WIWeb::HasDocument);
	classDefWeb.def("IsLoading", &WIWeb::IsLoading);
	classDefWeb.def("Reload", &WIWeb::Reload);
	classDefWeb.def("ReloadIgnoreCache", &WIWeb::ReloadIgnoreCache);
	classDefWeb.def("StopLoad", &WIWeb::StopLoad);
	classDefWeb.def("Copy", &WIWeb::Copy);
	classDefWeb.def("Cut", &WIWeb::Cut);
	classDefWeb.def("Delete", &WIWeb::Delete);
	classDefWeb.def("Paste", &WIWeb::Paste);
	classDefWeb.def("Redo", &WIWeb::Redo);
	classDefWeb.def("SelectAll", &WIWeb::SelectAll);
	classDefWeb.def("Undo", &WIWeb::Undo);
	classDefWeb.def("SetInitialUrl", &WIWeb::SetInitialUrl);
	classDefWeb.def("ExecuteJavaScript", &WIWeb::ExecuteJavaScript);

	classDefWeb.def("SetZoomLevel", &WIWeb::SetZoomLevel);
	classDefWeb.def("GetZoomLevel", &WIWeb::GetZoomLevel);
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
	  {{"DOWNLOAD_STATE_DOWNLOADING", umath::to_integral(cef::IChromiumWrapper::DownloadState::Downloading)}, {"DOWNLOAD_STATE_CANCELLED", umath::to_integral(cef::IChromiumWrapper::DownloadState::Cancelled)},
	    {"DOWNLOAD_STATE_COMPLETE", umath::to_integral(cef::IChromiumWrapper::DownloadState::Complete)}, {"DOWNLOAD_STATE_INVALIDATED", umath::to_integral(cef::IChromiumWrapper::DownloadState::Invalidated)}});
}

void Lua::chromium::register_library(Lua::Interface &l) { register_wiweb_class(l); }
