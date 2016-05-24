TEMPLATE=subdirs

SUBDIRS += client \
    qmltestapp
SUBDIRS += compositor
SUBDIRS += testapp

qmltestapp.depends = client

OTHER_FILES += protocol/custom.xml
