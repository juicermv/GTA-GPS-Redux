--! NUMAKE PROJECT
if workspace.arguments["mingw"] ~= nil then
    workspace:set("mingw", workspace.arguments["mingw"])
end


local project
local PLUGIN_SDK_DIR

if workspace.platform == "windows" and workspace:get("mingw") ~= "true" then
    PLUGIN_SDK_DIR = workspace:download_zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/msvc.zip")
    project = workspace:create_msvc_target("SA-GPS-Redux")
    project.arch = "x86"

    project.libraries = {
        "plugin.lib",
        "d3d9.lib",
        "d3dx9.lib",
        "simdstring.lib"
    }

    project.compiler_flags = {
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
    }

    project.linker_flags = {
        "/SUBSYSTEM:WINDOWS",
        "/OPT:ICF",
        "/OPT:REF",
        "/DLL"
    }
else --- MinGW build has memory access issues when calling DoPathFind. Don't use it.
    PLUGIN_SDK_DIR = workspace:download_zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/mingw.zip")
    project = workspace:create_mingw_target("SA-GPS-Redux")
    project.arch = "i686"

    project.libraries = {
        "d3d9",
        "d3dx9",
        "plugin"
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
end

project.assets = {
    ["README.md"] = "README.md",
    ["SA.GPS.CONF.ini"] = "SA.GPS.CONF.ini"
}

project.output = "SA.GPS.REDUX.asi"
project.library_paths = {
    PLUGIN_SDK_DIR .. "/lib/plugin_sa/",
    "external/d3dx9",
    "external/simdstring"
}

project.include_paths = {
    PLUGIN_SDK_DIR .. "/src/",
    PLUGIN_SDK_DIR .. "/src/plugin_sa/",
    PLUGIN_SDK_DIR .. "/src/plugin_sa/game_sa/",
    PLUGIN_SDK_DIR .. "/src/shared/",
    PLUGIN_SDK_DIR .. "/src/shared/game/",
    "external/mini/src",
    "external/simdstring",
    "src"
}

project.definitions = {
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
}

project.files = workspace:walk_dir("src", false, {"cpp"})

workspace:register_target(project)
