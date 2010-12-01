AC_INIT([temperature], [0.0.1], [minovotn@redhat.com])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_PROG_CC
AM_PROG_CC_C_O
AC_CHECK_LIB([xml2], [xmlParseFile], [], AC_MSG_ERROR([You need libxml to compile this utility]))
AC_CHECK_LIB([pci], [pci_scan_bus], [])
AC_CHECK_HEADERS([libxml/parser.h])
if test "$HAVE_LIBXML_PARSER_H" = ""; then
  AC_CHECK_PROGS([PKGCONFIG], [pkg-config], [:])
  if test "$PKGCONFIG" = :; then
    AC_MSG_ERROR("Cannot find libxml2.0 headers on path and pkg-config is not found")
  fi
  AM_CONDITIONAL([USE_PKGCONFIG], [true])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
