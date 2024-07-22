add_rules("mode.debug", "mode.release")
add_requires("unity_test", "raylib")

target("c_linked_list")
    set_kind("binary")
    add_files("src/*.c")
    set_languages("c99")
    add_includedirs("include")
    add_packages("raylib")

for _, file in ipairs(os.files("tests/test_*.c")) do
     local name = path.basename(file)
     target(name)
        set_kind("binary")
        set_default(false)
        add_files("tests/" .. name .. ".c")
        add_files("src/*.c|main.c")
        add_tests("default")
        add_packages("unity_test")
        add_includedirs("include")
end