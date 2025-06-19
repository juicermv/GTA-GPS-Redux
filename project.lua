local PLUGIN_SDK_DIR = network:zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/msvc.zip")
local SIMDStringX86 = network:zip("https://github.com/juicermv/SIMDString/releases/download/release/msvc.zip").."/SIMDStringX86"

local project = Project(
    "GPS Redux",
    "C++",
    "SA.GPS.REDUX.asi",
    filesystem:walk("src", true, {"cpp"}),
    {
        ["README.md"] = "README.md",
        ["SA.GPS.CONF.ini"] = "SA.GPS.CONF.ini"
    },
    {
        PLUGIN_SDK_DIR .. "/src/",
        PLUGIN_SDK_DIR .. "/src/plugin_sa/",
        PLUGIN_SDK_DIR .. "/src/plugin_sa/game_sa/",
        PLUGIN_SDK_DIR .. "/src/plugin_sa/game_sa/rw/",
        PLUGIN_SDK_DIR .. "/src/shared/",
        PLUGIN_SDK_DIR .. "/src/shared/game/",
        "external/mini/src",
        SIMDStringX86,
        "src"
    },
    {
        PLUGIN_SDK_DIR .. "/lib/plugin_sa/",
        "external/d3dx9",
        SIMDStringX86
    },
    {
        "plugin.lib",
        "d3d9.lib",
        "d3dx9.lib",
        "simdstring.lib"
    },
    {
        "_NDEBUG",
        "_CRT_SECURE_NO_WARNINGS",
        "_CRT_NON_CONFORMING_SWPRINTFS",
        "GTASA", 
        "GTAGAME_NAME=\"San Andreas\"", 
        "GTAGAME_ABBR=\"SA\"", 
        "GTAGAME_ABBRLOW=\"sa\"", 
        "GTAGAME_PROTAGONISTNAME=\"CJ\"", 
        "GTAGAME_CITYNAME=\"San Andreas\"", 
        "_LA_SUPPORT", 
        "_DX9_SDK_INSTALLED", 
        "PLUGIN_SGV_10US",
        "_USE_MATH_DEFINES",
        "RW"
    },
    {
        "/Ox",
        "/Ob1",
        "/std:c++latest",
        "/GT",
        "/fp:fast",
        "/W3",
        "/Gy",
        "/MT",
        "/Oi",
        "/EHsc"
    },
    {
        "/SUBSYSTEM:WINDOWS",
        "/OPT:ICF",
        "/OPT:REF",
        "/DLL"
    },
    nil,
    nil,
    "x86",
    "DynamicLibrary"
)

tasks:create("build_mingw", function()
    local PLUGIN_SDK_DIR_MINGW = network:zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/mingw.zip")
    local SIMDStringX86_MINGW = network:zip("https://github.com/juicermv/SIMDString/releases/download/release/mingw.zip")
    project.include_paths = {
        PLUGIN_SDK_DIR_MINGW .. "/src/",
        PLUGIN_SDK_DIR_MINGW .. "/src/plugin_sa/",
        PLUGIN_SDK_DIR_MINGW .. "/src/plugin_sa/game_sa/",
        PLUGIN_SDK_DIR_MINGW .. "/src/plugin_sa/game_sa/rw/",
        PLUGIN_SDK_DIR_MINGW .. "/src/shared/",
        PLUGIN_SDK_DIR_MINGW .. "/src/shared/game/",
        "external/mini/src",
        SIMDStringX86_MINGW,
        "src"
    }
    project.lib_paths = {
        PLUGIN_SDK_DIR_MINGW .. "/lib/plugin_sa/",
        "external/d3dx9",
        SIMDStringX86_MINGW
    }
    project.arch = i686
    project.libs = {
        "d3d9",
        "d3dx9",
        "plugin",
        "SIMDString"
    }

    project.compiler_flags = {
        "-std=gnu++23",
        "-Ofast",
        "-fpermissive",
        "-shared",
        "-g",
    }

    project.linker_flags = {
        "--dll",
        "--subsystem,windows",
        "--export-all-symbols"
    }
    mingw:build(project)
end)

tasks:create("build_msvc", function() 
    msvc:build(project)
end)