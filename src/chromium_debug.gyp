{
    'variables' : {
        'CHROMIUM_SRC' : '/data/chrome/tarball/chromium.25.0.1364.172/home/src_tarball/tarball/chromium/src',
    },
    'defines' : [
        '_FILE_OFFSET_BITS=64',
	'CHROMIUM_BUILD',
	'USE_DEFAULT_RENDER_THEME=1',
	'USE_LIBJPEG_TURBO=1',
	'USE_NSS=1',
	'ENABLE_ONE_CLICK_SIGNIN',
	'GTK_DISABLE_SINGLE_INCLUDES=1',
	'ENABLE_REMOTING=1',
	'ENABLE_WEBRTC=1',
	'ENABLE_PEPPER_THREADING',
	'ENABLE_CONFIGURATION_POLICY',
	'ENABLE_INPUT_SPEECH',
	'ENABLE_NOTIFICATIONS',
	'ENABLE_GPU=1',
	'ENABLE_EGLIMAGE=1',
	'USE_SKIA=1',
	'ENABLE_TASK_MANAGER=1',
	'ENABLE_WEB_INTENTS=1',
	'ENABLE_EXTENSIONS=1',
	'ENABLE_PLUGIN_INSTALLATION=1',
	'ENABLE_PLUGINS=1',
	'ENABLE_SESSION_SERVICE=1',
	'ENABLE_THEMES=1',
	'ENABLE_BACKGROUND=1',
	'ENABLE_AUTOMATION=1',
	'ENABLE_GOOGLE_NOW=1',
	'ENABLE_LANGUAGE_DETECTION=1',
	'ENABLE_PRINTING=1',
	'ENABLE_CAPTIVE_PORTAL_DETECTION=1',
	'UNIT_TEST',
	'GL_GLEXT_PROTOTYPES',
	'U_USING_ICU_NAMESPACE=0',
	'U_STATIC_IMPLEMENTATION',
	'GTEST_HAS_RTTI=0',
	'SK_BUILD_NO_IMAGE_ENCODE',
	'SK_DEFERRED_CANVAS_USES_GPIPE=1',
	'GR_GL_CUSTOM_SETUP_HEADER="GrGLConfig_chrome.h"',
	'GR_AGGRESSIVE_SHADER_OPTS=1',
	'SK_USE_POSIX_THREADS',
	'__STDC_CONSTANT_MACROS',
	'__STDC_FORMAT_MACROS',
	'DYNAMIC_ANNOTATIONS_ENABLED=1',
	'WTF_USE_DYNAMIC_ANNOTATIONS=1',
	'_DEBUG'
    ],

    'include_dirs' : [
        '<(CHROMIUM_SRC)',
        '<(CHROMIUM_SRC)/v8/include',
        '<(CHROMIUM_SRC)/third_party/angle/include',
        '<(CHROMIUM_SRC)/third_party/skia/include',
        '<(CHROMIUM_SRC)/third_party/skia/include/config',
        '<(CHROMIUM_SRC)/third_party/skia/include/core',
        '<(CHROMIUM_SRC)/third_party/WebKit/Source/Platform/chromium',
    ],
}
