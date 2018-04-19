#ifndef VVGL_QT_GLOBAL_H
#define VVGL_QT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VVGL_LIBRARY)
#  define VVGLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VVGLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VVGL_QT_GLOBAL_H
