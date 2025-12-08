// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.modules.chromium;

import pragma.client;
import pragma.iclient;

#define PR_CHROMIUM_FIND_SYMBOL(lib, sym) (sym = lib.FindSymbolAddress<decltype(sym)>("pr_chromium_" #sym)) != nullptr
cef::IChromiumWrapper::IChromiumWrapper(util::Library &lib)
{
	m_bValid = PR_CHROMIUM_FIND_SYMBOL(lib, register_javascript_function) && PR_CHROMIUM_FIND_SYMBOL(lib, initialize) && PR_CHROMIUM_FIND_SYMBOL(lib, close) && PR_CHROMIUM_FIND_SYMBOL(lib, do_message_loop_work) && PR_CHROMIUM_FIND_SYMBOL(lib, parse_url)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_create) && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_release) && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_set_user_data) && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_get_user_data)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_create) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_release) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_user_data) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_get_user_data)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_download_start_callback) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_download_update_callback) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_download_location)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_on_address_change_callback) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_on_loading_state_change) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_on_load_start) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_on_load_end)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_client_set_on_load_error) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_create) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_release) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_close) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_try_close)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_get_user_data) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_was_resized) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_invalidate) && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_set_image_data)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_clear_dirty_rects) && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_get_dirty_rects) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_load_url) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_can_go_back)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_can_go_forward) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_go_back) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_go_forward) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_has_document) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_is_loading)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_reload) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_reload_ignore_cache) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_stop_load) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_copy) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_cut)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_delete) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_paste) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_redo) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_select_all) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_undo)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_set_zoom_level) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_get_zoom_level) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_send_event_mouse_move) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_send_event_mouse_click)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_send_event_key) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_send_event_char) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_send_event_mouse_wheel) && PR_CHROMIUM_FIND_SYMBOL(lib, browser_set_focus)
	  && PR_CHROMIUM_FIND_SYMBOL(lib, browser_execute_java_script) && PR_CHROMIUM_FIND_SYMBOL(lib, render_handler_is_renderer_size_mismatched);
}

static std::optional<bool> initResult = {};
static std::shared_ptr<util::Library> g_libChromiumWrapper = nullptr;
std::unique_ptr<cef::IChromiumWrapper> g_chromiumWrapper = nullptr;
cef::IChromiumWrapper &cef::get_wrapper() { return *g_chromiumWrapper; }
static bool initialize_chromium(std::string &outErr)
{
	if(initResult.has_value())
		return *initResult;
	initResult = false;

	pragma::gui::WGUI::GetInstance().RegisterType<pragma::gui::types::WIWeb>("WIWeb");

	std::string err;

#if _WIN32
	auto lib = util::load_library_module("chromium/pr_chromium_wrapper", {}, {}, &err);
#else
	auto lib = util::load_library_module("chromium/libpr_chromium_wrapper", {}, {}, &err);
#endif
	if(!lib) {
		outErr = std::move(err);
		return false;
	}
	lib->SetDontFreeLibraryOnDestruct(); // Don't free until program shutdown (otherwise chromium may not shut down correctly)
	g_libChromiumWrapper = lib;
	g_chromiumWrapper = std::make_unique<cef::IChromiumWrapper>(*lib);
	if(!g_chromiumWrapper->valid()) {
		outErr = "Unable to initialize chromium wrapper: One or more symbols not found!";
		return false;
	}
#if _WIN32
	auto pathToSubProcess = util::FilePath("modules/chromium/pr_chromium_subprocess.exe");
#elif __linux__
	auto pathToSubProcess = util::FilePath("modules/chromium/pr_chromium_subprocess");
#endif

	std::string absPathToSubProcess;
	if(!filemanager::find_absolute_path(pathToSubProcess.GetString(), absPathToSubProcess)) {
		outErr = "Unable to locale path to pr_chromium_subprocess executable ('" + pathToSubProcess.GetString() + "')";
		return false;
	}
	pathToSubProcess = util::FilePath(absPathToSubProcess);

	auto localPathToCache = util::Path::CreatePath("cache/chromium");
	filemanager::create_path(localPathToCache.GetString());
	auto pathToCache = util::Path::CreatePath(filemanager::get_program_write_path()) + localPathToCache;

	// CEF overrides our crash handlers. To circumvent that, we re-initialize ours after CEF has been initialized.
	util::ScopeGuard reinitCrashHandler {[]() { pragma::debug::CrashHandler::Initialize(); }};

	std::string initErr;
	if(!g_chromiumWrapper->initialize(pathToSubProcess.GetString().c_str(), pathToCache.GetString().c_str(), pragma::get_cengine()->IsCPURenderingOnly(), initErr)) {
		g_chromiumWrapper = nullptr;
		outErr = "Unable to initialize chromium wrapper: Failed to initialize chromium: " + initErr;
		return false;
	}
	initResult = true;
	return true;
}
static void release_chromium()
{
	if(!initResult.has_value() || *initResult == false)
		return;
	g_chromiumWrapper->close();
	g_chromiumWrapper = nullptr;
	initResult = {};

	g_libChromiumWrapper = nullptr;
}
extern "C" {
bool PR_EXPORT pragma_attach(std::string &outErr) { return initialize_chromium(outErr); }
void PR_EXPORT pragma_detach() { release_chromium(); }
void PR_EXPORT pragma_initialize_lua(Lua::Interface &l)
{
	Lua::chromium::register_library(l);
	pragma::gui::types::WIWeb::register_callbacks();

	// GUI classes have to be registered for the client-state AND the GUI-state,
	// so we have to tell the engine that we need to be registered for both whenever we're loaded.
	iclient::load_as_gui_module();
}
};

extern "C" {

PR_EXPORT void wv_chromium_load_url(pragma::gui::types::WIBase *p, const std::string &url)
{
	auto *pWeb = dynamic_cast<pragma::gui::types::WIWeb *>(p);
	if(pWeb == nullptr)
		return;
	pWeb->LoadURL(url);
}

PR_EXPORT void wv_chromium_set_browser_view_size(pragma::gui::types::WIBase *p, const Vector2i &viewSize)
{
	auto *pWeb = dynamic_cast<pragma::gui::types::WIWeb *>(p);
	if(pWeb == nullptr)
		return;
	pWeb->SetBrowserViewSize(viewSize);
}

PR_EXPORT void wv_chromium_set_transparent_background(pragma::gui::types::WIBase *p, bool b)
{
	auto *pWeb = dynamic_cast<pragma::gui::types::WIWeb *>(p);
	if(pWeb == nullptr)
		return;
	pWeb->SetTransparentBackground(b);
}

PR_EXPORT void wv_chromium_register_javascript_function(const char *name, cef::JSValue *(*const fCallback)(cef::JSValue *, uint32_t)) { cef::get_wrapper().register_javascript_function(name, fCallback); }

PR_EXPORT void wv_chromium_exec_javascript(pragma::gui::types::WIBase *p, const std::string &js)
{
	auto *pWeb = dynamic_cast<pragma::gui::types::WIWeb *>(p);
	if(pWeb == nullptr)
		return;
	auto *browser = pWeb->GetBrowser();
	if(browser == nullptr)
		return;
	cef::get_wrapper().browser_execute_java_script(browser, js.c_str(), nullptr);
}
};

#if 0
static void write_tga(VFilePtrReal &f,uint32_t w,uint32_t h,const std::vector<unsigned char> &pixels)
{
	f->Write<char>((char)(0)); // Length of image ID field
	f->Write<char>((char)(0)); // No color map included
	f->Write<char>(2); // Uncompressed true-color image
	
	// Color map
	f->Write<short>((short)(0));
	f->Write<short>((short)(0));
	f->Write<char>((char)(0));

	f->Write<short>((short)(0)); // x-origin (origin = lower-left corner)
	f->Write<short>((short)(0)); // y-origin
	
	f->Write<char>(w %256);
	f->Write<char>(CInt8(w /256));

	f->Write<char>(h %256);
	f->Write<char>(CInt8(h /256));

	f->Write<char>(24); // Bits per pixel
	f->Write<char>((char)(0)); // Image descriptor

	f->Write(pixels.data(),pixels.size());
}

static void swap_red_blue(std::vector<unsigned char> &pixels,uint32_t w,uint32_t h)
{
	unsigned int size = w *h *3;
	unsigned char tmp;
	for(unsigned int i=0;i<size;i+=3)
	{
		tmp = pixels[i];
		pixels[i] = pixels[i +2];
		pixels[i +2] = tmp;
	}
}

static bool save_tga(const std::string &fileName,uint32_t width,uint32_t height,const void *data)
{
	auto f = FileManager::OpenFile<VFilePtrReal>(fileName.c_str(),"wb");
	if(f == nullptr)
		return false;
	uint32_t offset = 0;
	uint32_t rowPitch = width;
	auto bSwapped = false;
	auto byteSize = 4;
	auto format = vk::Format::eR8G8B8A8Unorm;
	std::vector<unsigned char> pixels(width *height *4);
	size_t pos = 0;
	auto *ptr = static_cast<const char*>(data);
	for(auto y=height;y>0;)
	{
		--y;
		ptr = static_cast<const char*>(data) +offset +y *rowPitch;
		auto *row = ptr;
		if(format == vk::Format::eR8G8B8A8Unorm)
		{
			for(auto x=decltype(width){0};x<width;++x)
			{
				auto *px = static_cast<const unsigned char*>(static_cast<const void*>(row));
				pixels[pos] = (bSwapped == false) ? px[0] : px[2];
				pixels[pos +1] = px[1];
				pixels[pos +2] = (bSwapped == false) ? px[2] : px[0];
				//std::cout<<"Pixel Color: "<<+pixels[pos]<<","<<+pixels[pos +1]<<","<<+pixels[pos +2]<<std::endl;
				pos += 3;
				row += byteSize;
			}
		}
		else if(format == vk::Format::eR8G8B8Unorm)
		{
			for(auto x=decltype(width){0};x<width;++x)
			{
				auto *px = static_cast<const unsigned char*>(static_cast<const void*>(row));
				pixels[pos] = (bSwapped == false) ? px[0] : px[2];
				pixels[pos +1] = px[1];
				pixels[pos +2] = (bSwapped == false) ? px[2] : px[0];
				pos += 3;
				row += byteSize;
			}
		}
		else if(format == vk::Format::eR8Unorm)
		{
			for(auto x=decltype(width){0};x<width;++x)
			{
				auto *px = static_cast<const unsigned char*>(static_cast<const void*>(row));
				pixels[pos] = px[0];
				pixels[pos +1] = px[0];
				pixels[pos +2] = px[0];
				pos += 3;
				row += byteSize;
			}
		}
		else if(format == vk::Format::eR16Unorm)
		{
			for(auto x=decltype(width){0};x<width;++x)
			{
				auto *px = static_cast<const unsigned char*>(static_cast<const void*>(row));
				pixels[pos] = px[0];
				pixels[pos +1] = px[0];
				pixels[pos +2] = px[0];
				pos += 3;
				row += byteSize;
			}
		}
		else if(format == vk::Format::eD16Unorm)
		{
			for(auto x=decltype(width){0};x<width;++x)
			{
				auto val = 0.f;
				auto *px = static_cast<const unsigned char*>(static_cast<const void*>(row));
				memcpy(&val,px,byteSize);
				val = umath::min(val *10000.f,255.f); // Vulkan TODO
				pixels[pos] = static_cast<unsigned char>(val);
				pixels[pos +1] = static_cast<unsigned char>(val);
				pixels[pos +2] = static_cast<unsigned char>(val);
				pos += 3;
				row += byteSize;
			}
		}
	}
	swap_red_blue(pixels,width,height);
	write_tga(f,width,height,pixels);
	return true;
}

class RenderHandler
	: public CefRenderHandler
{
public:
	RenderHandler()=default;
	virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,CefRect& rect) override
	{
		rect = CefRect(0,0,1920,1080);//m_texture->GetWidth(), m_texture->GetHeight());
		return true;
		//return false;
	}
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
	{
		rect = CefRect(0,0,1920,1080);//m_texture->GetWidth(), m_texture->GetHeight());
		return true;
		//return false;
	}
	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,int viewX,int viewY,int &screenX,int &screenY) override
	{
		screenX = viewX;
		screenY = viewY;
		return true;
	}
	virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,CefScreenInfo &screen_info) override
	{
		return false;
	}
	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser,bool show) override
	{

	}
	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,const CefRect& rect) override
	{

	}
	virtual void OnPaint(CefRefPtr<CefBrowser> browser,PaintElementType type,const RectList &dirtyRects,const void *buffer,int width, int height) override
	{
		static auto bSaveTGA = true;
		if(bSaveTGA == false)
		{
			//save_tga
			save_tga("test.tga",width,height,buffer);
		}
		std::cout<<"OnPaint..."<<std::endl;
		/*Ogre::HardwarePixelBufferSharedPtr texBuf = m_renderTexture->getBuffer();
		texBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		memcpy(texBuf->getCurrentLock().data, buffer, width*height*4);
		texBuf->unlock();*/
	}
	virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,CefCursorHandle cursor,CursorType type,const CefCursorInfo& custom_cursor_info) override
	{

	}
	virtual bool StartDragging(CefRefPtr<CefBrowser> browser,CefRefPtr<CefDragData> drag_data,DragOperationsMask allowed_ops,int x,int y) override
	{
		return false;
	}
	virtual void UpdateDragCursor(CefRefPtr<CefBrowser> browser,DragOperation operation) override
	{

	}
	virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,double x,double y) override
	{

	}
	virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,const CefRange &selected_range,const RectList &character_bounds) override
	{

	}
	IMPLEMENT_REFCOUNTING(RenderHandler);
private:
	Vulkan::Texture m_texture = nullptr;
};

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler {
 public:
  explicit SimpleHandler(bool use_views);
  ~SimpleHandler();

  // Provide access to the single global instance of this object.
  static SimpleHandler* GetInstance();

  // CefClient methods:
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
    return this;
  }

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

 private:
  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  // True if the application is using the Views framework.
  const bool use_views_;

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>

namespace {

SimpleHandler* g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler(bool use_views)
    : use_views_(use_views),
      is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (use_views_) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if (browser_view) {
      CefRefPtr<CefWindow> window = browser_view->GetWindow();
      if (window)
        window->SetTitle(title);
    }
  } else {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                                        const CefString& title) {
  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
 // SetWindowText(hwnd, title.c_str());
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL " << std::string(failedUrl) <<
        " with error " << std::string(errorText) << " (" << errorCode <<
        ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}


// for manual render handler
class BrowserClient
	: public CefClient
{
public:
	BrowserClient(RenderHandler *renderHandler)
		: m_renderHandler(renderHandler)
	{}
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {return m_renderHandler;}
	CefRefPtr<CefRenderHandler> m_renderHandler;
	IMPLEMENT_REFCOUNTING(BrowserClient);
};

#ifndef TEST_EXE

#else
int main(int argc,char *argv[])
{
	CefMainArgs args {};
	auto result = CefExecuteProcess(args,nullptr,nullptr);
	if(result > 0)
		return result;

	CefSettings settings {};
	//settings.locale
	settings.windowless_rendering_enabled = true;
	//settings.multi_threaded_message_loop = true;
	settings.no_sandbox = true;
	if(CefInitialize(args,settings,nullptr,nullptr) == false)
		return EXIT_FAILURE;

	auto renderHandler = std::make_shared<RenderHandler>();

	CefRefPtr<CefBrowser> browser;
	CefRefPtr<BrowserClient> browserClient;
	CefWindowInfo window_info;
	CefBrowserSettings browserSettings;
	//browserSettings.windowless_frame_rate = 60; // 30 is default

	auto hWindow = GetActiveWindow();
	window_info.SetAsWindowless(nullptr,false);//hWindow,false); // false means no transparency (site background colour)
	//window_info.windowless_rendering_enabled = true;
	//window_info.SetAsPopup(NULL, "cefsimple");

	CefRefPtr<SimpleHandler> handler(new SimpleHandler(false));

	browserClient = new BrowserClient(renderHandler.get());
   // CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
    //    handler, "https://forums.sciolyte.com/", browserSettings, NULL, NULL);
	//browserClient = new BrowserClient(renderHandler.get());
	//browser = CefBrowserHost::CreateBrowserSync(window_info,browserClient.get(),"http://www.google.com",browserSettings,nullptr);

    // Create the first browser window.
   // CefBrowserHost::CreateBrowser(window_info, browserClient, "https://www.w3schools.com/tags/tryit.asp?filename=tryhtml5_audio_autoplay", browserSettings,
   //                               NULL);
	browser = CefBrowserHost::CreateBrowserSync(window_info,browserClient.get(),"https://www.w3schools.com/tags/tryit.asp?filename=tryhtml5_audio_autoplay",browserSettings,nullptr);

	CefRunMessageLoop();
	CefShutdown();


	return EXIT_SUCCESS;
}
#endif
#endif
