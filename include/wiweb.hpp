#ifndef __WIWEB_HPP__
#define __WIWEB_HPP__

#include <wgui/types/wirect.h>
#include <image/prosper_texture.hpp>
#include "chromium_wrapper.hpp"

#ifdef __linux__
#define DLLEXPORT __attribute__((visibility("default")))
#else
#define DLLEXPORT __declspec(dllexport)
#endif

class DLLEXPORT WIWeb : public WITexturedRect {
  public:
	static void register_callbacks();

	WIWeb();
	virtual ~WIWeb() override;
	virtual void Initialize() override;
	virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;

	virtual void OnCursorEntered() override;
	virtual void OnCursorExited() override;
	virtual void OnCursorMoved(int x, int y) override;
	virtual util::EventReply MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods) override;
	virtual util::EventReply OnDoubleClick() override;
	virtual util::EventReply KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods) override;
	virtual util::EventReply CharCallback(unsigned int c, GLFW::Modifier mods = GLFW::Modifier::None) override;
	virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	virtual void DoUpdate() override;

	void Close();
	void LoadURL(const std::string &url);
	void SetBrowserViewSize(Vector2i size);
	const Vector2i &GetBrowserViewSize() const;
	bool CanGoBack();
	bool CanGoForward();
	void GoBack();
	void GoForward();
	bool HasDocument();
	bool IsLoading();
	void Reload();
	void ReloadIgnoreCache();
	void StopLoad();

	void Copy();
	void Cut();
	void Delete();
	void Paste();
	void Redo();
	void SelectAll();
	void Undo();

	void SetZoomLevel(double lv);
	double GetZoomLevel();
	void SetTransparentBackground(bool b);
	void ExecuteJavaScript(const std::string &js);

	void SetInitialUrl(std::string url);
	std::string GetUrl() const { return m_url; }

	cef::CWebRenderHandler *GetRenderer();
	cef::CWebBrowserClient *GetBrowserClient();
	cef::CWebBrowser *GetBrowser();
  private:
	bool InitializeChromiumBrowser();
	bool Resize();
	void ClearTexture();
	void CloseBrowserSafely();

	std::string m_initialUrl = "https://pragma-engine.com/";
	std::string m_url;
	std::shared_ptr<prosper::Texture> m_texture = nullptr;
	void *m_imgDataPtr = nullptr;
	bool m_bTransparentBackground = false;
	bool m_browserInitialized = false;
	Vector2i m_browserViewSize = {};
	Vector2i m_mousePos = {};
	Vector2i GetBrowserMousePos() const;

	cef::Modifier m_buttonMods = cef::Modifier::None;
	std::shared_ptr<cef::CWebRenderHandler> m_webRenderer = nullptr;
	std::shared_ptr<cef::CWebBrowserClient> m_browserClient = nullptr;
	std::shared_ptr<cef::CWebBrowser> m_browser = nullptr;
};

#endif
