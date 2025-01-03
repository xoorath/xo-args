-- premake5.lua
workspace "xo-args"
    location ("../build/" .. _ACTION)
    configurations { "Debug", "Release" }
    platforms { "x86", "x64" }
    filter "platforms:x86"
        architecture "x86"
    filter "platforms:x64"
        architecture "x64"
    filter {}
    startproject "xo-args-tests"

-- Common settings for all projects
function setupCommonProject(name, lang, filePaths)
    project(name)
        location ("../build/" .. _ACTION .. "/" .. name)

        kind "ConsoleApp"
        language(lang)
        targetdir ("../build/" .. _ACTION .. "/bin/%{cfg.buildcfg}_%{cfg.platform}")

        files(filePaths)

        includedirs {
            "../../include/"
        }

        cdialect("c99")
        cppdialect("c++98")

        warnings "Extra"

        flags { 
            "FatalCompileWarnings" 
        }

        filter "action:vs*"
            defines { "_CRT_SECURE_NO_WARNINGS" }
            clangtidy( "On" )

        filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"

        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"
end

-- Specific projects
setupCommonProject("xo-args-tests", "C", { "../tests/**.h", "../tests/**.c", "../../include/xo-args/xo-args.h" })
setupCommonProject("01-hello-world", "C", { "../../examples/01-hello-world/**.h", "../../examples/01-hello-world/**.c", "../../include/xo-args/xo-args.h" })
setupCommonProject("02-cpp", "C++", { "../../examples/02-cpp/**.h", "../../examples/02-cpp/**.cpp", "../../include/xo-args/xo-args.h" })
setupCommonProject("03-sqlite3", "C", { "../../examples/03-sqlite3/**.h", "../../examples/03-sqlite3/**.c", "../../include/xo-args/xo-args.h" })

project "config-files"
    location("../build/" .. _ACTION)
    kind "Utility"
    files { "../.clang-format", "../scripts/premake5.lua" }

    vpaths {
        ["Config Files"] = { "../.clang-format", "../scripts/premake5.lua" }
    }