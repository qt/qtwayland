TEMPLATE=subdirs

SUBDIRS += client
qtHaveModule(waylandcompositor): SUBDIRS += compositor
