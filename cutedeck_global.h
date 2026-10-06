#ifndef CUTEDECK_GLOBAL_H
#define CUTEDECK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CUTEDECK_LIBRARY)
#define CUTEDECK_EXPORT Q_DECL_EXPORT
#else
#define CUTEDECK_EXPORT Q_DECL_IMPORT
#endif

#endif // CUTEDECK_GLOBAL_H
