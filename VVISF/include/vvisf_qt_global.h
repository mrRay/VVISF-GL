#ifndef VVISF_GLOBAL_H
#define VVISF_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VVISF_LIBRARY)
#  define VVISFSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VVISFSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VVISF_GLOBAL_H
