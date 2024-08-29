
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
    IncludeDir["curl"] = "PicoMusicControllerApp/external/curl/include"
    IncludeDir["nlohmann"] = "PicoMusicControllerApp/external/nlohmann"
    IncludeDir["crow"] = "PicoMusicControllerApp/external/crow"


    LibDir = {}
    LibDir["curl"] = "PicoMusicControllerApp/external/curl/lib"


--[[************************************************************]]
--[[**                     Main App                           **]]
--[[**                                                        **]]
--[[**                                                        **]]
--[[************************************************************]]

    project "PicoMusicControllerApp"
        location "PicoMusicControllerApp"
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
            "{COPY} libcurl.dll %{cfg.targetdir}",
            "{COPY} zlib1.dll %{cfg.targetdir}",
            "{COPY} ../../SpotifyDevCredentials.txt ../bin/",
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
