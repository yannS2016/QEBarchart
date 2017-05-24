/*  QEScalarBarchart.cpp
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


#include <QDebug>
#include <QECommon.h>
#include <QCaObject.h>
#include <QEScalarBarchart.h>

#define DEBUG  qDebug () << "QEScalarBarchart" << __FUNCTION__ << __LINE__

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEScalarBarchart::QEScalarBarchart (QWidget * parent) : QEFrame (parent)
{
   // Set default property values
   // Super class....
   //
  /* this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);
   this->setVariableAsToolTip (true);*/

   // Create internal widget.
   //
   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->barchart = new QEBarchart (this);
   this->layout->addWidget (this->barchart);

   // Set barchart properties.
   //
   this->barchart->setFrameShape (QFrame::Panel);
   this->barchart->setFrameShadow (QFrame::Plain);
   this->barchart->setAutoScale (true);

   this->barchart->setMouseTracking (true);
   this->barchart->installEventFilter (this);

   this->mScaleMode = Manual;

   for (int j = 0; j < ARRAY_LENGTH (this->isFirstUpdate); j++) {
      this->isFirstUpdate [j] = true;
   }

   // Set up data
   //
   this->setNumVariables (ARRAY_LENGTH (this->vnpm));

   // Set up default properties
   //
   this->setAllowDrop (false);

   // Use default context menu.
   //
   this->setupContextMenu ();
   this->selectedChannel = -1;

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->vnpm); j++) {
      this->vnpm [j].setVariableIndex (j);
      QObject::connect (&this->vnpm [j], SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)),
                        this,            SLOT   (newVariableNameProperty  (QString, QString, unsigned int)));
   }
}


//------------------------------------------------------------------------------
// This is the slot used to recieve new PV information.
//
void QEScalarBarchart::newVariableNameProperty (QString pvName, QString subs, unsigned int pvi)
{
   this->barchart->clearValue ((int) pvi);
   this->setVariableNameAndSubstitutions (pvName, subs, pvi);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a barchart floating point numbers are required.
//
qcaobject::QCaObject* QEScalarBarchart::createQcaItem (unsigned int pvi)
{
   qcaobject::QCaObject* result = NULL;

   if (pvi < ARRAY_LENGTH (this->vnpm)) {
      QString pvName = this->getSubstitutedVariableName (pvi);
      result = new QEFloating (pvName, this, &this->floatingFormatting, pvi);
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
//
void QEScalarBarchart::establishConnection (unsigned int variableIndex)
{
   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca, SIGNAL (floatingChanged (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (setChannelValue (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int &)));

      // Also set/reset value.
      // Note: this also creates the underlying entry with the barchart widget.
      // Usefull for indexOfPosition calls before first PV update or missing PVs.
      //
      this->barchart->setValue ((int) variableIndex, 0.0);
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEScalarBarchart::connectionChanged (QCaConnectionInfo & connectionInfo,
                                           const unsigned int & variableIndex)
{
   bool pvConnected;

   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state
   //
   pvConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (pvConnected, variableIndex);

   // This is a multi PV widget.
   // Do not use updateConnectionStyle.
   //
   // If this is a disconnect - set gray.
   // If this is a connect, we will soon change from gray to required colour.
   //
   this->barchart->setColour ((int) variableIndex, QColor (0xe8e8e8));

   // More trob. than it's worth to check if this is a connect or disconnect.
   //
   this->isFirstUpdate [variableIndex] = true;
}

//------------------------------------------------------------------------------
//
void QEScalarBarchart::updatebarchartScale ()
{
   qcaobject::QCaObject* qca = NULL;
   double lopr;
   double hopr;

   switch (this->mScaleMode) {

      case Manual:
         this->barchart->setAutoScale (false);
         break;

      case Auto:
         this->barchart->setAutoScale (true);
         break;

      case OperationalRange:
         lopr = 0.0;
         hopr = 0.0;
         for (int j = 0; j < QE_BARCHART_NUMBER_VARIABLES; j++) {
            qca = this->getQcaItem (j);
            if (qca) {
               double lowLim = qca->getDisplayLimitLower ();
               double upLim = qca->getDisplayLimitUpper ();

               // Has the operating range been defined??
               //
               if ((lowLim != 0.0) || (upLim != 0.0)) {
                  // Yes - incorporate into overall operating range.
                  //
                  lopr = MIN (lopr, lowLim);
                  hopr = MAX (hopr, upLim);
               }
            }
         }

         // Has at least one PV specified a valid range?
         //
         if ((lopr != 0.0) || (hopr != 0.0)) {
            // Yes - use the range.
            //
            this->barchart->setMinimum (lopr);
            this->barchart->setMaximum (hopr);
            this->barchart->setAutoScale (false);
         }
         // else just leave as is.

         break;
   }
}



//------------------------------------------------------------------------------
// Update the barchart bar value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QEScalarBarchart::setChannelValue (const double& value,
                                         QCaAlarmInfo& alarmInfo,
                                         QCaDateTime&,
                                         const unsigned int &variableIndex)
{
   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   QColor colour;

   if (this->getDisplayAlarmState ()) {
      colour = this->getColor (alarmInfo, 255);
   } else {
      colour = this->barchart->getBarColour ();
   }

   this->barchart->setColour ((int) variableIndex, colour);
   this->barchart->setValue ((int) variableIndex, value);

   // First update (for this connection).
   //
   if (this->isFirstUpdate [variableIndex]) {
      this->updatebarchartScale ();
   }

   // Don't invoke common alarm handling processing.
   // Invoke for tool tip processing directly.
   //
   this->updateToolTipAlarm (alarmInfo.severityName (), variableIndex);
}

//------------------------------------------------------------------------------
//
void QEScalarBarchart::addPvName (const QString& pvName)
{
   for (int slot = 0; slot < ARRAY_LENGTH (this->vnpm); slot++) {
      QString currentPvName = this->getSubstitutedVariableName (slot);
      if (currentPvName.isEmpty ()) {
         // Found an empty slot.
         //
         this->setVariableName (pvName, slot);
         this->establishConnection (slot);
         break;
      }
   }
}

//------------------------------------------------------------------------------
//
void QEScalarBarchart::setPvNameSubstitutions (const QString& pvNameSubstitutions)
{
   // Set all (but read 1).
   //
   for (int j = 0; j < ARRAY_LENGTH (this->vnpm); j++) {
      this->vnpm [j].setSubstitutionsProperty (pvNameSubstitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QEScalarBarchart::getPvNameSubstitutions () const
{
   // All the same - any will do.
   //
   return this->vnpm [0].getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEScalarBarchart::setScaleMode (const ScaleModes scaleModeIn)
{
   this->mScaleMode = scaleModeIn;
   this->updatebarchartScale ();
}

//------------------------------------------------------------------------------
//
QEScalarBarchart::ScaleModes QEScalarBarchart::getScaleMode () const
{
   return this->mScaleMode;
}

//------------------------------------------------------------------------------
//
void QEScalarBarchart::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
// Index is channel number.
//
void QEScalarBarchart::genReadOut (const int index)
{
   qcaobject::QCaObject* qca = NULL;
   QString text;

   if (index >= 0) {
      qca = this->getQcaItem ((unsigned int) index);
   }

   if (qca) {
      bool isDefined;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, value, alarmInfo, timeStamp);

      text = qca->getRecordName ();
      if (isDefined) {
         text.append (" ").append (value.toString ());
         text.append(" ").append (qca->getEgu ());
      } else {
         text.append ("  undefined.");
      }
   } else {
      text = "";
   }
   this->setReadOut (text);
}

//==============================================================================
// Events.
//
bool QEScalarBarchart::eventFilter (QObject *obj, QEvent *event)
{
   const Qt::MouseButtons leftRight= Qt::LeftButton | Qt::RightButton;
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;
   Qt::MouseButton button;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast<QMouseEvent *> (event);
         button = mouseEvent->button();
         if ((obj == this->barchart) && ((button & leftRight) != 0)) {
            // We are about to launch the context menu or drag variable or data.
            // Identify current item (if any).
            //
            QMouseEvent*mouseEvent = static_cast<QMouseEvent *> (event);
            this->selectedChannel = this->barchart->indexOfPosition (mouseEvent->pos ());
            // event has not fully handled - fall through.
         }
         break;

      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->barchart) {
            int index = this->barchart->indexOfPosition (mouseEvent->pos ());
            this->genReadOut (index);
            return true; // event has been handled.
         }
         break;

      default:
         // Just fall through
         break;
   }

   return false; // we did not handle this event.
}

//==============================================================================
// Copy (no paste)
//
/*QString QEScalarBarchart::copyVariable ()
{
   if (this->selectedChannel >= 0) {
      return this->getSubstitutedVariableName ((unsigned int) this->selectedChannel);
   } else {
      return "";
   }
}*/

//------------------------------------------------------------------------------
//
/*QVariant QEScalarBarchart::copyData ()
{
   QVariant result (QVariant::Invalid);
   qcaobject::QCaObject* qca = NULL;

   if (this->selectedChannel >= 0) {
      qca = this->getQcaItem ((unsigned int) this->selectedChannel);
   }

   if (qca) {
      bool isDefined;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, value, alarmInfo, timeStamp);
      if (isDefined) {
         result = value;
      }
   } else {
      result = QVariant ();
   }

   return result;
}*/

//---------------------------------------------------------------------------------
//
/*void QEScalarBarchart::paste (QVariant s)
{
   QStringList pvNameList = QEUtilities::variantToStringList (s);
   for (int j = 0; j < pvNameList.count (); j++) {
      this->addPvName (pvNameList.value (j));
   }
}*/

// end
