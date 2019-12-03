#if defined(WIN32) && defined(QTEditor)

#include "Base.h"
#include "Platform.h"
#include "FileSystem.h"
#include "Game.h"
#include "Form.h"

#include "ScriptController.h"
#include <GL/wglew.h>
#include <windowsx.h>
#include <shellapi.h>
#ifdef GP_USE_GAMEPAD
#include <XInput.h>
#endif

double __timeTicksPerMillis;
double __timeStart;
double __timeAbsolute;
bool __vsync = 0; // WINDOW_VSYNC;
bool __mouseCaptured = false;
POINT __mouseCapturePoint = { 0, 0 };
bool __multiSampling = false;
bool __cursorVisible = true;
unsigned int __gamepadsConnected = 0;

HWND __hwnd = 0;
HDC __hdc = 0;
HGLRC __hrc = 0;
RECT __rect;
std::wstring __windowName;

static void WarpMouse(int clientX, int clientY)
{

}

namespace gameplay
{

	struct WindowCreationParams
	{
		RECT rect;
		std::wstring windowName;
		bool fullscreen;
		bool resizable;
		int samples;
	};

	extern void print(const char *format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		int sz = vfprintf(stderr, format, argptr);
		if (sz > 0)
		{
			char *buf = new char[sz + 1];
			vsprintf(buf, format, argptr);
			buf[sz] = 0;
			OutputDebugStringA(buf);
			SAFE_DELETE_ARRAY(buf);
		}
		va_end(argptr);
	}

	extern int strcmpnocase(const char* s1, const char* s2)
	{
		return _strcmpi(s1, s2);
	}

	Platform::Platform(Game *game)
		: _game(game)
	{
	}

	Platform::~Platform()
	{

	}

	bool initializeGL(WindowCreationParams* params)
	{
		// Get the initial time.
		LARGE_INTEGER tps;
		QueryPerformanceFrequency(&tps);
		__timeTicksPerMillis = (double)(tps.QuadPart / 1000L);
		LARGE_INTEGER queryTime;
		QueryPerformanceCounter(&queryTime);
		GP_ASSERT(__timeTicksPerMillis);
		__timeStart = queryTime.QuadPart / __timeTicksPerMillis;
		static    PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,
			8,
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		HWND hwnd = __hwnd;
		if (!__hdc)
		{
			__hdc = GetDC(hwnd);
		}
		HDC hdc = __hdc;
		int pixelFormat = ChoosePixelFormat(hdc, &pfd);
		if (pixelFormat == 0)
		{
			DestroyWindow(hwnd);
			GP_ERROR("Failed to choose a pixel format.");
			return false;
		}

		if (!SetPixelFormat(hdc, pixelFormat, &pfd))
		{
			DestroyWindow(hwnd);
			GP_ERROR("Failed to set the pixel format.");
			return false;
		}

		HGLRC tempContext = wglCreateContext(hdc);
		if (!tempContext)
		{
			DestroyWindow(hwnd);
			GP_ERROR("Failed to create temporary context for initialization.");
			return false;
		}
		__hrc = tempContext;
		wglMakeCurrent(hdc, tempContext);
		// Initialize GLEW
		if (GLEW_OK != glewInit())
		{
			wglDeleteContext(tempContext);
			DestroyWindow(hwnd);
			GP_ERROR("Failed to initialize GLEW.");
			return false;
		}
		// Vertical sync.
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(__vsync ? 1 : 0);
		else
			__vsync = false;

		// Some old graphics cards support EXT_framebuffer_object instead of ARB_framebuffer_object.
		// Patch ARB_framebuffer_object functions to EXT_framebuffer_object ones since semantic is same.
		if (!GLEW_ARB_framebuffer_object && GLEW_EXT_framebuffer_object)
		{
			glBindFramebuffer = glBindFramebufferEXT;
			glBindRenderbuffer = glBindRenderbufferEXT;
			glBlitFramebuffer = glBlitFramebufferEXT;
			glCheckFramebufferStatus = glCheckFramebufferStatusEXT;
			glDeleteFramebuffers = glDeleteFramebuffersEXT;
			glDeleteRenderbuffers = glDeleteRenderbuffersEXT;
			glFramebufferRenderbuffer = glFramebufferRenderbufferEXT;
			glFramebufferTexture1D = glFramebufferTexture1DEXT;
			glFramebufferTexture2D = glFramebufferTexture2DEXT;
			glFramebufferTexture3D = glFramebufferTexture3DEXT;
			glFramebufferTextureLayer = glFramebufferTextureLayerEXT;
			glGenFramebuffers = glGenFramebuffersEXT;
			glGenRenderbuffers = glGenRenderbuffersEXT;
			glGenerateMipmap = glGenerateMipmapEXT;
			glGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
			glGetRenderbufferParameteriv = glGetRenderbufferParameterivEXT;
			glIsFramebuffer = glIsFramebufferEXT;
			glIsRenderbuffer = glIsRenderbufferEXT;
			glRenderbufferStorage = glRenderbufferStorageEXT;
			glRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;
		}
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glClearColor(0.0, 0.0, 0.0, 0.5);
		glClearDepth(1.0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glEnable(GL_BLEND); //Enable blending.
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return true;
	}

	void Platform::SetInfo(void *hwnd, const std::wstring &name)
	{
		 GetClientRect(__hwnd, &__rect);
		__windowName = name;
		__hwnd = (HWND)hwnd;
		__hdc = GetDC(__hwnd);
	}

	void Platform::paintGL()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		if (_game->isInitialized())
		{
			// run : update and render ...
			_game->frame();
			Platform::swapBuffers();
		}
		else
		{
			Platform::swapBuffers();
			// config
			_game->run();
			// initialize
			_game->frame();
		}
	}

	Platform* Platform::create(Game *game)
	{
		WindowCreationParams params;
		params.rect = __rect;
		params.resizable = true;
		params.fullscreen = false;
		params.samples = 0;
		params.windowName = __windowName;

		Platform* platform = new Platform(game);
		if (!initializeGL(&params))
			goto error;
		return platform;

	error:

		exit(0);
		return NULL;
	}

	int Platform::enterMessagePump()
	{
		return 0;
	}

	void Platform::signalShutdown()
	{
		// nothing to do
	}

	bool Platform::canExit()
	{
		return true;
	}

	unsigned int Platform::getDisplayWidth()
	{
		static RECT rect;
		GetClientRect(__hwnd, &rect);
		return rect.right;
	}

	unsigned int Platform::getDisplayHeight()
	{
		static RECT rect;
		GetClientRect(__hwnd, &rect);
		return rect.bottom;
	}

	double Platform::getAbsoluteTime()
	{
		LARGE_INTEGER queryTime;
		QueryPerformanceCounter(&queryTime);
		GP_ASSERT(__timeTicksPerMillis);
		__timeAbsolute = queryTime.QuadPart / __timeTicksPerMillis;
		return __timeAbsolute - __timeStart;
	}

	void Platform::setAbsoluteTime(double time)
	{
		__timeAbsolute = time;
	}

	bool Platform::isVsync()
	{
		return __vsync;
	}

	void Platform::setVsync(bool enable)
	{
		__vsync = enable;
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(enable ? 1 : 0);
		else
			__vsync = false;
	}

	void Platform::swapBuffers()
	{
		if (__hdc)
			::SwapBuffers(__hdc);
	}

	void Platform::sleep(long ms)
	{
		::Sleep(ms);
	}

	void Platform::setMultiSampling(bool enabled)
	{
		if (enabled == __multiSampling)
		{
			return;
		}

		if (enabled)
		{
			glEnable(GL_MULTISAMPLE);
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
		}

		__multiSampling = enabled;
	}

	bool Platform::isMultiSampling()
	{
		return __multiSampling;
	}

	void Platform::setMultiTouch(bool enabled)
	{
		// not supported
	}

	bool Platform::isMultiTouch()
	{
		return false;
	}

	bool Platform::hasAccelerometer()
	{
		return false;
	}

	void Platform::getAccelerometerValues(float *pitch, float *roll)
	{
		GP_ASSERT(pitch);
		GP_ASSERT(roll);

		*pitch = 0;
		*roll = 0;
	}

	void Platform::getSensorValues(float *accelX, float *accelY, float *accelZ, float *gyroX, float *gyroY, float *gyroZ)
	{
		if (accelX)
		{
			*accelX = 0;
		}

		if (accelY)
		{
			*accelY = 0;
		}

		if (accelZ)
		{
			*accelZ = 0;
		}

		if (gyroX)
		{
			*gyroX = 0;
		}

		if (gyroY)
		{
			*gyroY = 0;
		}

		if (gyroZ)
		{
			*gyroZ = 0;
		}
	}

	void Platform::getArguments(int *argc, char*** argv)
	{
		if (argc)
			*argc = __argc;
		if (argv)
			*argv = __argv;
	}

	bool Platform::hasMouse()
	{
		return true;
	}

	void Platform::setMouseCaptured(bool captured)
	{
		if (captured != __mouseCaptured)
		{
			if (captured)
			{
				// Hide the cursor and warp it to the center of the screen
				__mouseCapturePoint.x = getDisplayWidth() / 2;
				__mouseCapturePoint.y = getDisplayHeight() / 2;

				ShowCursor(FALSE);
				WarpMouse(__mouseCapturePoint.x, __mouseCapturePoint.y);
			}
			else
			{
				// Restore cursor
				WarpMouse(__mouseCapturePoint.x, __mouseCapturePoint.y);
				ShowCursor(TRUE);
			}

			__mouseCaptured = captured;
		}
	}

	bool Platform::isMouseCaptured()
	{
		return __mouseCaptured;
	}

	void Platform::setCursorVisible(bool visible)
	{
		if (visible != __cursorVisible)
		{
			ShowCursor(visible ? TRUE : FALSE);
			__cursorVisible = visible;
		}
	}

	bool Platform::isCursorVisible()
	{
		return __cursorVisible;
	}

	void Platform::displayKeyboard(bool display)
	{
		// Do nothing.
	}

	bool Platform::isGestureSupported(Gesture::GestureEvent evt)
	{
		return false;
	}

	void Platform::registerGesture(Gesture::GestureEvent evt)
	{
	}

	void Platform::unregisterGesture(Gesture::GestureEvent evt)
	{
	}

	bool Platform::isGestureRegistered(Gesture::GestureEvent evt)
	{
		return false;
	}

	void Platform::pollGamepadState(Gamepad *gamepad) { }

	std::string Platform::displayFileDialog(size_t mode, const char* title, const char* filterDescription, const char* filterExtensions, const char* initialDirectory)
	{
		return "";
	}

	void Platform::shutdownInternal()
	{
		Game::getInstance()->shutdown();
	}

	bool Platform::launchURL(const char *url)
	{
		if (url == NULL || *url == '\0')
			return false;

		// Success when result code > 32
		int len = MultiByteToWideChar(CP_ACP, 0, url, -1, NULL, 0);
		wchar_t* wurl = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, url, -1, wurl, len);
		int r = (int)ShellExecute(NULL, NULL, wurl, NULL, NULL, SW_SHOWNORMAL);
		SAFE_DELETE_ARRAY(wurl);
		return (r > 32);
	}

}

#endif