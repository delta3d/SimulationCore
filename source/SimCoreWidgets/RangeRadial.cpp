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
// Commented this out. This has build problems on a lot of computers. So, commenting it out 
// until that is eventually resolved.
/*

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <cmath>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <SimCoreWidgets/RangeRadial.h>



////////////////////////////////////////////////////////////////////////////////
// CUSTOM WIDGET CODE
////////////////////////////////////////////////////////////////////////////////
RangeRadial::RangeRadial( QWidget* parent )
   : QWidget(parent)
   , mStart(0.0)
   , mEnd(90.0)
   , mHandleHeld(nullptr)
   , mHandleHovered(nullptr)
   , mHandleStart(QRect(0,0,4,4))
   , mHandleMiddle(QRect(0,0,4,4))
   , mHandleEnd(QRect(0,0,4,4))
   , mColorHandleStart(QColor(0,128,0,255))
   , mColorHandleMiddle(QColor(0,0,128,255))
   , mColorHandleEnd(QColor(128,0,0,255))
   , mColorHandleHover(QColor(255,255,0,255))
   , mColorPie(QColor(128,128,255,255))
   , mColorPieFocus(QColor(255,255,128,255))
{
   setMinimumSize( QSize(32,32) );

   updateHandles();
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::onCreated()
{
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setStartAngle( double angle )
{
   if( mStart != angle )
   {
      mStart = angle;
      emit changedStartAngle( mStart );
      updateHandles();
   }
}

////////////////////////////////////////////////////////////////////////////////
double RangeRadial::getStartAngle() const
{
   return mStart;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setEndAngle( double angle )
{
   if( mEnd != angle )
   {
      mEnd = angle;
      emit changedEndAngle( mEnd );
      updateHandles();
   }
}

////////////////////////////////////////////////////////////////////////////////
double RangeRadial::getEndAngle() const
{
   return mEnd;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setPieColor( const QColor& color )
{
   mColorPie = color;
   update();
}

////////////////////////////////////////////////////////////////////////////////
QColor RangeRadial::getPieColor() const
{
   return mColorPie;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setPieFocusColor( const QColor& color )
{
   mColorPieFocus = color;
   update();
}

////////////////////////////////////////////////////////////////////////////////
QColor RangeRadial::getPieFocusColor() const
{
   return mColorPieFocus;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setStartHandleColor( const QColor& color )
{
   mColorHandleStart = color;
   update();
}

////////////////////////////////////////////////////////////////////////////////
QColor RangeRadial::getStartHandleColor() const
{
   return mColorHandleStart;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setMiddleHandleColor( const QColor& color )
{
   mColorHandleMiddle = color;
   update();
}

////////////////////////////////////////////////////////////////////////////////
QColor RangeRadial::getMiddleHandleColor() const
{
   return mColorHandleMiddle;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setEndHandleColor( const QColor& color )
{
   mColorHandleEnd = color;
   update();
}

////////////////////////////////////////////////////////////////////////////////
QColor RangeRadial::getEndHandleColor() const
{
   return mColorHandleEnd;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setHandleHoverColor( const QColor& color )
{
   mColorHandleHover = color;
   update();
}

////////////////////////////////////////////////////////////////////////////////
QColor RangeRadial::getHandleHoverColor() const
{
   return mColorHandleHover;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::paintEvent( QPaintEvent *paintEvent )
{
   // Initialize painter.
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing);

   // Initialize pen.
   QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin );
   painter.setPen(pen);
   painter.setBrush( QBrush(mHandleHeld != nullptr ? mColorPieFocus : mColorPie) );

   // Pie Shape
   painter.drawPie( 1, 1, width()-3, height()-3, mStart * 16, mEnd * 16 );

   // Center Range Line
   QPoint lineOffset(1,1);
   pen.setStyle( Qt::DashLine );
   painter.drawLine( getCenter()-pos()+lineOffset, mHandleMiddle.center()+lineOffset );

   // Get ready to draw the knobs.
   pen.setStyle( Qt::NoPen );

   // Start Knob
   painter.setBrush( QBrush(mHandleHovered == &mHandleStart ? mColorHandleHover : mColorHandleStart) );
   painter.drawRect( mHandleStart );

   // Start Knob
   painter.setBrush( QBrush(mHandleHovered == &mHandleMiddle ? mColorHandleHover : mColorHandleMiddle) );
   painter.drawRect( mHandleMiddle );

   // End Knob
   painter.setBrush( QBrush(mHandleHovered == &mHandleEnd ? mColorHandleHover : mColorHandleEnd) );
   painter.drawRect( mHandleEnd );
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::mouseMoveEvent( QMouseEvent* mouseEvent )
{
   // Determine if a handle is being hovered over by the mouse cursor.
   bool handleHeld = mHandleHeld != nullptr;
   QRect* handle = getHandle( mouseEvent->pos() );
   if( handle != mHandleHovered )
   {
      mHandleHovered = handle;

      // Initiate a repaint here only if the handle positions are not
      // going to be changed.
      // Updating the handle positions will automatically initiate
      // a repaint event.
      if( ! handleHeld )
      {
         update();
      }
   }

   // Do not do anything else if a handle has not been selected.
   if( ! handleHeld )
   {
      return;
   }

   // Find the angle to the mouse point from the center of this widget.
   // Convert the in-going point to widget-relative coordinates from
   // absolute window coordinates.
   double angle = getAngleToPoint( mouseEvent->pos() + pos() );

   // If on middle/rotate handle, change the rotation of the range.
   if( mHandleHeld == &mHandleMiddle )
   {
      mStart = angle - mEnd * 0.5;
      mStart = mStart < 0.0 ? mStart + 360 : mStart;
      emit changedStartAngle( mStart );
   }
   else
   {
      double difference = 0.0;

      // Get the difference in angle by the selected handle.
      if( mHandleHeld == &mHandleStart )
      {
         difference = mStart - angle;
      }
      else if( mHandleHeld == &mHandleEnd )
      {
         difference = angle - (mStart + mEnd);
      }

      // Ensure the difference is a valid range.
      // It should not exceed 180 degrees in either direction.
      if( difference < -180.0 )
      {
         difference += 360.0;
      }
      else if( difference > 180.0 )
      {
         difference -= 360.0;
      }

      bool shrinking = difference < 0.0 && mEnd > 0.0;
      bool growing = difference > 0.0 && mEnd < 360.0;
      if( growing || shrinking )
      {
         if( mHandleHeld == &mHandleStart )
         {
            // Ensure adding the difference will not overflow or under flow
            // the range.
            double testValue = mEnd + difference * 2.0;
            if( growing )
            {
               if( testValue > 360.0 )
               {
                  difference = ( 360.0 - mEnd ) * 0.5;
               }
            }
            else // shrinking
            {
               if( testValue < 0.0 )
               {
                  difference = -mEnd * 0.5;
               }
            }

            // Apply the difference to the start and end points.
            // NOTE: End is the full range in degrees from the start point.
            mStart -= difference;
            mEnd += difference * 2.0;

            mStart = mStart < 0.0 ? mStart + 360.0 : mStart;
            mStart = mStart > 360.0 ? mStart - 360.0 : mStart;
            emit changedStartAngle( mStart );
         }
         else // This is the just end handle.
         {
            mEnd += difference;
         }

         // Ensure floating point integrity. Clamp the range amount.
         // It should not exceed one full revolution.
         mEnd = mEnd < 0.0 ? 0.0 : mEnd;
         mEnd = mEnd > 360.0 ? 360.0 : mEnd;
         emit changedEndAngle( mEnd );
      }
   }

   // Re-distribute the handles based on the new angles that have been set.
   // NOTE: This method will call update to initiate a repaint event.
   updateHandles();
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::mousePressEvent( QMouseEvent* mouseEvent )
{
   // Determine if a handle has been pressed.
   mHandleHeld = getHandle( mouseEvent->pos() );

   update();
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::mouseReleaseEvent( QMouseEvent* mouseEvent )
{
   // Handle has been released.
   mHandleHeld = nullptr;

   update();
}

////////////////////////////////////////////////////////////////////////////////
QPointF RangeRadial::getCenter() const
{
   return QPointF(geometry().center());
}

////////////////////////////////////////////////////////////////////////////////
QRect* RangeRadial::getHandle( const QPoint& mousePoint )
{
   // NOTE: In case the range has been set to 0, allow the "start" handle
   // to be selected first. By doing this, the range can be temporarily
   // expanded so that the user can access the other "end" handle.

   // Middle/Rotation Handle
   if( mHandleMiddle.contains( mousePoint ) )
   {
      return &mHandleMiddle;
   }
   // Start Handle
   if( mHandleStart.contains( mousePoint ) )
   {
      return &mHandleStart;
   }
   // End Handle
   if( mHandleEnd.contains( mousePoint ) )
   {
      return &mHandleEnd;
   }
   return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
double RangeRadial::getAngleToPoint( const QPoint& point ) const
{
   static const double RADS_TO_DEGS_RATIO = 180.0/3.141592653589;

   // Derive the radius from the center of the widget to the
   // specified point.
   QPointF radius( point - getCenter() );

   // Flip Y from left-handed system to right-handed system.
   radius.setY( -radius.y() );

   // Find the length of the radius so that it can be normalized.
   double radiusLength = sqrt( radius.x() * radius.x() + radius.y() * radius.y() );

   // Prevent division by 0.
   if( radiusLength == 0.0 )
   {
      return 0.0;
   }

   // Normalize the radius vector.
   radius /= radiusLength;

   // Use sine to find the standard right-handed angle in degrees.
   radius.setY( asin( radius.y() ) );

   double angle = 0.0;
   
   // 4th Quadrant?
   if( radius.x() >= 0.0 && radius.y() < 0.0 )
   {
      // Y is (-), subtract from 360
      angle = 360.0 + radius.y() * RADS_TO_DEGS_RATIO;
   }
   // 3rd Quadrant?
   else if( radius.x() < 0.0 && radius.y() < 0.0 )
   {
      // Y is (-), add to 180
      angle = 180.0 - radius.y() * RADS_TO_DEGS_RATIO;
   }
   // 2nd Quadrant?
   else if( radius.x() < 0.0 && radius.y() >= 0.0 )
   {
      // Y is (+), subtract from 180
      angle = 180.0 - radius.y() * RADS_TO_DEGS_RATIO;
   }
   // 1st Quadrant
   else
   {
      angle = radius.y() * RADS_TO_DEGS_RATIO;
   }

   return angle;
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::setHandleToAngle( QRect& handle, double angle, double radiusScale )
{
   static const double DEGS_TO_RADS_RATIO = 3.141592653589/180.0;

   // NOTE: Window Y-down is positive, so reverse the value so that value
   // matches that expected of a normal right-handled coordinate system.
   double angleSin = -sin( angle*DEGS_TO_RADS_RATIO )*radiusScale;
   double angleCos = cos( angle*DEGS_TO_RADS_RATIO )*radiusScale;

   // Derive a vector that represents an ellipse radius rotated to
   // the specified angle.
   QPointF radius( (width()-6)*0.5*angleCos, (height()-6)*0.5*angleSin );

   // Find the point on the perimeter of the ellipse by offsetting
   // the widget center point by the derived radius vector.
   // NOTE: The center point is in absolute window coordinates.
   // This results in an absolute window position for the handle.
   QPointF finalPoint( radius + getCenter() );
   // Remove the absolute window offset from the point so that
   // the point is relative to the widget.
   finalPoint -= pos();

   // Center the handle at the new point.
   handle.moveCenter( QPoint( int(finalPoint.x()), int(finalPoint.y()) ) );
}

////////////////////////////////////////////////////////////////////////////////
void RangeRadial::updateHandles()
{
   // Place handles to their final positions
   setHandleToAngle( mHandleStart, mStart );
   setHandleToAngle( mHandleEnd, mEnd+mStart );

   // Place the middle/rotation handle closer to the center
   // so that it will not overlap with the other handles.
   // This will allow the user to rotate the range without
   // having to change the range value to access the handle;
   // this would be the problem if the range is set to 0.
   setHandleToAngle( mHandleMiddle, mEnd*0.5 + mStart, 0.5 );

   update();
}
*/
