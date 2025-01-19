local PLUGIN_SDK_DIR = network:zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/msvc.zip")
local SIMDStringX86 = network:zip("https://github.com/juicermv/SIMDString/releases/download/release/release.zip").."\\SIMDStringX86"


local project = Project(
    "GPS Redux",
    "C++",
    "SA.GPS.REDUX.asi",
    filesystem:walk("src", false, {"cpp"}),
    {
        ["README.md"] = "README.md",
        ["SA.GPS.CONF.ini"] = "SA.GPS.CONF.ini"
    },
    {
        PLUGIN_SDK_DIR .. "/src/",
        PLUGIN_SDK_DIR .. "/src/plugin_sa/",
        PLUGIN_SDK_DIR .. "/src/plugin_sa/game_sa/",
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
        "_USE_MATH_DEFINES"
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
    nil,
    "x86",
    "DynamicLibrary"
)

tasks:create("build_msvc", function() 
    msvc:build(project)
end)