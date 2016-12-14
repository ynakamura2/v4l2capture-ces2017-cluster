#ifndef WAYLAND_H_
#define WAYLAND_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>

struct ivi_surface;
struct ivi_application;

// Wayland wrapper class
class Wayland {
 public:
	// Constructor
	Wayland();
	// Destructor
	~Wayland();
	// Create Context
	int32_t CreateContext(uint32_t width, uint32_t height, const char* title);
	// Destroy Resources
	void destroyContext();
	// Get Wayland Display
	struct wl_display*    GetDisplay()      const { return mpDisplay;      }
	// Get Wayland Registry
	struct wl_surface*    GetSurface()      const { return mpSurface;      }
	// Get EGL Native Window for Wayland Back-end server
	struct wl_egl_window* GetNativeWindow() const { return mpNativeWindow; }

 private:
	// shell ping listener
	static void ping(void* pData, struct wl_shell_surface* pShellSurface, uint32_t serial);
	// shell configure listener
	static void configure(void* pData, struct wl_shell_surface* pShellSurface,
	                      uint32_t edges, int32_t width, int32_t height);
	// shell popup listener
	static void popup(void* pData, struct wl_shell_surface* pShellSurface);

	// registry listener
	static void registryListener(void* pData, struct wl_registry* pRegistry,
	                             uint32_t name, const char* pInterface, uint32_t version);
	// shell surface listener table
	static struct wl_shell_surface_listener shellSurfaceListenerTbl;
	// registry listener table
	static struct wl_registry_listener registryListenerTbl;
 private:
	struct wl_display*       mpDisplay;
	struct wl_registry*      mpRegistry;
	struct wl_compositor*    mpCompositor;
	struct wl_surface*       mpSurface;
	struct wl_shell*         mpShell;
	struct wl_shell_surface* mpShellSurface;
	struct wl_egl_window*    mpNativeWindow;

	struct ivi_surface* ivi_surface_;
	struct ivi_application* ivi_application_;

 private:
	DISALLOW_COPY_AND_ASSIGN(Wayland);
};
#endif // WAYLAND_H_
