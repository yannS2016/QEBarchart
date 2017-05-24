#ifndef QE_BARCHART_H
#define QE_BARCHART_H

#include <QColor>
#include <QEvent>
#include <QFrame>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>



/// The QEBarchart class is a non-EPICS aware histogram widget.
/// The value of, i.e. the length of each bar, and colour may be set indepedently.
///
class QEBarchart:public QFrame {
   Q_OBJECT
   Q_ENUMS(Title)
public:
   enum Title{ High, Low, VeryHigh, VeryLow};
   Q_PROPERTY (bool   autoBarGapWidths READ getAutoBarGapWidths WRITE setAutoBarGapWidths)
   Q_PROPERTY (int    barWidth         READ getBarWidth         WRITE setBarWidth)
   Q_PROPERTY (int    gap              READ getGap              WRITE setGap)
   Q_PROPERTY (int    margin           READ getMargin           WRITE setMargin)
   Q_PROPERTY (bool   autoScale        READ getAutoScale        WRITE setAutoScale)
   Q_PROPERTY (double minimum          READ getMinimum          WRITE setMinimum)
   Q_PROPERTY (double maximum          READ getMaximum          WRITE setMaximum)
   Q_PROPERTY (double baseLine         READ getBaseLine         WRITE setBaseLine)
   Q_PROPERTY (bool   drawAxies        READ getDrawAxies        WRITE setDrawAxies)
   Q_PROPERTY (bool   showScale        READ getShowScale        WRITE setShowScale)
   Q_PROPERTY (bool   showGrid         READ getShowGrid         WRITE setShowGrid)
   Q_PROPERTY (bool   logScale         READ getLogScale         WRITE setLogScale)
   // Where possible I spell colour properly.
   Q_PROPERTY (QColor backgroundColour READ getBackgroundColour WRITE setBackgroundColour)
   Q_PROPERTY (QColor barColour        READ getBarColour        WRITE setBarColour)
   Q_PROPERTY (bool   drawBorder       READ getDrawBorder       WRITE setDrawBorder)
   Q_PROPERTY (Qt::Orientation orientation READ getOrientation  WRITE setOrientation)
   /// Allows specification of graph custom x-labels. If blank, the numbering is
   /// incremented by one for each added bars present i.e : 1,2,3 ...
   Q_PROPERTY (QStringList xlabels     READ getXLabels          WRITE setXLabels)
    ///Q_PROPERTY (Title  titles            READ getTitles        WRITE setTitles)
   /// Allows specification of graph custom y-labels. If blank, the numbering is
   /// is se to the framework defaults
   Q_PROPERTY (QStringList ylabels     READ getYLabels          WRITE setYLabels)
   /// Allows specification of graph custom x title.
   Q_PROPERTY (QString xTitle          READ getXTitle           WRITE setXTitle)
   /// Allows specification of graph custom y title.
   Q_PROPERTY (QString yTitle          READ getYTitle           WRITE setYTitle)
   // Test - used for previewing/testing - may be removed.
   //
   Q_PROPERTY (int    testSize         READ getTestSize         WRITE setTestSize)

   //Q_PROPERTY (QStringList XLabels READ getXLabels WRITE setXLabels)

public:
   typedef QVector <double> DataArray;


   // Constructor
   //
   explicit QEBarchart (QWidget* parent = 0);
   virtual ~QEBarchart () {}

   virtual QSize sizeHint () const;

   // Property functions
   // Standard propery access macro.
   //
#define PROPERTY_ACCESS(type, name)        \
   void set##name (const type value);      \
   type get##name () const;


   PROPERTY_ACCESS (int,    BarWidth)
   PROPERTY_ACCESS (int,    Gap)
   PROPERTY_ACCESS (int,    Margin)
   PROPERTY_ACCESS (double, Minimum)
   PROPERTY_ACCESS (double, Maximum)
   PROPERTY_ACCESS (double, BaseLine)
   PROPERTY_ACCESS (bool,   AutoScale)
   PROPERTY_ACCESS (bool,   AutoBarGapWidths)
   PROPERTY_ACCESS (bool,   ShowScale)
   PROPERTY_ACCESS (bool,   ShowGrid)
   PROPERTY_ACCESS (bool,   LogScale)
   PROPERTY_ACCESS (bool,   DrawAxies)
   PROPERTY_ACCESS (bool,   DrawBorder)
   PROPERTY_ACCESS (QColor, BackgroundColour)
   PROPERTY_ACCESS (QColor, BarColour)
   PROPERTY_ACCESS (Qt::Orientation, Orientation)
   PROPERTY_ACCESS (QString, XTitle)
   PROPERTY_ACCESS (QString, YTitle)
   PROPERTY_ACCESS (QStringList, XLabels)
   PROPERTY_ACCESS (QStringList, YLabels)
   //
   PROPERTY_ACCESS (int,    TestSize)
#undef PROPERTY_ACCESS

   int count () const;

   // Returns associated data index of specified position, or -1.
   // data position includes max value/potetial draw area, not currently
   // occupied draw area.
   //
   int indexOfPosition (const int x, const int y) const;
   int indexOfPosition (const QPoint& p) const;  // overloaded function

   void clearValue (const int index);
   void clearColour (const int index);
   void clear ();   // clear all data

   double value (const int index) const;
   DataArray values () const;
   void paintGrid (QPainter& painter) ;
   //QStringList getXLabels() const;
  // Title  getSpacing () const;
public slots:
   void setColour (const int index, const QColor& value);
   void setValue (const int index, const double value);
   void setValues (const DataArray& values);
   //void setTitles (const Title  titles);
   //void setXLabels(const QStringList& xLabelsIn);
  // void setYLabels(const QStringList value);
signals:
   //void labelsChanged (const QStringList& pvNameSet);
   // void spacingChanged (const Title  pvNameSet);
private:
   int   firstBarLeft () const;
   QRect fullBarRect (const int position) const;

   QString coordinateText (const double value, const bool type) const;
   int maxPaintTextWidth (QPainter& painter) const;
   //void paintGrid (QPainter& painter) const;

   // Returns true if item position is in the paintArea
   bool paintItem (QPainter& painter, const int position, const int index) const;

   void paintAllItems ();
   bool eventFilter (QObject* obj, QEvent* event);

   int scrollMaximum () const;

   // Detrmines the color to paint. If slot has a specific colour, that colour
   // is used, otherwise the default bar colour is returned.
   //
   QColor getPaintColour (const int index) const;

   // Usefull for preview mode and testing.
   //
   void createTestData ();
   // Private types
   //
   typedef QVector <QColor> ColourArray;
   typedef QList<int> drawArea;
   // Private data
   inline drawArea definePaintArea(const int position);
   // Internal widgets

   QVBoxLayout* layout;
   void setXlabelPositions(QPoint xlabels);
   // histogramArea does nothing per se other than be size managed by the
   // layout and provides a paint area for the QEBarchart.
   //
   QWidget* histogramArea;
   QRect paintArea;           // paint area last used - subset of histogramArea
   QScrollBar* scrollbar;

   DataArray dataArray;
   ColourArray colourArray;

   // class property member variable names start with m so as not to clash with
   // the propery names.
   //
   QColor mBarColour;
   QColor mBackgroundColour;

   double mMinimum;
   double mMaximum;
   double mBaseLine;
   int mBarWidth;
   int mGap;
   int mMargin;
   bool mAutoBarGapWidths;
   bool mAutoScale;
   bool mDrawAxies;
   bool mDrawBorder;
   bool mShowScale;
   bool mShowGrid;
   bool mLogScale;
   Qt::Orientation mOrientation;
   int mTestSize;
   // custom members
   QString mXTitle;
   QString mYTitle;
   QStringList mXLabels;
   QStringList mYLabels;
   QStringList mSpacing;
   bool titlesChangeInhibited;
   int firstDisplayed;
   int numberDisplayed;
   double drawMinimum;
   double drawMaximum;
   double drawMajor;
   double useGap;
   double useBarWidth;


private slots:
   void scrollBarValueChanged (int value);
};

#endif // QE_BARCHART_H

