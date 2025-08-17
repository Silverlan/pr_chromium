// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __CHROMIUM_WRAPPER_HPP__
#define __CHROMIUM_WRAPPER_HPP__

#include "util_javascript.hpp"
#include <sharedutils/util_library.hpp>

namespace cef {
	enum class Modifier : uint32_t {
		None = 0,
		CapsLockOn = 1,
		ShiftDown = CapsLockOn << 1u,
		ControlDown = CapsLockOn << 2u,
		AltDown = CapsLockOn << 3u,
		LeftMouseButton = CapsLockOn << 4u,
		MiddleMouseButton = CapsLockOn << 5u,
		RightMouseButton = CapsLockOn << 6u,
		CommandDown = CapsLockOn << 7u,
		NumLockOn = CapsLockOn << 8u,
		IsKeyPad = CapsLockOn << 9u,
		IsLeft = CapsLockOn << 10u,
		IsRight = CapsLockOn << 11u,
		AltGrDown = CapsLockOn << 12u,
		IsRepeat = CapsLockOn << 13u
	};
	using CWebRenderHandler = void;
	using CWebBrowserClient = void;
	using CWebBrowser = void;
	struct IChromiumWrapper final {
		enum class DownloadState : uint32_t { Downloading = 0, Cancelled, Complete, Invalidated };

		IChromiumWrapper(util::Library &lib);
		IChromiumWrapper() = default;
		void (*register_javascript_function)(const std::string &name, cef::JSValue *(*const fCallback)(cef::JSValue *, uint32_t)) = nullptr;
		bool (*initialize)(const char *, const char *, bool, std::string &) = nullptr;
		void (*close)() = nullptr;
		void (*do_message_loop_work)() = nullptr;
		bool (*parse_url)(const char *url, void (*r)(void *, const char *, const char *, const char *, const char *, const char *, const char *, const char *, const char *, const char *, const char *), void *);
		CWebRenderHandler *(*render_handler_create)(void (*fGetRootScreenRect)(CWebRenderHandler *, int &, int &, int &, int &), void (*fGetViewRect)(CWebRenderHandler *, int &, int &, int &, int &), void (*fGetScreenPoint)(CWebRenderHandler *, int, int, int &, int &)) = nullptr;
		void (*render_handler_release)(CWebRenderHandler *renderHandler) = nullptr;
		void (*render_handler_set_user_data)(CWebRenderHandler *renderHandler, void *userData) = nullptr;
		void *(*render_handler_get_user_data)(CWebRenderHandler *renderHandler) = nullptr;
		CWebBrowserClient *(*browser_client_create)(CWebRenderHandler *renderHandler) = nullptr;
		void (*browser_client_release)(CWebBrowserClient *browserClient) = nullptr;
		void (*browser_client_set_user_data)(cef::CWebBrowserClient *browserClient, void *userData) = nullptr;
		void *(*browser_client_get_user_data)(cef::CWebBrowserClient *browserClient) = nullptr;

		void (*browser_client_set_download_start_callback)(cef::CWebBrowserClient *browserClient, void (*onStart)(cef::CWebBrowserClient *, uint32_t, const char *)) = nullptr;
		void (*browser_client_set_download_update_callback)(cef::CWebBrowserClient *browserClient, void (*onUpdate)(cef::CWebBrowserClient *, uint32_t, DownloadState, int32_t)) = nullptr;

		void (*browser_client_set_download_location)(cef::CWebBrowserClient *browserClient, const char *location);
		void (*browser_client_set_on_address_change_callback)(cef::CWebBrowserClient *browserClient, void (*onAddressChange)(cef::CWebBrowserClient *, const char *));
		void (*browser_client_set_on_loading_state_change)(cef::CWebBrowserClient *browserClient, void (*onLoadingStateChange)(cef::CWebBrowserClient *, bool, bool, bool));
		void (*browser_client_set_on_load_start)(cef::CWebBrowserClient *browserClient, void (*onLoadStart)(cef::CWebBrowserClient *, int));
		void (*browser_client_set_on_load_end)(cef::CWebBrowserClient *browserClient, void (*onLoadEnd)(cef::CWebBrowserClient *, int));
		void (*browser_client_set_on_load_error)(cef::CWebBrowserClient *browserClient, void (*onLoadError)(cef::CWebBrowserClient *, int, const char *, const char *));

		CWebBrowser *(*browser_create)(CWebBrowserClient *browserClient, const char *initialUrl) = nullptr;
		void (*browser_release)(CWebBrowser *browser) = nullptr;
		void (*browser_close)(cef::CWebBrowser *browser) = nullptr;
		bool (*browser_try_close)(cef::CWebBrowser *browser) = nullptr;
		void *(*browser_get_user_data)(cef::CWebBrowser *browser) = nullptr;
		void (*browser_was_resized)(cef::CWebBrowser *browser) = nullptr;
		void (*browser_invalidate)(cef::CWebBrowser *browser) = nullptr;

		void (*render_handler_set_image_data)(cef::CWebRenderHandler *renderHandler, void *ptr, uint32_t w, uint32_t h) = nullptr;
		void (*render_handler_get_dirty_rects)(cef::CWebRenderHandler *renderHandler, const std::tuple<int, int, int, int> **, uint32_t &) = nullptr;
		void (*render_handler_clear_dirty_rects)(cef::CWebRenderHandler *renderHandler) = nullptr;
		bool (*render_handler_is_renderer_size_mismatched)(cef::CWebRenderHandler *renderHandler) = nullptr;
		// Browser
		void (*browser_load_url)(CWebBrowser *browser, const char *url) = nullptr;
		bool (*browser_can_go_back)(CWebBrowser *browser) = nullptr;
		bool (*browser_can_go_forward)(CWebBrowser *browser) = nullptr;
		void (*browser_go_back)(CWebBrowser *browser) = nullptr;
		void (*browser_go_forward)(CWebBrowser *browser) = nullptr;
		bool (*browser_has_document)(CWebBrowser *browser) = nullptr;
		bool (*browser_is_loading)(CWebBrowser *browser) = nullptr;
		void (*browser_reload)(CWebBrowser *browser) = nullptr;
		void (*browser_reload_ignore_cache)(CWebBrowser *browser) = nullptr;
		void (*browser_stop_load)(CWebBrowser *browser) = nullptr;
		void (*browser_copy)(CWebBrowser *browser) = nullptr;
		void (*browser_cut)(CWebBrowser *browser) = nullptr;
		void (*browser_delete)(CWebBrowser *browser) = nullptr;
		void (*browser_paste)(CWebBrowser *browser) = nullptr;
		void (*browser_redo)(CWebBrowser *browser) = nullptr;
		void (*browser_select_all)(CWebBrowser *browser) = nullptr;
		void (*browser_undo)(CWebBrowser *browser) = nullptr;
		void (*browser_set_zoom_level)(CWebBrowser *browser, double zoomLevel) = nullptr;
		double (*browser_get_zoom_level)(CWebBrowser *browser) = nullptr;
		void (*browser_send_event_mouse_move)(CWebBrowser *browser, int x, int y, bool mouseLeave, cef::Modifier mods) = nullptr;
		void (*browser_send_event_mouse_click)(CWebBrowser *browser, int x, int y, char btType, bool mouseUp, int clickCount) = nullptr;
		void (*browser_send_event_key)(CWebBrowser *browser, char c, int systemKey, int nativeKeyCode, bool pressed, cef::Modifier mods) = nullptr;
		void (*browser_send_event_char)(CWebBrowser *browser, char c, cef::Modifier mods) = nullptr;
		void (*browser_send_event_mouse_wheel)(CWebBrowser *browser, int x, int y, float deltaX, float deltaY) = nullptr;
		void (*browser_set_focus)(CWebBrowser *browser, bool focus) = nullptr;
		void (*browser_execute_java_script)(cef::CWebBrowser *browser, const char *js, const char *url) = nullptr;

		bool valid() const { return m_bValid; }
	  private:
		bool m_bValid = false;
	};
	IChromiumWrapper &get_wrapper();
};
REGISTER_BASIC_BITWISE_OPERATORS(cef::Modifier)

#endif
