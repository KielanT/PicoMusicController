
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
        language "C++"
        cppdialect "C++20"
        
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        defines { "_WIN32_WINNT=0x0601" }

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

        postbuildcommands 
        {
            "{COPY} ../PicoMusicController/libcurl.dll %{cfg.targetdir}",
            "{COPY} ../PicoMusicController/zlib1.dll %{cfg.targetdir}",
        }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines "_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        kind "WindowedApp"
        defines "_RELEASE"
        runtime "Release"
        optimize "On"
