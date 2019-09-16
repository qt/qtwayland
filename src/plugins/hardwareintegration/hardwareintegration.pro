TEMPLATE=subdirs

qtHaveModule(waylandclient): SUBDIRS += client
qtHaveModule(waylandcompositor): SUBDIRS += compositor
