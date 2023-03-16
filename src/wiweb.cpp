#include <prosper_context.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <buffers/prosper_buffer.hpp>
#include <pragma/lua/libraries/c_gui_callbacks.hpp>
#include "wiweb.hpp"
#include <prosper_window.hpp>
#include <fsys/filesystem.h>
#include <memory>

#if __linux__
#include <linux/input-event-codes.h> //for key defines
#endif

LINK_WGUI_TO_CLASS(WIWeb, WIWeb);
#pragma optimize("", off)
void WIWeb::register_callbacks()
{
	Lua::gui::register_lua_callback("wiweb", "OnDownloadStarted", [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle {
		return FunctionCallback<void, uint32_t, util::Path>::Create([l, callLuaFunc](uint32_t id, util::Path path) {
			callLuaFunc([l, id, &path]() {
				Lua::Push(l, id);
				Lua::Push(l, path);
			});
		});
	});
	Lua::gui::register_lua_callback("wiweb", "OnDownloadUpdate", [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle {
		return FunctionCallback<void, uint32_t, cef::IChromiumWrapper::DownloadState, int>::Create([l, callLuaFunc](uint32_t id, cef::IChromiumWrapper::DownloadState state, int percentage) {
			callLuaFunc([l, id, state, percentage]() {
				Lua::Push(l, id);
				Lua::Push(l, state);
				Lua::Push(l, percentage);
			});
		});
	});
	Lua::gui::register_lua_callback("wiweb", "OnAddressChanged",
	  [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle { return FunctionCallback<void, std::string>::Create([l, callLuaFunc](std::string address) { callLuaFunc([l, &address]() { Lua::PushString(l, address); }); }); });
	Lua::gui::register_lua_callback("wiweb", "OnLoadEnd",
	  [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle { return FunctionCallback<void, int>::Create([l, callLuaFunc](int httpStatusCode) { callLuaFunc([l, httpStatusCode]() { Lua::PushInt(l, httpStatusCode); }); }); });
	Lua::gui::register_lua_callback("wiweb", "OnLoadError", [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle {
		return FunctionCallback<void, int, std::string, std::string>::Create([l, callLuaFunc](int errorCode, std::string errorText, std::string failedUrl) {
			callLuaFunc([l, errorCode, &errorText, &failedUrl]() {
				Lua::PushInt(l, errorCode);
				Lua::PushString(l, errorText);
				Lua::PushString(l, failedUrl);
			});
		});
	});
	Lua::gui::register_lua_callback("wiweb", "OnLoadStart",
	  [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle { return FunctionCallback<void, int>::Create([l, callLuaFunc](int transitionType) { callLuaFunc([l, transitionType]() { Lua::PushInt(l, transitionType); }); }); });
	Lua::gui::register_lua_callback("wiweb", "OnLoadingStateChange", [](WIBase &el, lua_State *l, const std::function<void(const std::function<void()> &)> &callLuaFunc) -> CallbackHandle {
		return FunctionCallback<void, bool, bool, bool>::Create([l, callLuaFunc](bool isLoading, bool canGoBack, bool canGoForward) {
			callLuaFunc([l, isLoading, canGoBack, canGoForward]() {
				Lua::PushBool(l, isLoading);
				Lua::PushBool(l, canGoBack);
				Lua::PushBool(l, canGoForward);
			});
		});
	});
}
WIWeb::WIWeb() : WITexturedRect()
{
	RegisterCallback<void, uint32_t, util::Path>("OnDownloadStarted");
	RegisterCallback<void, uint32_t, cef::IChromiumWrapper::DownloadState, int>("OnDownloadUpdate");
	RegisterCallback<void, std::string>("OnAddressChanged");
	RegisterCallback<void, int>("OnLoadEnd");
	RegisterCallback<void, int, std::string, std::string>("OnLoadError");
	RegisterCallback<void, int>("OnLoadStart");
	RegisterCallback<void, bool, bool, bool>("OnLoadingStateChange");
}

WIWeb::~WIWeb()
{
	m_browser = nullptr;
	m_browserClient = nullptr;
	m_webRenderer = nullptr;
	ClearTexture();
}

void WIWeb::Initialize()
{
	WITexturedRect::Initialize();
	SetKeyboardInputEnabled(true);
	SetMouseInputEnabled(true);
	SetMouseMovementCheckEnabled(true);
	SetScrollInputEnabled(true);
}

void WIWeb::Think()
{
	WIBase::Think();
	if(m_browserClient == nullptr)
		return;
	cef::get_wrapper().do_message_loop_work();
}

void WIWeb::ClearTexture()
{
	if(m_imgDataPtr) {
		if(m_webRenderer)
			cef::get_wrapper().render_handler_set_image_data(m_webRenderer.get(), nullptr, 0, 0);
		m_texture->GetImage().Unmap();
		m_imgDataPtr = nullptr;
	}
	m_texture = nullptr;
}

void WIWeb::DoUpdate()
{
	WITexturedRect::DoUpdate();
	InitializeChromiumBrowser();
	Resize();
}

void WIWeb::SetTransparentBackground(bool b) { m_bTransparentBackground = b; }

void WIWeb::SetInitialUrl(std::string url) { m_initialUrl = std::move(url); }

void WIWeb::ExecuteJavaScript(const std::string &js)
{
	auto *browser = GetBrowser();
	if(browser)
		cef::get_wrapper().browser_execute_java_script(browser, js.c_str(), nullptr);
}

bool WIWeb::Resize()
{
	if(m_texture) {
		auto &img = m_texture->GetImage();
		if(img.GetWidth() == m_browserViewSize.x && img.GetHeight() == m_browserViewSize.y)
			return true;
	}
	ClearTexture();

	auto &context = WGUI::GetInstance().GetContext();
	if(m_browserViewSize.x == 0 || m_browserViewSize.y == 0)
		return false;
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = m_browserViewSize.x;
	imgCreateInfo.height = m_browserViewSize.y;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	imgCreateInfo.tiling = prosper::ImageTiling::Linear;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	auto img = context.CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.swizzleRed = prosper::ComponentSwizzle::B;
	imgViewCreateInfo.swizzleBlue = prosper::ComponentSwizzle::R;
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	m_texture = context.CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	if(m_texture == nullptr)
		return false;
	SetTexture(*m_texture);

	img->Map(0ull, img->GetSize(), &m_imgDataPtr);
	cef::get_wrapper().render_handler_set_image_data(m_webRenderer.get(), m_imgDataPtr, img->GetWidth(), img->GetHeight());
	cef::get_wrapper().browser_was_resized(GetBrowser());
	return true;
}

bool WIWeb::InitializeChromiumBrowser()
{
	if(m_browserInitialized)
		return true;
	m_browserInitialized = true;
	auto renderHandler = cef::get_wrapper().render_handler_create(
	  [](cef::CWebRenderHandler *renderHandler, int &x, int &y, int &w, int &h) {
		  auto &context = WGUI::GetInstance().GetContext();
		  auto &window = context.GetWindow();
		  auto windowPos = window->GetPos();
		  auto windowSize = window->GetSize();
		  x = windowPos.x;
		  y = windowPos.y;
		  w = windowSize.x;
		  h = windowSize.y;
	  },
	  [](cef::CWebRenderHandler *renderHandler, int &x, int &y, int &w, int &h) {
		  auto *el = static_cast<WIWeb *>(cef::get_wrapper().render_handler_get_user_data(renderHandler));
		  w = el->GetWidth();
		  h = el->GetHeight();
		  w = umath::max(w, 1);
		  h = umath::max(h, 1);

		  if(el->m_texture) {
			  auto extents = el->m_texture->GetImage().GetExtents();
			  w = extents.width;
			  h = extents.height;
		  }

		  auto &context = WGUI::GetInstance().GetContext();
		  auto &window = context.GetWindow();
		  auto windowPos = window->GetPos();
		  auto pos = el->GetAbsolutePos();
		  x = windowPos.x + pos.x;
		  y = windowPos.y + pos.y;
	  },
	  [](cef::CWebRenderHandler *renderHandler, int viewX, int viewY, int &screenX, int &screenY) {
		  auto *el = static_cast<WIWeb *>(cef::get_wrapper().render_handler_get_user_data(renderHandler));
		  auto &context = WGUI::GetInstance().GetContext();
		  auto &window = context.GetWindow();
		  auto windowPos = window->GetPos();
		  screenX = windowPos.x;
		  screenY = windowPos.y;

		  auto pos = el->GetAbsolutePos();
		  screenX += pos.x;
		  screenY += pos.y;

		  screenX += viewX;
		  screenY += viewY;
	  });
	if(!renderHandler)
		return false;
	m_webRenderer = std::shared_ptr<cef::CWebRenderHandler> {renderHandler, [](cef::CWebRenderHandler *renderHandler) { cef::get_wrapper().render_handler_release(renderHandler); }};
	cef::get_wrapper().render_handler_set_user_data(m_webRenderer.get(), this);

	//auto hWindow = GetActiveWindow();
	// TODO: Apply m_bTransparentBackground ?
	//window_info.windowless_rendering_enabled = true;
	//window_info.SetAsPopup(NULL, "cefsimple");

	//CefRefPtr<SimpleHandler> handler(new SimpleHandler(false));

	m_browserClient = std::shared_ptr<cef::CWebBrowserClient> {cef::get_wrapper().browser_client_create(m_webRenderer.get()), [](cef::CWebBrowserClient *browserClient) { cef::get_wrapper().browser_client_release(browserClient); }};
	cef::get_wrapper().browser_client_set_user_data(m_browserClient.get(), this);

	m_browser = std::shared_ptr<cef::CWebBrowser> {cef::get_wrapper().browser_create(m_browserClient.get(), m_initialUrl.c_str()), [](cef::CWebBrowser *browserClient) { cef::get_wrapper().browser_release(browserClient); }};

	auto dlPath = util::Path::CreatePath(util::get_program_path());
	dlPath += "cache/chromium/downloads/";
	filemanager::create_path(dlPath.GetString());
	cef::get_wrapper().browser_client_set_download_location(m_browserClient.get(), dlPath.GetString().c_str());
	cef::get_wrapper().browser_client_set_download_start_callback(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, uint32_t id, const char *fileName) {
		auto relPath = util::Path::CreateFile(fileName);
		if(!relPath.MakeRelative(util::Path::CreatePath(util::get_program_path())))
			return;
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->CallCallbacks<void, uint32_t, util::Path>("OnDownloadStarted", id, relPath);
	});
	cef::get_wrapper().browser_client_set_download_update_callback(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, uint32_t id, cef::IChromiumWrapper::DownloadState state, int percentageComplete) {
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->CallCallbacks<void, uint32_t, cef::IChromiumWrapper::DownloadState, int>("OnDownloadUpdate", id, state, percentageComplete);
	});
	cef::get_wrapper().browser_client_set_on_address_change_callback(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, const char *address) {
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->m_url = address;
		el->CallCallbacks<void, std::string>("OnAddressChanged", std::string {address});
	});
	cef::get_wrapper().browser_client_set_on_loading_state_change(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, bool isLoading, bool canGoBack, bool canGoForward) {
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->CallCallbacks<void, bool, bool, bool>("OnLoadingStateChange", isLoading, canGoBack, canGoForward);
	});
	cef::get_wrapper().browser_client_set_on_load_start(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, int transitionType) {
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->CallCallbacks<void, int>("OnLoadStart", transitionType);
	});
	cef::get_wrapper().browser_client_set_on_load_end(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, int httpStatusCode) {
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->CallCallbacks<void, int>("OnLoadEnd", httpStatusCode);
	});
	cef::get_wrapper().browser_client_set_on_load_error(m_browserClient.get(), [](cef::CWebBrowserClient *browserClient, int errorCode, const char *errorText, const char *failedUrl) {
		auto *el = static_cast<WIWeb *>(cef::get_wrapper().browser_client_get_user_data(browserClient));
		if(!el)
			return;
		el->CallCallbacks<void, int, std::string, std::string>("OnLoadError", errorCode, std::string {errorText}, std::string {failedUrl});
	});
	return true;
}

void WIWeb::Close()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_close(browser);
}

void WIWeb::LoadURL(const std::string &url)
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_load_url(browser, url.c_str());
}

void WIWeb::SetBrowserViewSize(Vector2i size)
{
	size.x = umath::max(size.x, 1);
	size.y = umath::max(size.y, 1);
	m_browserViewSize = size;
}
const Vector2i &WIWeb::GetBrowserViewSize() const { return m_browserViewSize; }

bool WIWeb::CanGoBack()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return cef::get_wrapper().browser_can_go_back(browser);
}
bool WIWeb::CanGoForward()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return cef::get_wrapper().browser_can_go_forward(browser);
}
void WIWeb::GoBack()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	return cef::get_wrapper().browser_go_back(browser);
}
void WIWeb::GoForward()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	return cef::get_wrapper().browser_go_forward(browser);
}
bool WIWeb::HasDocument()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return cef::get_wrapper().browser_has_document(browser);
}
bool WIWeb::IsLoading()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return false;
	return cef::get_wrapper().browser_is_loading(browser);
}
void WIWeb::Reload()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_reload(browser);
}
void WIWeb::ReloadIgnoreCache()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_reload_ignore_cache(browser);
}
void WIWeb::StopLoad()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_stop_load(browser);
}

void WIWeb::Copy()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_copy(browser);
}
void WIWeb::Cut()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_cut(browser);
}
void WIWeb::Delete()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_delete(browser);
}
void WIWeb::Paste()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_paste(browser);
}
void WIWeb::Redo()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_redo(browser);
}
void WIWeb::SelectAll()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_select_all(browser);
}
void WIWeb::Undo()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_undo(browser);
}
void WIWeb::SetZoomLevel(double lv)
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_set_zoom_level(browser, lv);
}
double WIWeb::GetZoomLevel()
{
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return 0.0;
	return cef::get_wrapper().browser_get_zoom_level(browser);
}

cef::CWebRenderHandler *WIWeb::GetRenderer() { return m_webRenderer.get(); }
cef::CWebBrowserClient *WIWeb::GetBrowserClient() { return m_browserClient.get(); }
cef::CWebBrowser *WIWeb::GetBrowser() { return m_browser.get(); }

void WIWeb::OnCursorEntered() { WIBase::OnCursorEntered(); }
void WIWeb::OnCursorExited() { WIBase::OnCursorExited(); }
Vector2i WIWeb::GetBrowserMousePos() const { return {m_mousePos.x / static_cast<float>(GetWidth()) * m_browserViewSize.x, m_mousePos.y / static_cast<float>(GetHeight()) * m_browserViewSize.y}; }
void WIWeb::OnCursorMoved(int x, int y)
{
	m_mousePos = {x, y};
	WIBase::OnCursorMoved(x, y);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	auto brMousePos = GetBrowserMousePos();
	cef::get_wrapper().browser_send_event_mouse_move(browser, brMousePos.x, brMousePos.y, !PosInBounds(x, y), m_buttonMods);
}
util::EventReply WIWeb::OnDoubleClick()
{
	WIBase::OnDoubleClick();

	auto *browser = GetBrowser();
	if(browser == nullptr)
		return util::EventReply::Unhandled;
	auto brMousePos = GetBrowserMousePos();
	cef::get_wrapper().browser_send_event_mouse_click(browser, brMousePos.x, brMousePos.y, 'l', false, 2);
	return util::EventReply::Handled;
}
util::EventReply WIWeb::MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
{
	switch(button) {
	case GLFW::MouseButton::Left:
		umath::set_flag(m_buttonMods, cef::Modifier::LeftMouseButton, state == GLFW::KeyState::Press);
		break;
	case GLFW::MouseButton::Right:
		umath::set_flag(m_buttonMods, cef::Modifier::RightMouseButton, state == GLFW::KeyState::Press);
		break;
	case GLFW::MouseButton::Middle:
		umath::set_flag(m_buttonMods, cef::Modifier::MiddleMouseButton, state == GLFW::KeyState::Press);
		break;
	}

	auto result = WIBase::MouseCallback(button, state, mods);
	if(result == util::EventReply::Handled)
		return result;
	auto *browser = GetBrowser();
	if(browser == nullptr || (state != GLFW::KeyState::Press && state != GLFW::KeyState::Release))
		return util::EventReply::Unhandled;
	auto brMousePos = GetBrowserMousePos();
	char btType;
	switch(button) {
	case GLFW::MouseButton::Left:
		btType = 'l';
		break;
	case GLFW::MouseButton::Right:
		btType = 'r';
		break;
	case GLFW::MouseButton::Middle:
		btType = 'm';
		break;
	default:
		return util::EventReply::Unhandled;
	}
	cef::get_wrapper().browser_send_event_mouse_click(browser, brMousePos.x, brMousePos.y, btType, (state == GLFW::KeyState::Press) ? false : true, 1);

	//auto frame = browser->GetMainFrame();
	//frame->ExecuteJavaScript("function test() {alert('ExecuteJavaScript works!');}",frame->GetURL(),0);

	//frame->GetV8Context()->GetGlobal()->GetValue("test")->ExecuteFunction();
	return util::EventReply::Handled;
}
static cef::Modifier get_cef_modifiers(GLFW::Modifier mods)
{
	auto cefMods = cef::Modifier::None;
	if(umath::is_flag_set(mods, GLFW::Modifier::Shift))
		cefMods |= cef::Modifier::ShiftDown;
	if(umath::is_flag_set(mods, GLFW::Modifier::Alt))
		cefMods |= cef::Modifier::AltDown;
	if(umath::is_flag_set(mods, GLFW::Modifier::Control))
		cefMods |= cef::Modifier::ControlDown;
	if(umath::is_flag_set(mods, GLFW::Modifier::Super))
		cefMods |= cef::Modifier::CommandDown;
	return cefMods;
}
util::EventReply WIWeb::KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods)
{
	WIBase::KeyboardCallback(key, scanCode, state, mods);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return util::EventReply::Unhandled;
	int32_t systemKey = -1;
	std::optional<char> c {};
	switch(key) {
#ifdef _WIN32
	case GLFW::Key::Escape:
		systemKey = VK_ESCAPE;
		break;
	case GLFW::Key::Enter:
		systemKey = VK_RETURN;
		break;
	case GLFW::Key::Tab:
		systemKey = VK_TAB;
		break;
	case GLFW::Key::Backspace:
		systemKey = VK_BACK;
		break;
	case GLFW::Key::Insert:
		systemKey = VK_INSERT;
		break;
	case GLFW::Key::Delete:
		systemKey = VK_DELETE;
		break;
	case GLFW::Key::Right:
		systemKey = VK_RIGHT;
		break;
	case GLFW::Key::Left:
		systemKey = VK_LEFT;
		break;
	case GLFW::Key::Down:
		systemKey = VK_DOWN;
		break;
	case GLFW::Key::Up:
		systemKey = VK_UP;
		break;
	case GLFW::Key::PageUp:
		systemKey = VK_PRIOR;
		break;
	case GLFW::Key::PageDown:
		systemKey = VK_NEXT;
		break;
	case GLFW::Key::Home:
		systemKey = VK_HOME;
		break;
	case GLFW::Key::End:
		systemKey = VK_END;
		break;
	case GLFW::Key::CapsLock:
		systemKey = VK_CAPITAL;
		break;
	case GLFW::Key::ScrollLock:
		systemKey = VK_SCROLL;
		break;
	case GLFW::Key::NumLock:
		systemKey = VK_NUMLOCK;
		break;
	case GLFW::Key::PrintScreen:
		systemKey = VK_PRINT;
		break;
	case GLFW::Key::Pause:
		systemKey = VK_PAUSE;
		break;
	case GLFW::Key::F1:
		systemKey = VK_F1;
		break;
	case GLFW::Key::F2:
		systemKey = VK_F2;
		break;
	case GLFW::Key::F3:
		systemKey = VK_F3;
		break;
	case GLFW::Key::F4:
		systemKey = VK_F4;
		break;
	case GLFW::Key::F5:
		systemKey = VK_F5;
		break;
	case GLFW::Key::F6:
		systemKey = VK_F6;
		break;
	case GLFW::Key::F7:
		systemKey = VK_F7;
		break;
	case GLFW::Key::F8:
		systemKey = VK_F8;
		break;
	case GLFW::Key::F9:
		systemKey = VK_F9;
		break;
	case GLFW::Key::F10:
		systemKey = VK_F10;
		break;
	case GLFW::Key::F11:
		systemKey = VK_F11;
		break;
	case GLFW::Key::F12:
		systemKey = VK_F12;
		break;
	case GLFW::Key::F13:
		systemKey = VK_F13;
		break;
	case GLFW::Key::F14:
		systemKey = VK_F14;
		break;
	case GLFW::Key::F15:
		systemKey = VK_F15;
		break;
	case GLFW::Key::F16:
		systemKey = VK_F16;
		break;
	case GLFW::Key::F17:
		systemKey = VK_F17;
		break;
	case GLFW::Key::F18:
		systemKey = VK_F18;
		break;
	case GLFW::Key::F19:
		systemKey = VK_F19;
		break;
	case GLFW::Key::F20:
		systemKey = VK_F20;
		break;
	case GLFW::Key::F21:
		systemKey = VK_F21;
		break;
	case GLFW::Key::F22:
		systemKey = VK_F22;
		break;
	case GLFW::Key::F23:
		systemKey = VK_F23;
		break;
	case GLFW::Key::F24:
		systemKey = VK_F24;
		break;
	//case GLFW::Key::F25:
	//	systemKey = VK_F25;
	//	break;
	case GLFW::Key::Kp0:
		systemKey = VK_NUMPAD0;
		break;
	case GLFW::Key::Kp1:
		systemKey = VK_NUMPAD1;
		break;
	case GLFW::Key::Kp2:
		systemKey = VK_NUMPAD2;
		break;
	case GLFW::Key::Kp3:
		systemKey = VK_NUMPAD3;
		break;
	case GLFW::Key::Kp4:
		systemKey = VK_NUMPAD4;
		break;
	case GLFW::Key::Kp5:
		systemKey = VK_NUMPAD5;
		break;
	case GLFW::Key::Kp6:
		systemKey = VK_NUMPAD6;
		break;
	case GLFW::Key::Kp7:
		systemKey = VK_NUMPAD7;
		break;
	case GLFW::Key::Kp8:
		systemKey = VK_NUMPAD8;
		break;
	case GLFW::Key::Kp9:
		systemKey = VK_NUMPAD9;
		break;
	case GLFW::Key::KpDecimal:
		systemKey = VK_DECIMAL;
		break;
	case GLFW::Key::KpDivide:
		systemKey = VK_DIVIDE;
		break;
	case GLFW::Key::KpMultiply:
		systemKey = VK_MULTIPLY;
		break;
	case GLFW::Key::KpSubtract:
		systemKey = VK_SUBTRACT;
		break;
	case GLFW::Key::KpAdd:
		systemKey = VK_ADD;
		break;
	case GLFW::Key::KpEnter:
		systemKey = VK_RETURN;
		break;
	//case GLFW::Key::KpEqual:
	//	systemKey = VK_KP_EQUAL;
	//	break;
	case GLFW::Key::LeftShift:
		systemKey = VK_LSHIFT;
		break;
	case GLFW::Key::LeftControl:
		systemKey = VK_LCONTROL;
		break;
	case GLFW::Key::LeftAlt:
		systemKey = VK_MENU;
		break;
	case GLFW::Key::LeftSuper:
		systemKey = VK_LWIN;
		break;
	case GLFW::Key::RightShift:
		systemKey = VK_LSHIFT;
		break;
	case GLFW::Key::RightControl:
		systemKey = VK_LCONTROL;
		break;
	case GLFW::Key::RightAlt:
		systemKey = VK_MENU;
		break;
	case GLFW::Key::RightSuper:
		systemKey = VK_RWIN;
		break;
	case GLFW::Key::Menu:
		systemKey = VK_MENU;
		break;
#else
	// Linux Equivalent
	case GLFW::Key::Escape:
		systemKey = KEY_ESC;
		break;
	case GLFW::Key::Enter:
		systemKey = KEY_ENTER;
		break;
	case GLFW::Key::Tab:
		systemKey = KEY_TAB;
		break;
	case GLFW::Key::Backspace:
		systemKey = KEY_BACKSPACE;
		break;
	case GLFW::Key::Insert:
		systemKey = KEY_INSERT;
		break;
	case GLFW::Key::Delete:
		systemKey = KEY_DELETE;
		break;
	case GLFW::Key::Right:
		systemKey = KEY_RIGHT;
		break;
	case GLFW::Key::Left:
		systemKey = KEY_LEFT;
		break;
	case GLFW::Key::Down:
		systemKey = KEY_DOWN;
		break;
	case GLFW::Key::Up:
		systemKey = KEY_UP;
		break;
	case GLFW::Key::PageUp:
		systemKey = KEY_PAGEUP;
		break;
	case GLFW::Key::PageDown:
		systemKey = KEY_PAGEDOWN;
		break;
	case GLFW::Key::Home:
		systemKey = KEY_HOME;
		break;
	case GLFW::Key::End:
		systemKey = KEY_END;
		break;
	case GLFW::Key::CapsLock:
		systemKey = KEY_CAPSLOCK;
		break;
	case GLFW::Key::ScrollLock:
		systemKey = KEY_SCROLLLOCK;
		break;
	case GLFW::Key::NumLock:
		systemKey = KEY_NUMLOCK;
		break;
	case GLFW::Key::PrintScreen:
		systemKey = KEY_PRINT;
		break;
	case GLFW::Key::Pause:
		systemKey = KEY_PAUSE;
		break;
	case GLFW::Key::F1:
		systemKey = KEY_F1;
		break;
	case GLFW::Key::F2:
		systemKey = KEY_F2;
		break;
	case GLFW::Key::F3:
		systemKey = KEY_F3;
		break;
	case GLFW::Key::F4:
		systemKey = KEY_F4;
		break;
	case GLFW::Key::F5:
		systemKey = KEY_F5;
		break;
	case GLFW::Key::F6:
		systemKey = KEY_F6;
		break;
	case GLFW::Key::F7:
		systemKey = KEY_F7;
		break;
	case GLFW::Key::F8:
		systemKey = KEY_F8;
		break;
	case GLFW::Key::F9:
		systemKey = KEY_F9;
		break;
	case GLFW::Key::F10:
		systemKey = KEY_F10;
		break;
	case GLFW::Key::F11:
		systemKey = KEY_F11;
		break;
	case GLFW::Key::F12:
		systemKey = KEY_F12;
		break;
	case GLFW::Key::F13:
		systemKey = KEY_F13;
		break;
	case GLFW::Key::F14:
		systemKey = KEY_F14;
		break;
	case GLFW::Key::F15:
		systemKey = KEY_F15;
		break;
	case GLFW::Key::F16:
		systemKey = KEY_F16;
		break;
	case GLFW::Key::F17:
		systemKey = KEY_F17;
		break;
	case GLFW::Key::F18:
		systemKey = KEY_F18;
		break;
	case GLFW::Key::F19:
		systemKey = KEY_F19;
		break;
	case GLFW::Key::F20:
		systemKey = KEY_F20;
		break;
	case GLFW::Key::F21:
		systemKey = KEY_F21;
		break;
	case GLFW::Key::F22:
		systemKey = KEY_F22;
		break;
	case GLFW::Key::F23:
		systemKey = KEY_F23;
		break;
	case GLFW::Key::F24:
		systemKey = KEY_F24;
		break;
	//case GLFW::Key::F25:
	//	systemKey = VK_F25;
	//	break;
	case GLFW::Key::Kp0:
		systemKey = KEY_KP0;
		break;
	case GLFW::Key::Kp1:
		systemKey = KEY_KP1;
		break;
	case GLFW::Key::Kp2:
		systemKey = KEY_KP2;
		break;
	case GLFW::Key::Kp3:
		systemKey = KEY_KP3;
		break;
	case GLFW::Key::Kp4:
		systemKey = KEY_KP4;
		break;
	case GLFW::Key::Kp5:
		systemKey = KEY_KP5;
		break;
	case GLFW::Key::Kp6:
		systemKey = KEY_KP6;
		break;
	case GLFW::Key::Kp7:
		systemKey = KEY_KP7;
		break;
	case GLFW::Key::Kp8:
		systemKey = KEY_KP8;
		break;
	case GLFW::Key::Kp9:
		systemKey = KEY_KP9;
		break;
	case GLFW::Key::KpDecimal:
		systemKey = KEY_KPDOT;
		break;
	case GLFW::Key::KpDivide:
		systemKey = KEY_KPSLASH;
		break;
	case GLFW::Key::KpMultiply:
		systemKey = KEY_KPASTERISK;
		break;
	case GLFW::Key::KpSubtract:
		systemKey = KEY_KPMINUS;
		break;
	case GLFW::Key::KpAdd:
		systemKey = KEY_KPPLUS;
		break;
	case GLFW::Key::KpEnter:
		systemKey = KEY_KPENTER;
		break;
	//case GLFW::Key::KpEqual:
	//	systemKey = VK_KP_EQUAL;
	//	break;
	case GLFW::Key::LeftShift:
		systemKey = KEY_LEFTSHIFT;
		break;
	case GLFW::Key::LeftControl:
		systemKey = KEY_LEFTCTRL;
		break;
	case GLFW::Key::LeftAlt:
		systemKey = KEY_LEFTALT;
		break;
	case GLFW::Key::LeftSuper:
		systemKey = KEY_LEFTMETA;
		break;
	case GLFW::Key::RightShift:
		systemKey = KEY_RIGHTSHIFT;
		break;
	case GLFW::Key::RightControl:
		systemKey = KEY_LEFTCTRL; //separate key here?
		break;
	case GLFW::Key::RightAlt:
		systemKey = KEY_RIGHTALT;
		break;
	case GLFW::Key::RightSuper:
		systemKey = KEY_RIGHTMETA;
		break;
	case GLFW::Key::Menu:
		systemKey = KEY_MENU;
		break;
#endif
	default:
		{
			if(systemKey == -1) {
				if((mods & (GLFW::Modifier::Alt | GLFW::Modifier::Control | GLFW::Modifier::Super)) == GLFW::Modifier::None) {
					// Already handled by CharCallback
					return util::EventReply::Handled;
				}
				// If alt, control or super keys are pressed, CharCallback won't get invoked, so we'll
				// have to inform cef of the key event here instead.
				c = std::tolower(umath::to_integral(key));
				break;
			}
			else
				return util::EventReply::Unhandled;
		}
	}
	auto press = (state == GLFW::KeyState::Press || state == GLFW::KeyState::Repeat);
	auto cefMods = get_cef_modifiers(mods) | m_buttonMods;
	if(state == GLFW::KeyState::Repeat)
		cefMods |= cef::Modifier::IsRepeat;
	cef::get_wrapper().browser_send_event_key(browser, c.has_value() ? *c : systemKey, systemKey, systemKey, press, cefMods);
	return util::EventReply::Handled;
}
util::EventReply WIWeb::CharCallback(unsigned int c, GLFW::Modifier mods)
{
	WIBase::CharCallback(c, mods);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return util::EventReply::Unhandled;
	cef::get_wrapper().browser_send_event_char(browser, c, get_cef_modifiers(mods) | m_buttonMods);
	return util::EventReply::Handled;
}
util::EventReply WIWeb::ScrollCallback(Vector2 offset)
{
	WIBase::ScrollCallback(offset);
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return util::EventReply::Unhandled;
	auto brMousePos = GetBrowserMousePos();
	cef::get_wrapper().browser_send_event_mouse_wheel(browser, brMousePos.x, brMousePos.y, offset.x * 5.f, offset.y * 5.f);
	return util::EventReply::Handled;
}
void WIWeb::OnFocusGained()
{
	WIBase::OnFocusGained();
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_set_focus(browser, true);
}
void WIWeb::OnFocusKilled()
{
	WIBase::OnFocusKilled();
	auto *browser = GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_set_focus(browser, false);
}
#pragma optimize("", on)
