#include "lchromium.hpp"
#include "wiweb.hpp"
#include "wvmodule.hpp"
#include "wiweb.hpp"
#include <pragma/lua/luaapi.h>
#include <luabind/copy_policy.hpp>
#include <luasystem.h>
#include <luainterface.hpp>

static void register_wiweb_class(Lua::Interface &l)
{
	auto &modGUI = l.RegisterLibrary("gui");
	auto classDefWeb = luabind::class_<::WIWeb,luabind::bases<WITexturedShape,WIShape,::WIBase>>("Web");
	classDefWeb.def("LoadUrl",&WIWeb::LoadURL);
	classDefWeb.def("SetBrowserViewSize",&WIWeb::SetBrowserViewSize);
	classDefWeb.def("GetBrowserViewSize",&WIWeb::GetBrowserViewSize,luabind::copy_policy<0>{});
	
	classDefWeb.def("Close",&WIWeb::Close);
	classDefWeb.def("CanGoBack",&WIWeb::CanGoBack);
	classDefWeb.def("CanGoForward",&WIWeb::CanGoForward);
	classDefWeb.def("GoBack",&WIWeb::GoBack);
	classDefWeb.def("GoForward",&WIWeb::GoForward);
	classDefWeb.def("HasDocument",&WIWeb::HasDocument);
	classDefWeb.def("IsLoading",&WIWeb::IsLoading);
	classDefWeb.def("Reload",&WIWeb::Reload);
	classDefWeb.def("ReloadIgnoreCache",&WIWeb::ReloadIgnoreCache);
	classDefWeb.def("StopLoad",&WIWeb::StopLoad);
	classDefWeb.def("Copy",&WIWeb::Copy);
	classDefWeb.def("Cut",&WIWeb::Cut);
	classDefWeb.def("Delete",&WIWeb::Delete);
	classDefWeb.def("Paste",&WIWeb::Paste);
	classDefWeb.def("Redo",&WIWeb::Redo);
	classDefWeb.def("SelectAll",&WIWeb::SelectAll);
	classDefWeb.def("Undo",&WIWeb::Undo);
	classDefWeb.def("SetInitialUrl",&WIWeb::SetInitialUrl);

	classDefWeb.def("SetZoomLevel",&WIWeb::SetZoomLevel);
	classDefWeb.def("GetZoomLevel",&WIWeb::GetZoomLevel);
	modGUI[classDefWeb];

	Lua::RegisterLibrary(l.GetState(),"chromium",{});
	Lua::RegisterLibraryEnums(l.GetState(),"chromium",{
		{"DOWNLOAD_STATE_DOWNLOADING",umath::to_integral(cef::IChromiumWrapper::DownloadState::Downloading)},
		{"DOWNLOAD_STATE_CANCELLED",umath::to_integral(cef::IChromiumWrapper::DownloadState::Cancelled)},
		{"DOWNLOAD_STATE_COMPLETE",umath::to_integral(cef::IChromiumWrapper::DownloadState::Complete)},
		{"DOWNLOAD_STATE_INVALIDATED",umath::to_integral(cef::IChromiumWrapper::DownloadState::Invalidated)}
	});
}

void Lua::chromium::register_library(Lua::Interface &l)
{
	register_wiweb_class(l);
}
