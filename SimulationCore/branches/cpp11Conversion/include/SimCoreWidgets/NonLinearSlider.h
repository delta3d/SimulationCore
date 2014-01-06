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

#ifndef NONLINEAR_SLIDER_H
#define NONLINEAR_SLIDER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QWidget>
#include <QtDesigner/QDesignerExportWidget>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
class QLabel;
class QSlider;



////////////////////////////////////////////////////////////////////////////////
// CUSTOM WIDGET CODE
////////////////////////////////////////////////////////////////////////////////
class QDESIGNER_WIDGET_EXPORT QDoubleSpinBoxExtended : public QDoubleSpinBox
{
   Q_OBJECT

   public:
      QDoubleSpinBoxExtended( QWidget* parent = nullptr );

      virtual void stepBy( int steps );

   signals:
      void signalStepped();
};



////////////////////////////////////////////////////////////////////////////////
// CUSTOM WIDGET CODE
////////////////////////////////////////////////////////////////////////////////
class QDESIGNER_WIDGET_EXPORT NonLinearSlider : public QWidget
{
   Q_OBJECT
      Q_PROPERTY( int resolution READ getResolution WRITE setResolution )
      Q_PROPERTY( double minValue READ getMinValue WRITE setMinValue )
      Q_PROPERTY( double maxValue READ getMaxValue WRITE setMaxValue )
      Q_PROPERTY( double middleValue READ getMiddleValue WRITE setMiddleValue )
      Q_PROPERTY( double value READ getValue WRITE setValue )

   public:
      NonLinearSlider( QWidget* parent = nullptr );

      void onCreated();

      void setResolution( int resolution );
      int getResolution() const;

      double getValue() const;

      double getMinValue() const;

      double getMaxValue() const;

      double getMiddleValue() const;

      double getRangeSize() const;

      QSlider& getSlider();

      QDoubleSpinBox& getSpinBox();

   signals:
      void changedResolution( int resolution );

      void changedRange( double minValue, double maxValue );

      void changedValue( double value );

      void changedMiddleValue( double midValue );

   public slots:
      void setMinValue( double minValue );
      
      void setMaxValue( double maxValue );

      void setMiddleValue( double midValue );

      void setRange( double minValue, double maxValue );

      void setValue( double value );

   protected slots:
      void setValueInteger( int sliderValue );

      void onSliderRangeChanged( int minValue, int maxValue );

      void onSpinBoxEditFinished();

   protected:
      double getValueFromSliderPosition( int slidePosition ) const;

      int getSliderPositionFromValue( double spinBoxValue ) const;

      void setLabelValue( QLabel& label, double value );

   private:
      int mLastSlidePosition;
      double mLastValue;
      double mMiddleValue;
      QSlider* mSlider;
      QDoubleSpinBoxExtended* mSpinBox;
      QLabel* mLabelMin;
      QLabel* mLabelMax;
      QLabel* mLabelMiddle;
};
*/
#endif
