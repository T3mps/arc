workspace "arc"
    architecture "x64"
    startproject "arc_example"

    configurations { 
        "Debug", 
        "Release", 
        "Distribution" 
    }
    
    flags { 
        "MultiProcessorCompile" 
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    project "arc"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "on"

        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        files {
            "include/**.hpp",
        }

        includedirs {
            "include"
        }

        filter "configurations:Debug"
            runtime "Debug"
            symbols "on"
            optimize "off"
        
        filter "configurations:Release"
            runtime "Release"
            optimize "on"
            symbols "on"
        
        filter "configurations:Distribution"
            runtime "Release"
            optimize "full"
            symbols "off"
