#include "wayland.h"

#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "ivi-application-client-protocol.h"
#define IVI_SURFACE_ID 20000

// Constructor
Wayland::Wayland()
	: mpDisplay(NULL)
	, mpRegistry(NULL)
	, mpCompositor(NULL)
	, mpSurface(NULL)
	, mpShell(NULL)
	, mpShellSurface(NULL)
	, mpNativeWindow(NULL)
	, ivi_surface_()
	, ivi_application_()
{
}

// Destructor
Wayland::~Wayland() {
}

// if ping event is received, pong request has to be called
void Wayland::ping(void* pData,
                           struct wl_shell_surface* pShellSurface,
                           uint32_t serial) {
    (void)pData;
    wl_shell_surface_pong(pShellSurface, serial);
}

// shell configure listener
void Wayland::configure(void* pData,
                                struct wl_shell_surface* pShellSurface,
                                uint32_t edges, int32_t width, int32_t height) {
    (void)pData;
    (void)pShellSurface;
    (void)edges;
    (void)width;
    (void)height;
}

// shell popup listener
void Wayland::popup(void* pData, struct wl_shell_surface* pShellSurface) {
    (void)pData;
    (void)pShellSurface;
}

// shell surface listener table
struct wl_shell_surface_listener Wayland::shellSurfaceListenerTbl = {
	Wayland::ping,
	Wayland::configure,
	Wayland::popup
};

// registry listener
void Wayland::registryListener(void* pData,
        struct wl_registry* pRegistry, uint32_t name,
        const char* pInterface, uint32_t version) {
	(void)version;
	Wayland* pUtW = static_cast<Wayland*>(pData);

	int ans = 0;

	do {
		// Check pInterface is the needed protocol or not
		ans = strcmp(pInterface, "wl_compositor");
		if (0 == ans) {
			pUtW->mpCompositor
				= (struct wl_compositor*)wl_registry_bind(
						pRegistry, name, &wl_compositor_interface, 1);
				break;
		}

		ans= strcmp(pInterface, "wl_shell");
		if (0 == ans) {
			pUtW->mpShell
				= (struct wl_shell*)wl_registry_bind(
						pRegistry, name, &wl_shell_interface, 1);
		}

		ans = strcmp(pInterface, "ivi_application");
		if (0 == ans) {
			pUtW->ivi_application_
				= (struct ivi_application*)wl_registry_bind(
						pRegistry, name, &ivi_application_interface, 1);
		}
	} while(0);
}

// registry listener table
struct wl_registry_listener Wayland::registryListenerTbl = {
	Wayland::registryListener,
	NULL
};

// Create Resources
int32_t Wayland::CreateContext(uint32_t width, uint32_t height, const char* title) {
	bool ret = 0;
	do {
		// display connect to server
		mpDisplay = wl_display_connect(NULL);
		if (NULL == mpDisplay) {
			std::cout << "ERROR: wl_display_connect() failed." << std::endl;
			ret = -1;
			break;
		}

		// get registry object
		mpRegistry = wl_display_get_registry(mpDisplay);
		if (NULL == mpRegistry) {
			std::cout << "ERROR: wl_display_get_registry() failed." << std::endl;
			ret = -2;
			break;
		}

		// add listener to registry
		wl_registry_add_listener(mpRegistry, &Wayland::registryListenerTbl, (void*)this);
		wl_display_dispatch(mpDisplay);
		wl_display_roundtrip(mpDisplay);

		// create wl_surface
		mpSurface = wl_compositor_create_surface(mpCompositor);
		if (NULL == mpSurface) {
			std::cout << "ERROR: wl_compositor_create_surface() failed." << std::endl;
			ret = -3;
			break;
		}

		if (NULL != mpShell) {
			mpShellSurface = wl_shell_get_shell_surface(mpShell, mpSurface);
			if (NULL == mpShellSurface) {
				std::cout << "ERROR: wl_shell_get_shell_surface() failed." << std::endl;
				ret = -5;
				break;
			}
		}

		if (NULL != mpShell) {
			wl_shell_surface_add_listener(
				reinterpret_cast<struct wl_shell_surface*>(mpShellSurface),
				&Wayland::shellSurfaceListenerTbl, this);

			wl_shell_surface_set_toplevel(mpShellSurface);
			wl_shell_surface_set_title(mpShellSurface, title);
		}

		mpNativeWindow = wl_egl_window_create(mpSurface, width, height);
		if (NULL == mpNativeWindow) {
			std::cout << "Error: wl_egl_window_create() failed." << std::endl;
			ret = -6;
		}

		uint32_t id_ivisurf = IVI_SURFACE_ID;
		ivi_surface_ =
			ivi_application_surface_create(ivi_application_,
						       id_ivisurf, mpSurface);
		if (ivi_surface_ == NULL) {
			fprintf(stderr, "Failed to create ivi_client_surface\n");
			abort();
		}

		return 0;

	} while(0);

	destroyContext();

    return ret;
}

// Destroy Resources
void Wayland::destroyContext() {
	if (NULL != mpNativeWindow) {
		wl_egl_window_destroy(mpNativeWindow);
		mpNativeWindow = NULL;
	}
	if (NULL != mpShellSurface) {
		wl_shell_surface_destroy(mpShellSurface);
		mpShellSurface = NULL;
	}
	if (NULL != mpSurface) {
		wl_surface_destroy(mpSurface);
		mpSurface = NULL;
	}
	if (NULL != mpShell) {
		wl_shell_destroy(mpShell);
		mpShell = NULL;
	}

	if (ivi_application_) {
		ivi_surface_destroy(ivi_surface_);
		ivi_application_destroy(ivi_application_);
	}

	if (NULL != mpCompositor) {
		wl_compositor_destroy(mpCompositor);
		mpCompositor = NULL;
	}
	if (NULL != mpRegistry) {
		wl_registry_destroy(mpRegistry);
		mpRegistry = NULL;
	}

	if (ivi_application_) {
		wl_display_roundtrip(mpDisplay);
	}

	if (NULL != mpDisplay) {
		wl_display_disconnect(mpDisplay);
		mpDisplay = NULL;
	}
}
