set_project("lcc2ply")

add_rules("mode.debug", "mode.release")
add_requires("jsoncpp", "cli11")

target("lcc2ply")
    set_languages("cxx20")
    set_kind("binary")
    set_warnings("all", "error", "extra", "pedantic")

    add_includedirs("include")
    add_files("src/**.cpp")
    add_packages("jsoncpp", "cli11")

    if is_plat("windows") then
        add_cxflags("/utf-8", {force = true})
        add_cxxflags("/utf-8", {force = true})
        add_cxxflags("/wd4068", {force = true}) -- for #pragma clang
    end

    after_build(function (target)
        os.cp(target:targetfile(), "bin/")
    end)
target_end()
