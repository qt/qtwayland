if (TARGET PkgConfig::XComposite)
    set(XComposite_FOUND 1)
    return()
endif()

find_package(PkgConfig QUIET)

pkg_check_modules(XComposite xcomposite IMPORTED_TARGET)

if (NOT TARGET PkgConfig::XComposite)
    set(XComposite_FOUND 0)
endif()
