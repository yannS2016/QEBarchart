/*  QEScalarHistogramManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_SCALAR_BARCHART_MANAGER_H
#define QE_SCALAR_BARCHART_MANAGER_H

#include <QEDesignerPluginCommon.h>
#include <QEPluginLibrary_global.h>

// QEScalarBarchart plugin interface.
//
class QEPLUGINLIBRARYSHARED_EXPORT QEScalarBarchartManager :
      public QObject, public QDesignerCustomWidgetInterface {

   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)

   QE_DECLARE_PLUGIN_MANAGER (QEScalarBarchart)
};

#endif                          // QE_SCALAR_BARCHART_MANAGER_H
