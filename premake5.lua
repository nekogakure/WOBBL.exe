workspace "WOBBL.exe"
    configurations { "Debug", "Release" }

project "WOBBL"
    kind "WindowedApp"
    language "C"
    targetdir "bin/%{cfg.buildcfg}"

    files { "src/**.c", "include/**.h" }

    includedirs { "include" }

    links { "gdi32", "gdiplus" }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"