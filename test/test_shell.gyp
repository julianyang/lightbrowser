{
    'targets' : [
    {
        'target_name' : 'test_shell',
        'type' : 'executable',
        'cflags': ['-g', '-Wno-narrowing', '-Wsign-compare', '-fno-threadsafe-statics' , '-fvisibility-inlines-hidden', '-fPIC', '-fno-rtti'],
        'includes' : [
            '../src/chromium.gyp',
        ],
        'ldflags' : [
            '-L<(CHROMIUM_SRC)/out/Release/lib.target',
            #'-Wl,-rpath=<(CHROMIUM_SRC)/out/Release/lib.target',
        ],
        'sources' : [
            'test_shell_main.cpp',
        ],
        'dependencies' : [
            '../src/lightBrowser.gyp:*',
        ],
        'link_settings' : {
            'libraries' : [
                '-lglue',
                '-lbase',
                '-lbase_i18n',
                '-lprinting',
                '-lcc',
                '-lcrcrypto',
                '-ldbus',
                '-lffmpeg',
                '-lgles2_c_lib',
                '-lgles2_implementation',
                '-lgles2_utils',
                '-lgl_wrapper',
                '-ltranslator_glsl',
                '-lppapi_shared',
                '-lgoogleurl',
                '-lgpu',
                '-licui18n',
                '-licuuc',
                '-lipc',
                '-lnative_theme',
                '-lnet',
                '-lwebkit_base',
                '-lwebkit_gpu',
                '-lwebkit',
                '-lwebkit_storage',
                '-lv8',
                '-luser_agent',
                '-lskia',
                '-lshared_memory_support',
                '-lsql',
                '-lmedia',
                '-lui',
                '-lsurface',
                '-luser_agent',
                '-lpixman-1',
            ]
        },
        'include_dirs' : [
            '../',
            '../include',
        ],
    },
    ]
}
