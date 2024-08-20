
workspace "PicoMusicController"
    architecture "x64"
    startproject "App"

    configurations
    {
        "Debug",
        "Release"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    IncludeDir = {} 
    IncludeDir["curl"] = "PicoMusicController/external/curl/include"
    IncludeDir["nlohmann"] = "PicoMusicController/external/nlohmann"
    IncludeDir["crow"] = "PicoMusicController/external/crow"


    LibDir = {}
    LibDir["curl"] = "PicoMusicController/external/curl/lib"


--[[************************************************************]]
--[[**                     Main App                           **]]
--[[**                                                        **]]
--[[**                                                        **]]
--[[************************************************************]]

    project "PicoMusicController"
        location "PicoMusicController"
        kind "WindowedApp"
        language "C++"
        cppdialect "C++20"
        
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        files
        {
            "%{prj.name}/source/**.h",
		    "%{prj.name}/source/**.cpp",
        }

        includedirs
        {
            "%{prj.name}/source",
            "%{IncludeDir.curl}",
            "%{IncludeDir.nlohmann}",
            "%{IncludeDir.crow}",
        }

        libdirs
        {
           "%{LibDir.curl}",
        }
    
        links
        {
            "libcurl.lib"
        }

    filter "configurations:Debug"
        defines "_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "_RELEASE"
        runtime "Release"
        optimize "On"
