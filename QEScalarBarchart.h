/*  QEScalarBarchart.h
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

#ifndef QE_SCALAR_BARCHART_H
#define QE_SCALAR_BARCHART_H

#include <QLayout>
#include <QString>
#include <QVector>

#include <QECommon.h>
#include <QEBarchart.h>
#include <QEFrame.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEPluginLibrary_global.h>

/// The QEScalarBarchart class is a EPICS aware histogram widget.
/// The value of, i.e. the length of each bar of the histogram is controlled by
/// its own process variable.
/// When a variable is defined (connected), the bar length is updated, and
/// optionally the bar colour set to reflect the variable's severity status.
/// The bar is 'grayed-out' when its variable is disconnected (although the bar
/// retains its last known value/length).
/// The histogram nature of the this widget is provided by a QEHistogram widget.
/// The QEScalarBarchart widget is tighly integrated with the base class QEWidget,
/// via QEFrame, which provides generic support such as macro substitutions,
/// drag/drop, and standard properties.
///
// Maximum number of variables.
#define QE_BARCHART_NUMBER_VARIABLES 35

class QEPLUGINLIBRARYSHARED_EXPORT QEScalarBarchart:public QEFrame{
   Q_OBJECT

   // All standard properties are inherited from QEFrame.
   //
   /// Macro substitutions. The default is no substitutions.
   /// The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings.
   /// For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
   /// These substitutions are applied to all the variable names.
   /*Q_PROPERTY (QString variableSubstitutions READ getPvNameSubstitutions WRITE
               setPvNameSubstitutions)*/

public:
   /// \enum ScaleModes
   enum ScaleModes {
      Manual,              ///< Use property minimum/maximum to scale histogram
      Auto,                ///< Dynamically scale based on minimum/maximum displayed value
      OperationalRange     ///< Use process variable operational range (LOPR/HOPR).
   };

   Q_ENUMS (ScaleModes)

   // Histogram properties
   //
   Q_PROPERTY (bool   autoBarGapWidths READ getAutoBarGapWidths WRITE setAutoBarGapWidths)
   Q_PROPERTY (int    barWidth         READ getBarWidth         WRITE setBarWidth)
   Q_PROPERTY (int    gap              READ getGap              WRITE setGap)
   Q_PROPERTY (int    margin           READ getMargin           WRITE setMargin)
   Q_PROPERTY (ScaleModes scaleMode    READ getScaleMode        WRITE setScaleMode)
   Q_PROPERTY (double minimum          READ getMinimum          WRITE setMinimum)
   Q_PROPERTY (double maximum          READ getMaximum          WRITE setMaximum)
   Q_PROPERTY (double baseLine         READ getBaseLine         WRITE setBaseLine)
   Q_PROPERTY (bool   drawAxies        READ getDrawAxies        WRITE setDrawAxies)
   Q_PROPERTY (bool   showScale        READ getShowScale        WRITE setShowScale)
   Q_PROPERTY (bool   showGrid         READ getShowGrid         WRITE setShowGrid)
   Q_PROPERTY (bool   logScale         READ getLogScale         WRITE setLogScale  )
   // When dislayAlarmState set ture, this property value effectively ignored.
   Q_PROPERTY (QColor backgroundColour READ getBackgroundColour WRITE setBackgroundColour)
   Q_PROPERTY (QColor barColour        READ getBarColour        WRITE setBarColour)
   Q_PROPERTY (bool   drawBorder       READ getDrawBorder       WRITE setDrawBorder)
   Q_PROPERTY (Qt::Orientation orientation READ getOrientation  WRITE setOrientation)

   /// EPICS variable names (CA PV).
   // Note variableN uses variable index N-1.
   //
   Q_PROPERTY (QString variable1  READ getPvName0   WRITE setPvName0)
   Q_PROPERTY (QString variable2  READ getPvName1   WRITE setPvName1)
   Q_PROPERTY (QString variable3  READ getPvName2   WRITE setPvName2)
   Q_PROPERTY (QString variable4  READ getPvName3   WRITE setPvName3)
   Q_PROPERTY (QString variable5  READ getPvName4   WRITE setPvName4)
   Q_PROPERTY (QString variable6  READ getPvName5   WRITE setPvName5)
   Q_PROPERTY (QString variable7  READ getPvName6   WRITE setPvName6)
   Q_PROPERTY (QString variable8  READ getPvName7   WRITE setPvName7)
   Q_PROPERTY (QString variable9  READ getPvName8   WRITE setPvName8)
   Q_PROPERTY (QString variable10 READ getPvName9   WRITE setPvName9)
   Q_PROPERTY (QString variable11 READ getPvName10 WRITE setPvName10)
   Q_PROPERTY (QString variable12 READ getPvName11 WRITE setPvName11)
   Q_PROPERTY (QString variable13 READ getPvName12 WRITE setPvName12)
   Q_PROPERTY (QString variable14 READ getPvName13 WRITE setPvName13)
   Q_PROPERTY (QString variable15 READ getPvName14 WRITE setPvName14)
   Q_PROPERTY (QString variable16 READ getPvName15 WRITE setPvName15)
   Q_PROPERTY (QString variable17 READ getPvName16 WRITE setPvName16)
   Q_PROPERTY (QString variable18 READ getPvName17 WRITE setPvName17)
   Q_PROPERTY (QString variable19 READ getPvName18 WRITE setPvName18)
   Q_PROPERTY (QString variable20 READ getPvName19 WRITE setPvName19)
   Q_PROPERTY (QString variable21 READ getPvName20 WRITE setPvName20)
   Q_PROPERTY (QString variable22 READ getPvName21 WRITE setPvName21)
   Q_PROPERTY (QString variable23 READ getPvName22 WRITE setPvName22)
   Q_PROPERTY (QString variable24 READ getPvName23 WRITE setPvName23)
   Q_PROPERTY (QString variable25 READ getPvName24 WRITE setPvName24)
   Q_PROPERTY (QString variable26 READ getPvName25 WRITE setPvName25)
   Q_PROPERTY (QString variable27 READ getPvName26 WRITE setPvName26)
   Q_PROPERTY (QString variable28 READ getPvName27 WRITE setPvName27)
   Q_PROPERTY (QString variable29 READ getPvName28 WRITE setPvName28)
   Q_PROPERTY (QString variable30 READ getPvName29 WRITE setPvName29)
   Q_PROPERTY (QString variable31 READ getPvName30 WRITE setPvName30)
   Q_PROPERTY (QString variable32 READ getPvName31 WRITE setPvName31)
   Q_PROPERTY (QString variable33 READ getPvName32 WRITE setPvName32)
   Q_PROPERTY (QString variable34 READ getPvName33 WRITE setPvName33)
   Q_PROPERTY (QString variable35 READ getPvName34 WRITE setPvName34)


public:
   explicit QEScalarBarchart (QWidget* parent = 0);
   ~QEScalarBarchart () { }

   void setScaleMode (const ScaleModes scaleMode);
   ScaleModes getScaleMode () const;

   // Expose access to the internal widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, double, getMinimum,     setMinimum)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, double, getMaximum,     setMaximum)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, double, getBaseLine,    setBaseLine)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, int,    getGap,         setGap)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, int,    getBarWidth,    setBarWidth)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, int,    getMargin,      setMargin)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, bool,   getAutoBarGapWidths, setAutoBarGapWidths)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, bool,   getShowScale,  setShowScale)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, bool,   getShowGrid,   setShowGrid)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, bool,   getLogScale,    setLogScale)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, bool,   getDrawAxies,  setDrawAxies)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, bool,   getDrawBorder,  setDrawBorder)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, QColor, getBackgroundColour, setBackgroundColour)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, QColor, getBarColour,   setBarColour)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (barchart, Qt::Orientation, getOrientation, setOrientation)

protected:
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);
   bool eventFilter (QObject* obj, QEvent* event);

   // Adds the specified pvName to the first unused slot (if room) and
   // establish the connection.
   //
   void addPvName (const QString& pvName);

private:
   void setPvNameSubstitutions (const QString& subs);
   QString getPvNameSubstitutions () const;
   void updatebarchartScale();
   void setReadOut (const QString& text);
   void genReadOut (const int index);

   QEBarchart* barchart;
   QHBoxLayout* layout;         // holds the barchart - any layout type will do
   QCaVariableNamePropertyManager vnpm [QE_BARCHART_NUMBER_VARIABLES];
   QEFloatingFormatting floatingFormatting;
   int selectedChannel;         //
   ScaleModes mScaleMode;
   bool isFirstUpdate [QE_BARCHART_NUMBER_VARIABLES];

private slots:
   void newVariableNameProperty (QString pvName, QString subs, unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int&variableIndex);

   void setChannelValue (const double&value, QCaAlarmInfo&,
                         QCaDateTime&, const unsigned int&);

private:
   // Define a variable access
   // Note, the Q_PROPERTY declaration itself can't be in a macro.
   //
#define VARIABLE_PROPERTY_ACCESS(VAR_INDEX)                                    \
   void setPvName##VAR_INDEX (const QString& pvName) {                         \
   this->vnpm [VAR_INDEX].setVariableNameProperty (pvName);                    \
}                                                                              \
                                                                               \
   QString getPvName##VAR_INDEX () const {                                     \
   return this->vnpm [VAR_INDEX].getVariableNameProperty ();                   \
}


   VARIABLE_PROPERTY_ACCESS (0)
   VARIABLE_PROPERTY_ACCESS (1)
   VARIABLE_PROPERTY_ACCESS (2)
   VARIABLE_PROPERTY_ACCESS (3)
   VARIABLE_PROPERTY_ACCESS (4)
   VARIABLE_PROPERTY_ACCESS (5)
   VARIABLE_PROPERTY_ACCESS (6)
   VARIABLE_PROPERTY_ACCESS (7)
   VARIABLE_PROPERTY_ACCESS (8)
   VARIABLE_PROPERTY_ACCESS (9)
   VARIABLE_PROPERTY_ACCESS (10)
   VARIABLE_PROPERTY_ACCESS (11)
   VARIABLE_PROPERTY_ACCESS (12)
   VARIABLE_PROPERTY_ACCESS (13)
   VARIABLE_PROPERTY_ACCESS (14)
   VARIABLE_PROPERTY_ACCESS (15)
   VARIABLE_PROPERTY_ACCESS (16)
   VARIABLE_PROPERTY_ACCESS (17)
   VARIABLE_PROPERTY_ACCESS (18)
   VARIABLE_PROPERTY_ACCESS (19)
   VARIABLE_PROPERTY_ACCESS (20)
   VARIABLE_PROPERTY_ACCESS (21)
   VARIABLE_PROPERTY_ACCESS (22)
   VARIABLE_PROPERTY_ACCESS (23)
   VARIABLE_PROPERTY_ACCESS (24)
   VARIABLE_PROPERTY_ACCESS (25)
   VARIABLE_PROPERTY_ACCESS (26)
   VARIABLE_PROPERTY_ACCESS (27)
   VARIABLE_PROPERTY_ACCESS (28)
   VARIABLE_PROPERTY_ACCESS (29)
   VARIABLE_PROPERTY_ACCESS (30)
   VARIABLE_PROPERTY_ACCESS (31)
   VARIABLE_PROPERTY_ACCESS (32)
   VARIABLE_PROPERTY_ACCESS (33)
   VARIABLE_PROPERTY_ACCESS (34)
#undef VARIABLE_PROPERTY_ACCESS

};

#endif                          // QE_SCALAR_BARCHART_H
