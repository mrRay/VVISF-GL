#ifndef VVGL_QT_GLOBAL_H
#define VVGL_QT_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QCoreApplication>

#if defined(VVGL_LIBRARY)
#  define VVGL_EXPORT Q_DECL_EXPORT
#else
#  define VVGL_EXPORT Q_DECL_IMPORT
#endif

#endif // VVGL_QT_GLOBAL_H
