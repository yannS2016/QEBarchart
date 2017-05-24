#include <QDebug>
#include <QBrush>
#include <QPen>

#include <QECommon.h>
#include <QEScaling.h>
#include <QEBarchart.h>
#include <QECommon.h>
#include <QEDisplayRanges.h>
#include <QList>
#define DEBUG qDebug () << "QEBarchart (" << __FUNCTION__ << __LINE__ << "): "

// Magic null values - use NaN ?
// 'Unlikely' to occur and can be exactly represented as a double.
//
static const double  NO_DATA_VALUE = -1073741824.0;
static const int monPos[26] = {1,2,3,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,29,30,36,37};
//static const int tickValues[26] = {0,1,10,100,1000,10000,100000,1000000};
//static const char* tickValues[26] = {0,1,10,100,1000,10000,100000,1000000};
static const char* const  tickValue[] = { "0", "1","10","100","1000","10000","100000", "1000000"};
static const QColor  NO_COLOUR_VALUE = QColor (3, 1, 4, 2);   // Pi colour ;-)
int scaleWidths[8]={0,0,0,0,0,0,0,0};

static const double  MINIMUM_SPAN  = +1.0E-18;
static const int     MAX_CAPACITY  = 10000;

//------------------------------------------------------------------------------
//
static bool isNullDataValue (const double x) {
   double d = ABS (x - NO_DATA_VALUE);
   return (d <= 0.001);
}

//------------------------------------------------------------------------------
//
static bool isNullColourValue (const QColor& x) {
   return x == NO_COLOUR_VALUE;
}


//------------------------------------------------------------------------------
//
QEBarchart::QEBarchart (QWidget *parent) : QFrame (parent)
{
   // Set default property values
   // Super class....
   //
   this->setMinimumSize (80, 40);
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // And local properties.
   //
 //  this->mBackgroundColour = Qt::green;   // pale gray
   this->mBackgroundColour = QColor (224, 224, 224);   // pale gray
   this->mBarColour = QColor (255, 255, 255);           // blue
   this->mDrawAxies = true;
   this->mDrawBorder = true;
   this->mAutoScale = false;
   this->mAutoBarGapWidths = true;
   this->mShowScale = true;
   this->mShowGrid = true;
   this->mLogScale = false;

   this->mGap = 5;                // 0 .. 10
   this->mBarWidth = 30;           // 1 .. 80
   this->mMargin = 1;             // 0 .. 20
   this->mBaseLine = 0.0;
   this->mMinimum = 0.0;
   this->mMaximum = 14.0;
   this->mOrientation = Qt::Horizontal;
   this->mXTitle = QString();
   this->mXLabels = QStringList();
   this->mYLabels= QStringList();
   this->mTestSize = 0;
   this->mSpacing = QStringList();

   // Create internal widgets
   //
   this->layout = new QVBoxLayout (this);
   this->layout->setMargin (this->mMargin);
   this->layout->setSpacing (2);

   this->histogramArea = new QWidget (this);
   this->histogramArea->setMouseTracking (true);
   this->layout->addWidget (this->histogramArea);

   this->scrollbar = new QScrollBar (this);
   this->scrollbar->setOrientation (Qt::Horizontal);
   this->scrollbar->setRange (0, 0);
   this->layout->addWidget (this->scrollbar);

   this->dataArray.clear ();
   this->dataArray.reserve (100);
   this->numberDisplayed = 0;
   this->firstDisplayed = 0;
   // Do this only once, not in paintEvent as it causes another paint event.
   //
   this->histogramArea->setAutoFillBackground (false);
   this->histogramArea->setBackgroundRole (QPalette::NoRole);
   QObject::connect (this->scrollbar, SIGNAL (valueChanged (int)),
                     this,     SLOT (scrollBarValueChanged (int))); //when this is true the scrollbar becomes visible


   this->histogramArea->installEventFilter (this);
}
/// the following are some custom methods

//------------------------------------------------------------------------------
//  Define default size for this widget class.
//
QSize QEBarchart::sizeHint () const
{
   return QSize (300, 250);
}

//------------------------------------------------------------------------------
//
void QEBarchart::clearValue (const int index)
{
   this->setValue (index, NO_DATA_VALUE);
}

//------------------------------------------------------------------------------
//
void QEBarchart::clearColour (const int index)
{
   this->setColour (index, NO_COLOUR_VALUE);
}

//------------------------------------------------------------------------------
//
void QEBarchart::clear ()
{
   this->firstDisplayed = 0;
   this->dataArray.clear ();
   this->colourArray.clear ();
   this->update ();
}

//------------------------------------------------------------------------------
//
void QEBarchart::setColour (const int index, const QColor& value)
{
   if (index >= 0 && index < MAX_CAPACITY) {     // sanity check
      while (this->colourArray.count () < index + 1) {
         this->colourArray.append (NO_COLOUR_VALUE);
      }

      this->colourArray [index] = value;

      while ((this->colourArray.count () > 0) &&
             (isNullColourValue (this->colourArray.last ()))) {
         this->colourArray.remove (this->colourArray.count () - 1);
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
void QEBarchart::setValue (const int index, const double value)
{
   if (index >= 0 && index < MAX_CAPACITY) {     // sanity check
      while (this->dataArray.count () < index + 1) {
         this->dataArray.append (NO_DATA_VALUE);
      }

      this->dataArray [index] = value;

      while ((this->dataArray.count () > 0) &&
             (isNullDataValue (this->dataArray.last ()))) {
         this->dataArray.remove (this->dataArray.count () - 1);
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QEBarchart::count () const
{
   return this->dataArray.count ();
}

//------------------------------------------------------------------------------
//
double QEBarchart::value (const int index) const
{
   return this->dataArray.value (index, 0.0);
}

//------------------------------------------------------------------------------
//
void QEBarchart::setValues (const DataArray& values)
{
   this->dataArray = values;

   this->update ();
}

//------------------------------------------------------------------------------
//
QEBarchart::DataArray QEBarchart::values () const
{
   return this->dataArray;
}

//------------------------------------------------------------------------------
//
QColor QEBarchart::getPaintColour (const int index) const
{
   QColor result;

   result = this->colourArray.value (index, NO_COLOUR_VALUE);
   if (isNullColourValue (result)) {
      result = this->mBarColour;
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QEBarchart::scrollMaximum () const
{
    // the number of bar items on present on the chart
    return MAX (0, this->dataArray.count () - this->numberDisplayed);// enable scroll bar when numberDisplayy is the maximum
    // possible at the time on the paintArea;when this happen numberDisplay < dataArray.count();less item than possible

}

//------------------------------------------------------------------------------
//
void QEBarchart::scrollBarValueChanged (int value)
{
   this->firstDisplayed = LIMIT (value, 0, (MAX_CAPACITY - 1));
    DEBUG << this->firstDisplayed;
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEBarchart::firstBarLeft () const
{
   return this->paintArea.left ();
}

//------------------------------------------------------------------------------
//
QRect QEBarchart::fullBarRect  (const int position) const
{
   // paintArea defines overall paint area.
   //
   const int top = this->paintArea.top ();
   const int left = this->firstBarLeft () +
                    (this->useBarWidth + this->useGap + 1) * position;
   const int bottom = this->paintArea.bottom ();
   const int right = left + this->useBarWidth;
   QRect result;

   result.setTop (top);
   result.setLeft (left);
   result.setBottom (bottom);
   result.setRight (right);
   return result;
}
int QEBarchart::indexOfPosition (const int x, const int y) const
{
   // Convert from histogram co-ordinates (which is what the external world sees)
   // to local internal widget co-ordinates.
   //
   const int hax = x - this->histogramArea->geometry ().left ();
   const int hay = y - this->histogramArea->geometry ().top ();

   const int guess = (hax - this->firstBarLeft ()) /
                     MAX (1, this->useBarWidth + this->useGap + 1);

   // Add +/- 2 - very conservative.
   //
   const int lower = MAX (guess - 2, 0);
   const int upper = MIN (guess + 2, this->numberDisplayed - 1);

   int result = -1;

   for (int j = lower; j <= upper; j++) {
      QRect jbar = this->fullBarRect (j);

      if (hax >= jbar.left () && hax <= jbar.right () &&
          hay >= jbar.top ()  && hay <= jbar.bottom ()) {
         // found it.
         //
         result = j + this->firstDisplayed;
         if (result >= this->count ()) result = -1;
         break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
int QEBarchart::indexOfPosition (const QPoint& p) const
{
   return this->indexOfPosition (p.x (), p.y ());
}
//------------------------------------------------------------------------------
bool QEBarchart::paintItem (QPainter& painter,
                             const int position,
                             const int valueIndex) const
{
   const int finishRight = this->paintArea.right ();
   QRect bar;
   double value;
   int tickPos;
   double base;
   double baseLineFraction;
   double valueFraction;
   double tickLength;
   int top;
   int bot;
   QColor colour;
   QColor boarderColour;
   QBrush brush;
   QPen pen;
   QString text;
   bool labelType =false;
   QFontMetrics fm =  painter.fontMetrics();
   bar = this->fullBarRect (position);
   if (bar.left () >= finishRight) return false;   // Off to the side
   if (bar.right () > finishRight) {
      bar.setRight (finishRight);                  // Truncate
      if (bar.width () < 5) return false;          // Tooo small!!
   }

   value = this->dataArray.value (valueIndex, NO_DATA_VALUE);
   base = this->mBaseLine;
//
   // Is value invalid, i.e. un-defined BUT still in paint area?
   //
   if (isNullDataValue (value)) return true;

   if (this->mLogScale) {
      value = LOG10 (value);
      base = LOG10 (base);
   }

   valueFraction = (value             - this->drawMinimum) /
                   (this->drawMaximum - this->drawMinimum);
   valueFraction = LIMIT (valueFraction, 0.0, 1.0);

   baseLineFraction = (base              - this->drawMinimum) /
                      (this->drawMaximum - this->drawMinimum);
   baseLineFraction = LIMIT (baseLineFraction, 0.0, 1.0);



/*if (value <=1){tickPos = scaleWidths[0]-scaleWidths[1];tickLength=(value*tickPos);top = scaleWidths[0]-(int)tickLength;}
else if(value >1 && value <=10){tickPos = scaleWidths[1]-scaleWidths[2];tickLength=(value*tickPos)/10;top = scaleWidths[1]- (int)tickLength;}
else if (value >10 && value <=100){tickPos = scaleWidths[2]-scaleWidths[3];tickLength=(value*tickPos)/100;top = scaleWidths[2]- (int)tickLength;}
else if (value >100 && value <=1000){tickPos = scaleWidths[3]-scaleWidths[4];tickLength=(value*tickPos)/1000;top = scaleWidths[3] - (int)tickLength;}
else if (value >1000 && value <=10000){tickPos = scaleWidths[4]-scaleWidths[5];tickLength=(value*tickPos)/10000;top = scaleWidths[4] - (int)tickLength;}
else if (value >10000 && value <=100000){tickPos = scaleWidths[5]-scaleWidths[6];tickLength=(value*tickPos)/100000;top = scaleWidths[5] - (int)tickLength;}
else if (value >100000 && value <=1000000){tickPos = scaleWidths[6]-scaleWidths[7];tickLength=(value*tickPos)/1000000;top = scaleWidths[6] - (int)tickLength;}*/


   top = bar.bottom () - (int) (valueFraction * bar.height ());
   bot = bar.bottom () - (int) (baseLineFraction * bar.height ());
   DEBUG <<bot;
   bar.setTop (top);
   bar.setBottom (bot);
/* 579 
498 
416 // 82 seems to be the tick widhts for all
334 
253 
171 
89 
7 */
   // All good to go - set up colour.
   //
   colour = this->getPaintColour (valueIndex);
   boarderColour = QEUtilities::darkColour (colour);

   if (!this->isEnabled ()) {
      colour = QEUtilities::blandColour (colour);
      boarderColour = QEUtilities::blandColour (boarderColour);
   }

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (colour);
   painter.setBrush (brush);

   pen.setWidth (1);
   if (this->mDrawBorder) { // draw the bar item boarder
      // Use darker version of the color for the boarder.
      //
      pen.setColor (boarderColour);
   } else {
      pen.setColor (colour);
   }
   painter.setPen (pen);
   painter.drawRect (bar);

   if(!this->mXLabels.isEmpty())
   {
       QFont tFont("Sans Serif",9, QFont::AllLowercase);
       painter.setFont(tFont);
       painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));

       const int axisOffset = QEScaling::scale (8);
       int tHeight = tFont.pointSize() + axisOffset;
       //int barMid = this->useBarWidth/2 +1;
       text = this->mXLabels.value(position,"NAN");
       //int tMid = fm.width(text) / 2 + 1;
       //int tOffset = tMid - barMid;
       //DEBUG << tOffset;
       int tLeft = bar.left() ;//- tOffset;
       int tTop  = bar.bottom() + axisOffset;
       int tRight = bar.right();
       int tBottom = tTop + tHeight;
       //painter.drawRect(QRect(tLeft,tTop,tRight - tLeft,tBottom - tTop));
       painter.drawText(QRect(tLeft,tTop,tRight - tLeft,tBottom - tTop),Qt::AlignCenter,text);
   }

  return true;
}

//------------------------------------------------------------------------------
// Ensure text is generated consistantly.
//
QString QEBarchart::coordinateText (const double value,const bool type) const
{
   QString result;

   if (this->mLogScale && type) {
      // Scale, this given value, is logged, must unlog it.
      //
      result = QString ("%1").arg (EXP10 (value), 0, 'e', 0);
   } else {
      result = QString ("%1").arg (value);
   }
   return result;
}

//------------------------------------------------------------------------------
// This is like paintGrid, BUT with no actual painting.
// Maybe we could keep / cache generated value text images.
//
int QEBarchart::maxPaintTextWidth (QPainter& painter) const
{
   int result;
   bool labeltype = true;
   if (this->mShowScale) {
      // yes - scale present. Find required text width.
      //
      QFontMetrics fm = painter.fontMetrics ();
      result = 1;
      for (int j = 0; true; j++) {
         // user define scale or defaults
         double value = this->drawMinimum + (j*this->drawMajor);
         if (value > this->drawMaximum) break;
         if (j > 1000) break;  // sainity check
         QString text;
         if(this->mYLabels.isEmpty()){
             text = this->coordinateText (value,labeltype);
             int w = fm.width (text);
             if (result < w) result = w;
         }
         else {
             text =  this->mYLabels.value(j," ");
             result = 1; // sanity, might be redundant
             int w = fm.width(text);
             if(result < w) result = w;
         }

      }
   } else {
      // no scale - use minimal "text" width.
      //
      result = 0;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEBarchart::paintGrid (QPainter& painter) 
{
   const int axisOffset = QEScaling::scale (4);

   QColor penColour;
   QPen pen;
   int j;
   double value;
   double fraction;
   int y;
   QFontMetrics fm = painter.fontMetrics ();
   QFont pf (this->font ());
   QString text;
   int x=0;
   bool labelType = true;
   penColour = QEUtilities::fontColour (Qt::gray); // black/white
   if (!this->isEnabled ()) {
      penColour = QEUtilities::blandColour (penColour);   // washed/greyed out
   }
   pen.setColor (penColour);
   pen.setWidth (1);
   pen.setStyle (Qt::DashLine);
   painter.setPen (pen);

   for (j = 0; true; j++) {
      value = this->drawMinimum + (j*this->drawMajor);
      if (value > this->drawMaximum) break;
      if (j > 1000) break;  // sainity check

      fraction = (value             - this->drawMinimum) /
                 (this->drawMaximum - this->drawMinimum);

      // Same idea as we used in paintItem.
      //
      y = this->paintArea.bottom () - (int) (fraction * this->paintArea.height ());
      scaleWidths[j] = y;
      int scale = (this->drawMaximum - this->drawMinimum) /this->drawMajor;
     // DEBUG << j << scaleWidths[1] << y;
      if (this->mShowGrid && (j > 0 && j < scale)) {
         painter.drawLine (this->paintArea.left () - axisOffset, y,
                           this->paintArea.right(), y);
      }
      if (this->mShowScale) {
         // Centre text. For height, pointSize seems better than fm.height ()
         // painter.drawText needs bottom left coordinates.
         //
         if(this->mYLabels.isEmpty()){
             text = this->coordinateText (value,labelType);
             x = this->paintArea.left () - fm.width (text) - 2 * axisOffset;
             y = y +  (pf.pointSize () + 1)/2;
         }
         else {
             text = this->mYLabels.value(j," ");
             x = this->paintArea.left () - fm.width (text) - 2 * axisOffset;
             y = y +  (pf.pointSize () + 1)/2;
         }
         painter.drawText (x, y, text);
      }

      /*if (this->mShowScale) {
         // Centre text. For height, pointSize seems better than fm.height ()
         // painter.drawText needs bottom left coordinates.
         //
         //text = this->coordinateText (tickValues[j],labelType);
         x = this->paintArea.left () - fm.width (tickValue[j]) - 2 * axisOffset;
         y = y +  (pf.pointSize () + 1)/2;
         if(this->mYLabels.isEmpty()){
             painter.drawText (x, y, coordinateText(j,labelType));
         }
         else {
             QString text = this->mYLabels.value(j," ");
             painter.drawText (x, y, text);
         }
      }*/
   }

   if (this->mDrawAxies) {
      pen.setWidth (1);
      pen.setStyle (Qt::SolidLine);
      painter.setPen (pen);
      QRect axisRect(paintArea.adjusted(-axisOffset,-1,0,axisOffset-1));
      painter.drawRect(axisRect);
      /// following are the optional addtions
      if(!this->mXTitle.isEmpty()){
          //painter.setFont(QFont("Cantarell",14, QFont::Bold));
          int titleW = fm.width(this->mXTitle);
          //DEBUG << titleW;
          int left = (this->paintArea.right() - this->paintArea.left()) / 2 + titleW/2;
          int top = this->paintArea.bottom() + 2*pf.pointSize() + axisOffset;
          // DEBUG << this->paintArea.bottom() << pf.pointSize() << top << left;
          //int right = this->paintArea.right()/2 + (titleW/2 + 1) + margin;
          //int bottom = this->paintArea.bottom() + 2*pf.pointSize() + 3*margin;
          //QRect tRec = QRect(left,top,right,bottom);
          //DEBUG << tRec.left() << tRec.top() << tRec.right() << tRec.bottom();
          //painter.drawText(tRec,Qt::AlignCenter,this->mXTitle);
          painter.drawText(left,top,this->mXTitle);
      }
      //painter.setFont(QFont("Cantarell",14, QFont::Bold));
      //painter.rotate(-90.0);
      //QString yLabel("Dose Rate mrem/h");
     // painter.drawText(-(this->paintArea.bottom()/2)- (3*(fm.width (yLabel))/4),x-5, yLabel);
      //painter.rotate(90.0);
   }
}

//------------------------------------------------------------------------------
//
void QEBarchart::paintAllItems ()
{
   const int numberGrid = 7;   // approx number of y grid lines.
   const int margin = QEScaling::scale (2);
   const int extra = QEScaling::scale (8);

   // Only apply style on change as this casues a new paint event.
   // Maybe we just just paint a rectangle of the appropriate colour.
   //
/* this where i changed the background color of the widget it was this->getBackgroundColour () 
   after Qt::white now this->getBackgroundColour ()
*/
  QString ownStyle = QEUtilities::colourToStyle (this->getBackgroundColour ()); 
   if (this->styleSheet() != ownStyle) {
      this->setStyleSheet (ownStyle);
   }

   // Draw everything with antialiasing off.
   //
   QPainter painter (this->histogramArea);
   painter.setRenderHint (QPainter::Antialiasing, false);

   double useMinimum = this->mMinimum;
   double useMaximum = this->mMaximum;
   if (this->mAutoScale) {
      bool foundValue = false;
      double searchMinimum = +1.0E25;
      double searchMaximum = -1.0E25;
      for (int j = 0; j < this->dataArray.count (); j++) {
         double v = this->dataArray.at (j);
         if (isNullDataValue (v)) continue;
         searchMinimum  = MIN (v, searchMinimum);
         searchMaximum  = MAX (v, searchMaximum);
         foundValue = true;
      }
      if (foundValue) {
         useMinimum  = searchMinimum;
         useMaximum  = searchMaximum;
      }
   }
   // Do not allow ultra small spans, which will occur when autoscaling
   // a histogram with a single value.
   //
   if ((useMaximum - useMinimum) < MINIMUM_SPAN) {
      double midway = (useMaximum + useMinimum)/2.0;
      useMinimum = midway - MINIMUM_SPAN/2.0;
      useMaximum = midway + MINIMUM_SPAN/2.0;
   }

   // Now calc draw min max  - log of min / max if necessary.
   //
   QEDisplayRanges displayRange;
   displayRange.setRange (useMinimum, useMaximum);

   if (this->mLogScale) {
      displayRange.adjustLogMinMax (this->drawMinimum, this->drawMaximum, this->drawMajor);
      // We use, and this store,  the log of thwse values when using the log scale.
      // drawMajor already reflects the scale scale and is typicaly 1 (as in 1 decade).
      //
      this->drawMinimum = LOG10 (this->drawMinimum);
      this->drawMaximum = LOG10 (this->drawMaximum);

   } else {
      displayRange.adjustMinMax (numberGrid, true, this->drawMinimum, this->drawMaximum, this->drawMajor);
   }

   // Define actual chart draw area ...
   //

   QRect hostWidgetArea = this->histogramArea->geometry ();
   DEBUG << this->histogramArea->geometry ();
   QFont ownFont (this->font ());
   int halfPointSize = (ownFont.pointSize () + 1) / 2;//6
   this->paintArea.setTop (margin + halfPointSize);//8
   //this->paintArea.setBottom (hostWidgetArea.height () - 55);
   DEBUG << this->mXTitle;
   //DEBUG << this->mBarWidth;
   if(this->mXTitle.isEmpty() && this->mXLabels.isEmpty()){
       /// this is the framework defaults: no xlabel no xtitle
       DEBUG << "NO char";
       this->paintArea.setBottom (hostWidgetArea.height () - margin - halfPointSize);
   }
   /// add xlabels and/or title
   else {
       this->paintArea.setBottom (hostWidgetArea.height () - 3*margin - (4*halfPointSize));
       DEBUG << "Char" << hostWidgetArea.height ();
   }
   this->paintArea.setLeft ((this->maxPaintTextWidth (painter) + extra));//the 10 are added for space between the ylabel and the ytick numbers
   this->paintArea.setRight (hostWidgetArea.width () - margin);
   //DEBUG <<this->maxPaintTextWidth (painter) + extra;
   // Do grid and axis - note this might tweak useMinimum/useMaximum.
   //
   this->paintGrid (painter);

   this->useGap = this->mGap;
   this->useBarWidth = this->mBarWidth;
   if (this->mAutoBarGapWidths) {
      const int n = this->dataArray.count ();

      if (n <= 1) {
         this->useGap = 0;
         this->useBarWidth = this->paintArea.width ();
         // DEBUG << "Hello";
      } else {
         const int markSpace = 6;

         // For large n itemWidth is essentially paintArea.width / n
         // For small n, this accounts for n bars and n-1 gaps.
         //
         int itemWidth = ((markSpace + 1) * paintArea.width ()) / ((markSpace + 1)*n - 1);
         if (itemWidth < 3) itemWidth = 3;
         this->useGap = itemWidth / markSpace;
         // There is an implicit +1 in the fullBarRect function.
         this->useBarWidth = MAX (1, itemWidth - this->useGap - 1);
         DEBUG << "New Bar width: " << this->useBarWidth;
      }
   }

   // Maximum number of items that could be drawn.
   //
   const int maxDrawable = this->dataArray.count () - this->firstDisplayed;
   this->numberDisplayed = 0;
   for (int posnIndex = 0; posnIndex < maxDrawable; posnIndex++) {
      int dataIndex = this->firstDisplayed + posnIndex;

      bool painted = paintItem (painter, posnIndex, dataIndex);
      if (painted) {
         this->numberDisplayed = posnIndex + 1;
      } else {
         break;
      }
   }
   // Lastly ...
   //
   const int max = this->scrollMaximum ();
   //DEBUG << max;
   this->scrollbar->setRange (0, max);
   this->scrollbar->setVisible (max > 0);// the scrollbar appear when  max
}

//------------------------------------------------------------------------------
//
bool QEBarchart::eventFilter (QObject *obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   if (type == QEvent::Paint) {
      if (obj == this->histogramArea) {
         this->paintAllItems ();
         result = true;  // event has been handled
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEBarchart::createTestData ()
{
   this->clear ();

   for (int j = 0; j < this->mTestSize; j++) {
      double f;
      double v;
      QColor c;

      // Do special for first and last.
      //
      if (j == 0 || j == this->mTestSize - 1) {
         v = this->mMaximum;
      } else {
         f = qrand () / (double) RAND_MAX;
         v = f * (this->mMaximum - this->mMinimum) +  this->mMinimum;
      }

      f = qrand () / (double) RAND_MAX;
      c.setHsl ((int)(f * 360.0), 255, 128);

      this->dataArray << v;
      this->colourArray << c;
   }
}
/*void QEBarchart::setTitles(const Title  titleIn)
{
   // Guard against circular signal-slot connections.
   //
   //if (this->titlesChangeInhibited) return;
   //for (int slot = 0; slot < (this->mXLabels.size()); slot++) {
   //   this->mXLabels.insert(slot, xLabelsIn.value (slot, ""));
   //}
  // this->titlesChangeInhibited = true;
   emit this->titlesChanged(titleIn);
  // this->titlesChangeInhibited = false;
   //this->update();
}
enum Title  QEBarchart::getTitles () const
{
    return Low;
}*/
//==============================================================================
// Property functions (standard)
//==============================================================================
//
// Standard propery access macro.
//
#define PROPERTY_ACCESS(type, name, convert, extra)          \
                                                             \
void QEBarchart::set##name (const type value) {             \
   type temp;                                                \
   temp = convert;                                           \
   if (this->m##name != temp) {                              \
      this->m##name = temp;                                  \
      extra;                                                 \
      this->update ();                                       \
   }                                                         \
}                                                            \
                                                             \
type QEBarchart::get##name () const {                       \
   return this->m##name;                                     \
}

#define NO_EXTRA

PROPERTY_ACCESS (int,    BarWidth,         LIMIT (value, 1, 120),                                 this->mAutoBarGapWidths = false)
PROPERTY_ACCESS (int,    Gap,              LIMIT (value, 0, 20),                                  this->mAutoBarGapWidths= false)
PROPERTY_ACCESS (int,    Margin,           LIMIT (value, 0, 20),                                  this->layout->setMargin (this->mMargin))
PROPERTY_ACCESS (double, Minimum,          LIMIT (value, -1.0E20, this->mMaximum - MINIMUM_SPAN), this->mAutoScale = false)
PROPERTY_ACCESS (double, Maximum,          LIMIT (value, this->mMinimum + MINIMUM_SPAN, +1.0E40), this->mAutoScale = false)
PROPERTY_ACCESS (double, BaseLine,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   AutoScale,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   AutoBarGapWidths, value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   ShowScale,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   ShowGrid,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   LogScale,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   DrawAxies,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   DrawBorder,       value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QColor, BackgroundColour, value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QColor, BarColour,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QString, XTitle,          value,                                                NO_EXTRA)
PROPERTY_ACCESS (QString, YTitle,          value,                                                NO_EXTRA)
PROPERTY_ACCESS (QStringList, XLabels,     value,                                         NO_EXTRA)
PROPERTY_ACCESS (QStringList, YLabels,     value,                                           NO_EXTRA)
PROPERTY_ACCESS (Qt::Orientation,  Orientation,  value,                                           NO_EXTRA)
PROPERTY_ACCESS (int,    TestSize,          LIMIT (value, 0, MAX_CAPACITY),                       this->createTestData ())

#undef PROPERTY_ACCESS

// end

