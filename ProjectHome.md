A fork of the [Angle project](http://code.google.com/p/angleproject/), adding fullscreen support and extending the es utilities a little.

Fullscreen support means that you can enable your applications to support [Nvidia 3D-Vision](http://www.nvidia.com/object/3d-vision-main.html) while staying with OpenGL ES code instead of DirectX! Nvidia supports OpenGL in conjunction with Nvidia 3D-Vision only on costly Quadro cards. This fork allows you to use 3D-Vision on consumer cards as well (within the limits of OpenGL ES 2.0).

All newly added code is also under the New BSD license, use it for whatever you want :)

Here's what's different from the upstream Angle project.

It includes all the projects of the original repository, slightly modified were needed. I
had to change parts of EGL and also took the freedom to extend the es-
utils a little. Here's what i changed in EGL.
  * egl.h has a new attribute called EGL\_FULLSCREEN\_ANGLE (constant 0x304f, out of the reserved attribute range). This is to be used as any other attribute like EGL\_GREEN\_SIZE etc. in all the EGLConfig related methods. Not exactly optimal but i think it's OK given the usage scenarios of Angle.
  * Config has a new member called mFullscreen (type EGLBoolean). It stores whether the configuration is a fullscreen configuration or not(who'd have guessed...)
  * Config::Config() and Config::set() have been changed accordingly to include an argument specifying whether the config is a fullscreen config or not.
  * ConfigSet::add() was also extended to include the fullscreen flag.
  * ConfigSet::getConfigs() also takes EGL\_FULLSCREEN\_ANGLE into account when enumerating which configurations fit the specified attribute list.
  * Display::getConfigsAttrib() can cope with the new EGL\_FULLSCREEN\_ANGLE attribute as well, returning 1 (set) or 0 (not set).
  * Display::initialize() usually enumerates and stores all configurations consisting of combinations of the different D3DFORMAT renderTargetFormats and depthStencilFormats. For each combination I also check if it is supported in fullscreen and add an additional Config with the mFullscreen flag set to true. E.g. render target format = D3DFMT\_X8R8G8B8 and depth/stencil format = D3DFMT\_D24S8 would create two configurations internally, one for windowed (Config.mFullscreen = 0) and one for fullscreen (Config.mFullscreen = 1).
  * When fetching compatible configurations for an attribute list eglChooseConfig uses ConfigSet:getConfigs() internally which will check all the configurations stored in the Display instance. Fullscreen configurations will be ignored when EGL\_FULLSCREEN\_ANGLE was not specified.
  * Surface::resetSwapChain has been modified as well. In case the Surface was created with a fullscreen Config it will reset the default fullscreen swap chain. In case of a windowed Config it will use the old code path. I created two new methods called Surface::createFullScreen() and Surface::createWindowed() which do the dirty work of reseting/creating the swap chain. Note that Surface::createFullScreen will ignore the configurations backbuffer width and height and use the default display dimensions instead. This might be a big bozo but works so far.
  * Surface::checkForOutOfDateSwapChain() has also been modified to use the default displays resolution as the client area in case of a fullscreen Config.

That's basically all that was necessary in EGL to get fullscreen support and hence 3D-Vision support. So far i didn't encounter any problems and i think the way i approached it is mostly OK. I can't however guarantee that it won't explode in your face in more exotic circumstances. My use case at the moment is to use an extended version of the es-utils to create a (fullscreen) window and perform the message dispatching. How switching between fullscreen and windowed mode works out i don't know cause i didn't test it. Destroying the Surface and recreating it should work though.

Here's what i changed in the es-utils:
  * the ESContext struct has a couple of new callbacks: resizeFunc, called on a window resize event, mouseFunc, called on a mouse(wheel) event and quitFunc, called before the application is terminated. The use of those should be fairly straight forward. I also extended keyFunc a little similar to how mouseFunc works.
  * I added ES\_WINDOW\_FULLSCREEN to be used like the other ES\_WINDOW\_XXX defines in conjunction with esCreateWindow(). When specified in the flags parameter of that function the function will add a EGL\_FULLSCREEN\_ANGLE attribute to the attribute list it hands over to eglCreateWindowSurface() and other egl methods in CreateEGLContext.
  * esCreateWindow will ignore the given width and height in case ES\_WINDOW\_FULLSCREEN was specified and use the current default displays screen resolution instead.
  * ESWindowProc has been beefed up a little so that the new callback functions aren't useless :)

That's basically it. The fork is based on revision [r498](https://code.google.com/p/angle-fullscreen/source/detail?r=498), so is pretty up to date. A few things i had to comment out cause i couldn't get them to compile (and i didn't have the time to figure it out).

  * debug.cpp includes commons/debug.h. For some reason VC complains that it can't find that header file. It's probably just a little fuck up in the project settings. Didn't look to hard as nothing was used from the header.

Final notes:
  * you have to make sure to call eglTerminate or at least destroy the context/surface when running in fullscreen + Nvidia 3D-Vision. It seems the driver isn't capable of restoring the original desktop display config when you perform a hard quit.
  * While i tested this already and it seems to work as expected i can't give an absolute guarantee that it won't kill anything. Use at your own risk.

Keeping up with upstream shouldn't be a big deal as the modifications are minimal and mostly contained in Display.cpp and Surface.cpp. I'll try to keep up with the mainline code as good as possible. Inclusion in the official project would of course be welcome but i guess for that it's to experimental.