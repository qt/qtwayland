TEMPLATE=subdirs
CONFIG += ordered

module_qtwayland_src.subdir = src
module_qtwayland_src.target = module-qtwayland-src

module_qtwayland_examples.subdir = examples
module_qtwayland_examples.target = module-qtwayland-examples
module_qtwayland_examples.depends = module_qtwayland_src
!contains(QT_BUILD_PARTS,examples) {
    module_qtwayland_examples.CONFIG = no_default_target no_default_install
}

module_qtwayland_tests.subdir = tests
module_qtwayland_tests.target = module-qtwayland-tests
module_qtwayland_tests.depends = module_qtwayland_src
!contains(QT_BUILD_PARTS,tests):!with-tests {
    module_qtwayland_tests.CONFIG += no_default_target
}

SUBDIRS += module_qtwayland_src \
           module_qtwayland_examples \
           module_qtwayland_tests
