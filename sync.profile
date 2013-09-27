%modules = ( # path to module name map
    "QtCompositor" => "$basedir/src/compositor",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%deprecatedheaders = (
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
    "qtbase" => "bdfbc9e493b4df7da89fb62222dc7cb283b3794a",
    "qtjsbackend" => "refs/heads/stable",
    "qtdeclarative" => "refs/heads/stable",
);
