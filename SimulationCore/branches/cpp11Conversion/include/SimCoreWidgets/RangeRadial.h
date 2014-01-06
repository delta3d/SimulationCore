/* -*-c++-*-
 * Simulation Core
 * Copyright 2007-2008, Alion Science and Technology
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#ifndef RANGE_RADIAL_H
#define RANGE_RADIAL_H
// Commented this out. This has build problems on a lot of computers. So, commenting it out 
// until that is eventually resolved.
/*

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <QtGui/QWidget>
#include <QtDesigner/QDesignerExportWidget>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
class QMouseEvent;



////////////////////////////////////////////////////////////////////////////////
// CUSTOM WIDGET CODE
////////////////////////////////////////////////////////////////////////////////
class QDESIGNER_WIDGET_EXPORT RangeRadial : public QWidget
{
   Q_OBJECT
      Q_PROPERTY( double startAngle READ getStartAngle WRITE setStartAngle )
      Q_PROPERTY( double endAngle READ getEndAngle WRITE setEndAngle )
      Q_PROPERTY( QColor startHandleColor READ getStartHandleColor WRITE setStartHandleColor )
      Q_PROPERTY( QColor middleHandleColor READ getMiddleHandleColor WRITE setMiddleHandleColor )
      Q_PROPERTY( QColor endHandleColor READ getEndHandleColor WRITE setEndHandleColor )
      Q_PROPERTY( QColor pieColor READ getPieColor WRITE setPieColor )

   public:
      RangeRadial( QWidget* parent = nullptr );

      void onCreated();

      void setPieColor( const QColor& color );
      QColor getPieColor() const;

      void setPieFocusColor( const QColor& color );
      QColor getPieFocusColor() const;

      void setStartHandleColor( const QColor& color );
      QColor getStartHandleColor() const;

      void setMiddleHandleColor( const QColor& color );
      QColor getMiddleHandleColor() const;

      void setEndHandleColor( const QColor& color );
      QColor getEndHandleColor() const;

      void setHandleHoverColor( const QColor& color );
      QColor getHandleHoverColor() const;

      double getStartAngle() const;

      double getEndAngle() const;

   public slots:
      void setStartAngle( double angle );

      void setEndAngle( double angle );

   signals:
      void changedStartAngle( double angle );

      void changedEndAngle( double angle );

   protected:
      virtual void paintEvent( QPaintEvent *paintEvent );
      virtual void mouseMoveEvent( QMouseEvent* mouseEvent );
      virtual void mousePressEvent( QMouseEvent* mouseEvent );
      virtual void mouseReleaseEvent( QMouseEvent* mouseEvent );

      QPointF getCenter() const;

      QRect* getHandle( const QPoint& mousePoint );

      double getAngleToPoint( const QPoint& point ) const;

      void setHandleToAngle( QRect& handle, double angle, double radiusScale = 1.0 );

      void updateHandles();

   private:
      double mStart;
      double mEnd;
      QRect* mHandleHeld;
      QRect* mHandleHovered;
      QRect mHandleStart;
      QRect mHandleMiddle;
      QRect mHandleEnd;
      QColor mColorHandleStart;
      QColor mColorHandleMiddle;
      QColor mColorHandleEnd;
      QColor mColorHandleHover;
      QColor mColorPie;
      QColor mColorPieFocus;
};
*/
#endif
