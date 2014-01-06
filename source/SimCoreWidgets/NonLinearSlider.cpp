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
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QSlider>
#include <SimCoreWidgets/NonLinearSlider.h>



////////////////////////////////////////////////////////////////////////////////
// CUSTOM WIDGET CODE
////////////////////////////////////////////////////////////////////////////////
QDoubleSpinBoxExtended::QDoubleSpinBoxExtended( QWidget* parent )
   : QDoubleSpinBox(parent)
{
}

////////////////////////////////////////////////////////////////////////////////
void QDoubleSpinBoxExtended::stepBy( int steps )
{
   QDoubleSpinBox::stepBy( steps );
   if( steps != 0 )
   {
      signalStepped();
   }
}



////////////////////////////////////////////////////////////////////////////////
// CUSTOM WIDGET CODE
////////////////////////////////////////////////////////////////////////////////
NonLinearSlider::NonLinearSlider( QWidget* parent )
   : QWidget(parent)
   , mLastSlidePosition(0)
   , mLastValue(0.0)
   , mMiddleValue(50.0)
   , mSlider(nullptr)
   , mSpinBox(nullptr)
   , mLabelMin(nullptr)
   , mLabelMax(nullptr)
   , mLabelMiddle(nullptr)
{
   setMinimumSize( QSize(128,32) );

   // Create the sub-widgets.
   mSlider = new QSlider;
   mSpinBox = new QDoubleSpinBoxExtended;

   // Set default values on the sub-widgets
   mSlider->setOrientation( Qt::Horizontal );
   mSlider->setRange( 0, 1000 );
   mSpinBox->setRange( 0.0, 100.0 );

   // Create the label widgets.
   mLabelMin = new QLabel;
   mLabelMax = new QLabel;
   mLabelMiddle = new QLabel;
   setLabelValue( *mLabelMin, getMinValue() );
   setLabelValue( *mLabelMax, getMaxValue() );
   setLabelValue( *mLabelMiddle, mMiddleValue );
   mLabelMin->setAlignment( Qt::AlignLeft );
   mLabelMax->setAlignment( Qt::AlignRight );
   mLabelMiddle->setAlignment( Qt::AlignCenter );

   // Connect the signals and slots of both the spin box and the slider.
   // ---  This Object's Signals
   QObject::connect(
      this, SIGNAL(changedValue(double)),
      mSpinBox, SLOT(setValue(double)) );

   // --- Slider Signals
   QObject::connect(
      mSlider, SIGNAL(valueChanged(int)),
      this, SLOT(setValueInteger(int)) );
   QObject::connect(
      mSlider, SIGNAL(rangeChanged(int,int)),
      this, SLOT(onSliderRangeChanged(int,int)) );

   // --- Spin Box Signals
   QObject::connect(
      mSpinBox, SIGNAL(editingFinished()),
      this, SLOT(onSpinBoxEditFinished()) );
   QObject::connect(
      mSpinBox, SIGNAL(signalStepped()),
      this, SLOT(onSpinBoxEditFinished()) );

   // Create the primary layout.
   QHBoxLayout* layoutLabels = new QHBoxLayout;
   QVBoxLayout* layoutSliderAndLabels = new QVBoxLayout;
   layoutSliderAndLabels->addWidget(mSlider);
   layoutLabels->addWidget(mLabelMin);
   layoutLabels->addWidget(mLabelMiddle);
   layoutLabels->addWidget(mLabelMax);
   layoutSliderAndLabels->addLayout(layoutLabels);

   // Attach the sub-widgets.
   QHBoxLayout* layoutMain = new QHBoxLayout;
   layoutMain->addLayout(layoutSliderAndLabels);
   layoutMain->addWidget(mSpinBox);

   // Finally attach the main layout
   setLayout(layoutMain);

   // Set default value on spinner.
   mSpinBox->setValue(0);
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::onCreated()
{
}

////////////////////////////////////////////////////////////////////////////////
int NonLinearSlider::getResolution() const
{
   return mSlider->maximum();
}

////////////////////////////////////////////////////////////////////////////////
double NonLinearSlider::getValue() const
{
   return mSpinBox->value();
}

////////////////////////////////////////////////////////////////////////////////
double NonLinearSlider::getMinValue() const
{
   return mSpinBox->minimum();
}

////////////////////////////////////////////////////////////////////////////////
double NonLinearSlider::getMaxValue() const
{
   return mSpinBox->maximum();
}

////////////////////////////////////////////////////////////////////////////////
double NonLinearSlider::getMiddleValue() const
{
   return mMiddleValue;
}

////////////////////////////////////////////////////////////////////////////////
double NonLinearSlider::getRangeSize() const
{
   return mSpinBox->maximum() - mSpinBox->minimum();
}

////////////////////////////////////////////////////////////////////////////////
QSlider& NonLinearSlider::getSlider()
{
   return *mSlider;
}

////////////////////////////////////////////////////////////////////////////////
QDoubleSpinBox& NonLinearSlider::getSpinBox()
{
   return *mSpinBox;
}

////////////////////////////////////////////////////////////////////////////////
double NonLinearSlider::getValueFromSliderPosition( int slidePosition ) const
{
   double value = 0.0;
   double resolution = double(getResolution());

   if( resolution != 0 )
   {
      double ratio = double(slidePosition) / (resolution);
      if( ratio > 0.5 )
      {
         // Clip and expand ratio for the higher range.
         ratio = (ratio - 0.5) * 2.0;

         // Interpolate between the mid and max values to get the final value.
         value = (getMaxValue() - mMiddleValue) * ratio + mMiddleValue;
      }
      else
      {
         // Expand the ratio for the lower range.
         ratio *= 2.0;

         // Interpolate between the min and mid values to get the final value.
         value = (mMiddleValue - getMinValue()) * ratio + getMinValue();
      }
   }
   
   return value;
}

////////////////////////////////////////////////////////////////////////////////
int NonLinearSlider::getSliderPositionFromValue( double spinBoxValue ) const
{
   int slideValue = 0;
   double resolution = double(getResolution());
   
   if( resolution != 0 )
   {
      double ratio = 0.0;

      if( spinBoxValue > mMiddleValue )
      {
         double diff = getMaxValue() - mMiddleValue;
         if( diff != 0.0 )
         {
            ratio = ((spinBoxValue - mMiddleValue)/diff) * 0.5 + 0.5;
         }
         else // Middle value must be the same as max.
         {
            ratio = 1.0;
         }
      }
      else
      {
         double diff = mMiddleValue - getMinValue();
         if( diff != 0.0 )
         {
            ratio = (spinBoxValue-mMiddleValue)/diff + 1.0;
            if( ratio < 0.0 )
            {
               ratio += 1.0;
            }
            ratio *= 0.5;
         }
         else // Middle value must be the same as min.
         {
            ratio = 0.0;
         }
      }
      slideValue = int(double(getResolution()) * ratio);
   }

   return slideValue;
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setLabelValue( QLabel& label, double value )
{
   QString stringValue;
   stringValue.setNum( value );
   label.setText( stringValue );
}



////////////////////////////////////////////////////////////////////////////////
// SLOTS
////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setMinValue( double minValue )
{
   if( getMinValue() != minValue )
   {
      mSpinBox->setMinimum( minValue );
      setLabelValue( *mLabelMin, minValue );
      emit changedRange( mSpinBox->minimum(), mSpinBox->maximum() );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setMaxValue( double maxValue )
{
   if( getMaxValue() != maxValue )
   {
      mSpinBox->setMaximum( maxValue );
      setLabelValue( *mLabelMax, maxValue );
      emit changedRange( mSpinBox->minimum(), mSpinBox->maximum() );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setMiddleValue( double midValue )
{
   if( mMiddleValue != midValue )
   {
      mMiddleValue = midValue;
      setLabelValue( *mLabelMiddle, midValue );
      emit changedMiddleValue( midValue );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setResolution( int resolution )
{
   if( mSlider->maximum() != resolution )
   {
      mSlider->setRange( 0, resolution );
      emit changedResolution( resolution );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setRange( double minValue, double maxValue )
{
   if( mSpinBox->minimum() != minValue || mSpinBox->maximum() != maxValue )
   {
      mSpinBox->setMinimum( minValue );
      mSpinBox->setMaximum( maxValue );
      emit changedRange( mSpinBox->minimum(), mSpinBox->maximum() );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setValue( double value )
{
   if( mLastValue != value )
   {
      mLastValue = value;
      mSlider->setValue( getSliderPositionFromValue( mLastValue ) );

      mLastSlidePosition = mSlider->value();

      // NOTE: The spin box will be updated with the value after this signal is emitted.
      emit changedValue( mLastValue );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::setValueInteger( int sliderValue )
{
   // Update the slider in case the value is set directly on this widget.
   if( mLastSlidePosition != sliderValue )
   {
      mLastSlidePosition = sliderValue;
      mSlider->setValue( sliderValue );

      // Update the spin box.
      mSpinBox->setValue( getValueFromSliderPosition( sliderValue ) );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::onSliderRangeChanged( int minValue, int maxValue )
{
   // Ensure the slider remains in a positive range.
   if( mSlider->minimum() != 0 || mSlider->maximum() != maxValue )
   {
      maxValue -= minValue;
      if( maxValue < 0 )
      {
         maxValue = -maxValue;
      }
      
      setResolution( maxValue );
   }
}

////////////////////////////////////////////////////////////////////////////////
void NonLinearSlider::onSpinBoxEditFinished()
{
   setValue( mSpinBox->value() );
}
*/
