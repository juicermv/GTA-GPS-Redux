--! NUMAKE PROJECT
workspace:set("mingw", workspace.arguments["mingw"])

local project
local PLUGIN_SDK_DIR

if workspace.platform == "windows" and workspace:get("mingw") ~= "true" then
    PLUGIN_SDK_DIR = workspace:download_zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/msvc.zip")
    project = workspace:create_msvc_target("SA-GPS-Redux")
    project.arch = "x86"

    project.libraries = {
        "plugin.lib",
        "d3d9.lib",
        "d3dx9.lib"
    }

    project.compiler_flags = {
        "/Ot",
        "/Ob1",
        "/std:c++17",
        "/GT",
        "/fp:fast",
        "/W3",
        "/Gy",
        "/MT",
        "/Oi"
    }

    project.linker_flags = {
        "/SUBSYSTEM:WINDOWS",
        "/OPT:ICF",
        "/OPT:REF",
        "/LTCG",
        "/DLL"
    }
else
    PLUGIN_SDK_DIR = workspace:download_zip("https://github.com/juicermv/plugin-sdk/releases/latest/download/mingw.zip")
    project = workspace:create_mingw_target("SA-GPS-Redux")
    project.arch = "i686"

    project.libraries = {
        "d3d9",
        "d3dx9",
        "plugin"
    }

    project.compiler_flags = {
        "-std=c++17",
        "-Ofast",
        "-fpermissive",
        "-shared",
        "-g",
        "-flto"
    }

    project.linker_flags = {
        "--dll",
        "--subsystem,windows",
        "--export-all-symbols"
    }
end

project.assets = {
    ["README.md"] = "README.md",
    ["GPSLine/SA.GPS.CONF.ini"] = "SA.GPS.CONF.ini"
}

project.output = "SA.GPS.REDUX.asi"
project.library_paths = {
    PLUGIN_SDK_DIR .. "/lib/plugin_sa/",
    "GPSLine"
}

project.include_paths = {
    PLUGIN_SDK_DIR,
    PLUGIN_SDK_DIR .. "/plugin_sa/",
    PLUGIN_SDK_DIR .. "/plugin_sa/game_sa/",
    PLUGIN_SDK_DIR .. "/shared/",
    PLUGIN_SDK_DIR .. "/shared/game/",
    "external/mini/src",
    "GPSLine"
}

project.definitions = {
    --"_NDEBUG",
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
    "PLUGIN_SGV_10US"
}

project.files = workspace:walk_dir("GPSLine", false, {"cpp"})

workspace:register_target(project)
