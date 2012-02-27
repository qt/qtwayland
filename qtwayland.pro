TEMPLATE=subdirs

module_qtcompositor_src.subdir = src
module_qtcompositor_src.target = module-module_qtcompositor_src

module_qtcompositor_examples.subdir = examples
module_qtcompositor_examples.target = module_qtcompositor_examples
module_qtcompositor_examples.depends = module_qtcompositor_src
!contains(QT_BUILD_PARTS,examples) {
    module_qtcompositor_examples.CONFIG = no_default_target no_default_install
}

SUBDIRS += tests \
           module_qtcompositor_src \
           module_qtcompositor_examples \
