#ifndef DOBOTDLL_GLOBAL_H
#define DOBOTDLL_GLOBAL_H

//#include <QtCore/qglobal.h>

#if defined(DOBOTDLL_LIBRARY)
#  define DOBOTDLLSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DOBOTDLLSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DOBOTDLL_GLOBAL_H
