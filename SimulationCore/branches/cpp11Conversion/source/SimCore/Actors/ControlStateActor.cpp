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
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/uniqueid.h>
#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/groupactorproperty.h>
#include <dtDAL/namedparameter.h>
#include <dtUtil/log.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <osg/Endian>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // BASE CONTROL TYPE
      //////////////////////////////////////////////////////////////////////////
      const unsigned BaseControl::NAME_LENGTH = 64;

      //////////////////////////////////////////////////////////////////////////
      BaseControl::BaseControl( const std::string& name )
         : dtCore::Base(name),
         mChanged(false)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      BaseControl::~BaseControl()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      std::string BaseControl::GetEncodableName() const
      {
         const std::string& originalName = GetName();
         return originalName.size() > NAME_LENGTH
            ? originalName.substr(0,NAME_LENGTH)
            : originalName;
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseControl::SetChanged( bool changed )
      {
         mChanged = changed;
      }

      //////////////////////////////////////////////////////////////////////////
      bool BaseControl::HasChanged() const
      {
         return mChanged;
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseControl::Encode( char* buffer )
      {
         const std::string& nameOutgoing = GetEncodableName();
         memset( buffer, 0, NAME_LENGTH );
         if( ! nameOutgoing.empty() )
         {
            memcpy( buffer, nameOutgoing.c_str(), nameOutgoing.size() );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseControl::Decode( const char* buffer )
      {
         char nameBuffer[NAME_LENGTH+1];
         memset( nameBuffer, 0, NAME_LENGTH+1 );
         memcpy( nameBuffer, buffer, NAME_LENGTH );

         SetName( std::string(nameBuffer) );
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseControl::Clear()
      {
         SetName("");
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseControl::SetByGroupParameter( const dtDAL::NamedGroupParameter& groupParam )
      {
         SetName( groupParam.GetName() );
      }

      //////////////////////////////////////////////////////////////////////////
      std::shared_ptr<dtDAL::NamedGroupParameter> BaseControl::GetAsGroupParameter() const
      {
         std::shared_ptr<dtDAL::NamedGroupParameter> groupParam
            = new dtDAL::NamedGroupParameter( GetName() );

         return groupParam;
      }



      //////////////////////////////////////////////////////////////////////////
      // CONTINUOUS CONTROL TYPE CODE
      //////////////////////////////////////////////////////////////////////////
      const unsigned ContinuousControl::CONTROL_BYTE_SIZE = BaseControl::NAME_LENGTH
         + sizeof(double)*3;
      const dtUtil::RefString ContinuousControl::PARAM_NAME_VALUE_MIN("MinValue");
      const dtUtil::RefString ContinuousControl::PARAM_NAME_VALUE_MAX("MaxValue");
      const dtUtil::RefString ContinuousControl::PARAM_NAME_VALUE("Value");

      //////////////////////////////////////////////////////////////////////////
      ContinuousControl::ContinuousControl( const std::string& name,
         double minValue, double maxValue, double value )
         : BaseControl(name),
         mMinValue(minValue),
         mMaxValue(maxValue),
         mValue(value)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      ContinuousControl::~ContinuousControl()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::SetMinValue( double minValue )
      {
         SetChanged( mMinValue != minValue );
         mMinValue = minValue;
      }

      //////////////////////////////////////////////////////////////////////////
      double ContinuousControl::GetMinValue() const
      {
         return mMinValue;
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::SetMaxValue( double maxValue )
      {
         SetChanged( mMaxValue != maxValue );
         mMaxValue = maxValue;
      }

      //////////////////////////////////////////////////////////////////////////
      double ContinuousControl::GetMaxValue() const
      {
         return mMaxValue;
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::SetValue( double value )
      {
         SetChanged( mValue != value );
         mValue = value;
      }

      //////////////////////////////////////////////////////////////////////////
      double ContinuousControl::GetValue() const
      {
         return mValue;
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::Encode( char* buffer )
      {
         // Encode the name
         BaseControl::Encode( buffer );
         unsigned bufferOffset = BaseControl::NAME_LENGTH;
         unsigned doubleSize = sizeof(double);

         // Encode the rest
         double tmpMinValue = mMinValue;
         double tmpMaxValue = mMaxValue;
         double tmpValue = mValue;

         // --- Reverse bytes if necessary
         if(osg::getCpuByteOrder() == osg::LittleEndian)
         {
            osg::swapBytes( (char*)&tmpMinValue, doubleSize );
            osg::swapBytes( (char*)&tmpMaxValue, doubleSize );
            osg::swapBytes( (char*)&tmpValue, doubleSize );
         }

         // --- Write to the buffer
         memcpy( &buffer[bufferOffset], &tmpMinValue, doubleSize );
         bufferOffset += doubleSize;

         memcpy( &buffer[bufferOffset], &tmpMaxValue, doubleSize );
         bufferOffset += doubleSize;

         memcpy( &buffer[bufferOffset], &tmpValue, doubleSize );
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::Decode( const char* buffer )
      {
         // Decode the name
         BaseControl::Decode( buffer );
         unsigned bufferOffset = BaseControl::NAME_LENGTH;
         unsigned doubleSize = sizeof(double);

         // Decode the rest
         double tmpMinValue;
         double tmpMaxValue;
         double tmpValue;

         // --- Capture the values
         memcpy( &tmpMinValue, &buffer[bufferOffset], doubleSize );
         bufferOffset += doubleSize;

         memcpy( &tmpMaxValue, &buffer[bufferOffset], doubleSize );
         bufferOffset += doubleSize;

         memcpy( &tmpValue, &buffer[bufferOffset], doubleSize );

         // --- Reverse bytes if necessary
         if(osg::getCpuByteOrder() == osg::LittleEndian)
         {
            osg::swapBytes( (char*)&tmpMinValue, doubleSize );
            osg::swapBytes( (char*)&tmpMaxValue, doubleSize );
            osg::swapBytes( (char*)&tmpValue, doubleSize );
         }

         // --- Assign the resulting values
         mMinValue = tmpMinValue;
         mMaxValue = tmpMaxValue;
         mValue = tmpValue;
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::Clear()
      {
         BaseControl::Clear();
         mMinValue = 0.0;
         mMaxValue = 0.0;
         mValue = 0.0;
      }

      //////////////////////////////////////////////////////////////////////////
      void ContinuousControl::SetByGroupParameter( const dtDAL::NamedGroupParameter& groupParam )
      {
         // Set the name and any other base members
         BaseControl::SetByGroupParameter( groupParam );

         // Set min value
         const dtDAL::NamedFloatParameter* param
            = dynamic_cast<const dtDAL::NamedFloatParameter*>
            (groupParam.GetParameter( PARAM_NAME_VALUE_MIN ));

         if( param != nullptr )
            mMinValue = param->GetValue();

         // Set max value
         param = dynamic_cast<const dtDAL::NamedFloatParameter*>
            (groupParam.GetParameter( PARAM_NAME_VALUE_MAX ));

         if( param != nullptr )
            mMaxValue = param->GetValue();

         // Set value
         param = dynamic_cast<const dtDAL::NamedFloatParameter*>
            (groupParam.GetParameter( PARAM_NAME_VALUE ));

         if( param != nullptr )
            mValue = param->GetValue();
      }

      //////////////////////////////////////////////////////////////////////////
      std::shared_ptr<dtDAL::NamedGroupParameter> ContinuousControl::GetAsGroupParameter() const
      {
         std::shared_ptr<dtDAL::NamedGroupParameter> groupParam
            = BaseControl::GetAsGroupParameter();

         if( ! groupParam.valid() )
            return nullptr;

         // Set min value
         std::shared_ptr<dtDAL::NamedFloatParameter> paramValue
            = new dtDAL::NamedFloatParameter( PARAM_NAME_VALUE_MIN );
         paramValue->SetValue( mMinValue );
         groupParam->AddParameter( *paramValue );

         // Set max value
         paramValue = new dtDAL::NamedFloatParameter( PARAM_NAME_VALUE_MAX );
         paramValue->SetValue( mMaxValue );
         groupParam->AddParameter( *paramValue );

         // Set value
         paramValue = new dtDAL::NamedFloatParameter( PARAM_NAME_VALUE );
         paramValue->SetValue( mValue );
         groupParam->AddParameter( *paramValue );

         return groupParam;
      }



      //////////////////////////////////////////////////////////////////////////
      // DISCRETE CONTROL TYPE CODE
      //////////////////////////////////////////////////////////////////////////
      const unsigned DiscreteControl::CONTROL_BYTE_SIZE = BaseControl::NAME_LENGTH
         + sizeof(unsigned long) + sizeof(long);
      const dtUtil::RefString DiscreteControl::PARAM_NAME_TOTAL_STATES("TotalStates");
      const dtUtil::RefString DiscreteControl::PARAM_NAME_CURRENT_STATE("CurrentState");

      //////////////////////////////////////////////////////////////////////////
      DiscreteControl::DiscreteControl( const std::string& name,
         unsigned long totalStates, long currentState)
         : BaseControl(name),
         mTotalStates(totalStates),
         mCurrentState(currentState)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      DiscreteControl::~DiscreteControl()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void DiscreteControl::SetTotalStates( unsigned long totalStates )
      {
         SetChanged( mTotalStates != totalStates );
         mTotalStates = totalStates;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned long DiscreteControl::GetTotalStates() const
      {
         return mTotalStates;
      }

      //////////////////////////////////////////////////////////////////////////
      void DiscreteControl::SetCurrentState( long state )
      {
         SetChanged( mCurrentState != state );
         mCurrentState = state;
      }

      //////////////////////////////////////////////////////////////////////////
      long DiscreteControl::GetCurrentState() const
      {
         return mCurrentState;
      }

      //////////////////////////////////////////////////////////////////////////
      void DiscreteControl::Encode( char* buffer )
      {
         // Encode the name
         BaseControl::Encode( buffer );
         unsigned bufferOffset = BaseControl::NAME_LENGTH;

         // Encode the rest
         unsigned long totalStates = mTotalStates;
         long currentState = mCurrentState;

         // --- Reverse bytes if necessary
         if(osg::getCpuByteOrder() == osg::LittleEndian)
         {
            osg::swapBytes( (char*)&totalStates, sizeof(totalStates) );
            osg::swapBytes( (char*)&currentState, sizeof(currentState) );
         }

         // --- Write the values to the buffer
         memcpy( &buffer[bufferOffset], &totalStates, sizeof(totalStates) );
         bufferOffset += sizeof(totalStates);

         memcpy( &buffer[bufferOffset], &currentState, sizeof(currentState) );
      }

      //////////////////////////////////////////////////////////////////////////
      void DiscreteControl::Decode( const char* buffer )
      {
         // Decode the name
         BaseControl::Decode( buffer );
         unsigned bufferOffset = BaseControl::NAME_LENGTH;

         // Decode the rest
         unsigned long totalStates;
         long currentState;

         // --- Capture the values from the buffer
         memcpy( &totalStates, &buffer[bufferOffset], sizeof(totalStates) );
         bufferOffset += sizeof(totalStates);

         memcpy( &currentState, &buffer[bufferOffset], sizeof(currentState) );

         // --- Reverse bytes if necessary
         if(osg::getCpuByteOrder() == osg::LittleEndian)
         {
            osg::swapBytes( (char*)&totalStates, sizeof(totalStates) );
            osg::swapBytes( (char*)&currentState, sizeof(currentState) );
         }

         // --- Assign the resulting values
         mTotalStates = totalStates;
         mCurrentState = currentState;
      }

      //////////////////////////////////////////////////////////////////////////
      void DiscreteControl::Clear()
      {
         BaseControl::Clear();
         mTotalStates = 0;
         mCurrentState = 0;
      }

      //////////////////////////////////////////////////////////////////////////
      void DiscreteControl::SetByGroupParameter( const dtDAL::NamedGroupParameter& groupParam )
      {
         // Set the name and any other base members
         BaseControl::SetByGroupParameter( groupParam );

         // Set total states
         const dtDAL::NamedUnsignedIntParameter* paramTotalStates
            = dynamic_cast<const dtDAL::NamedUnsignedIntParameter*>
            (groupParam.GetParameter( PARAM_NAME_TOTAL_STATES ));

         if( paramTotalStates != nullptr )
            mTotalStates = paramTotalStates->GetValue();

         // Set current state
         const dtDAL::NamedIntParameter* paramCurrentState
            = dynamic_cast<const dtDAL::NamedIntParameter*>
            (groupParam.GetParameter( PARAM_NAME_CURRENT_STATE ));

         if( paramCurrentState != nullptr )
            mCurrentState = paramCurrentState->GetValue();
      }

      //////////////////////////////////////////////////////////////////////////
      std::shared_ptr<dtDAL::NamedGroupParameter> DiscreteControl::GetAsGroupParameter() const
      {
         std::shared_ptr<dtDAL::NamedGroupParameter> groupParam
            = BaseControl::GetAsGroupParameter();

         if( ! groupParam.valid() )
            return nullptr;

         // Set total states
         std::shared_ptr<dtDAL::NamedUnsignedIntParameter> paramTotalStates
            = new dtDAL::NamedUnsignedIntParameter( PARAM_NAME_TOTAL_STATES );
         paramTotalStates->SetValue( mTotalStates );
         groupParam->AddParameter( *paramTotalStates );

         // Set current state
         std::shared_ptr<dtDAL::NamedIntParameter> paramCurrentState
            = new dtDAL::NamedIntParameter( PARAM_NAME_CURRENT_STATE );
         paramCurrentState->SetValue( mCurrentState );
         groupParam->AddParameter( *paramCurrentState );

         return groupParam;
      }



      //////////////////////////////////////////////////////////////////////////
      // ENTITY CONTROL STATE CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString ControlStateActor::PARAM_NAME_ENTITY_ID("EntityID");
      const dtUtil::RefString ControlStateActor::PARAM_NAME_STATION_TYPE("StationType");
      const dtUtil::RefString ControlStateActor::PARAM_NAME_NUM_DISCRETE_CONTROLS("NumDiscreteControls");
      const dtUtil::RefString ControlStateActor::PARAM_NAME_NUM_CONTINUOUS_CONTROLS("NumContinuousControls");
      const dtUtil::RefString ControlStateActor::PARAM_NAME_ARRAY_DISCRETE_CONTROLS("ArrayDiscreteControls");
      const dtUtil::RefString ControlStateActor::PARAM_NAME_ARRAY_CONTINUOUS_CONTROLS("ArrayContinuousControls");

      const float ControlStateActor::TIME_BETWEEN_UPDATES(5.0f);

      //////////////////////////////////////////////////////////////////////////
      ControlStateActor::ControlStateActor( ControlStateProxy& proxy )
         : dtGame::GameActor(proxy),
         mChanged(false),
         mNumDiscreteControls(0),
         mNumContinuousControls(0),
         mStationType(0),
         mTimeUntilNextUpdate(TIME_BETWEEN_UPDATES)
      {
         // DEBUG: std::cout << "ControlState: " << GetUniqueId().ToString().c_str() << std::endl;
      }

      //////////////////////////////////////////////////////////////////////////
      ControlStateActor::~ControlStateActor()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::UniqueId& ControlStateActor::GetEntityID() const
      {
         static const dtCore::UniqueId EMPTY("");

         if( mEntity.valid() )
         {
            return mEntity->GetUniqueId();
         }
         return EMPTY;
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::SetEntity( dtDAL::ActorProxy* proxy )
      {
         Platform* oldEntity = mEntity.get();

         if( proxy != nullptr )
         {
            mEntity = dynamic_cast<Platform*>(proxy->GetDrawable());
         }
         else
         {
            mEntity = nullptr;
         }

         // DEBUG:
         //if( proxy != nullptr )
         //   std::cout << "ControlState: " << GetUniqueId() << " -> " << proxy->GetId() << std::endl;

         mChanged = mEntity.get() != oldEntity;
      }

      //////////////////////////////////////////////////////////////////////////
      Platform* ControlStateActor::GetEntity()
      {
         return mEntity.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::SetStationType( int stationType )
      {
         mChanged = mStationType != stationType;
         mStationType = stationType;
      }

      //////////////////////////////////////////////////////////////////////////
      int ControlStateActor::GetStationType() const
      {
         return mStationType;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned ControlStateActor::GetContinuousControlCount() const
      {
         return mContinuousTypes.size();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned ControlStateActor::GetDiscreteControlCount() const
      {
         return mDiscreteTypes.size();
      }

      //////////////////////////////////////////////////////////////////////////
      const std::map<const std::string, std::shared_ptr<DiscreteControl> >&
         ControlStateActor::GetDiscreteControls() const
      {
         return mDiscreteTypes;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::map<const std::string, std::shared_ptr<ContinuousControl> >&
         ControlStateActor::GetContinuousControls() const
      {
         return mContinuousTypes;
      }

      //////////////////////////////////////////////////////////////////////////
      bool ControlStateActor::AddControl( std::shared_ptr<BaseControl>& control )
      {
         if( ! control.valid() )
            return false;

         if( dynamic_cast<DiscreteControl*>(control.get()) != nullptr )
         {
            return AddControlToMap( static_cast<DiscreteControl&>(*control), mDiscreteTypes );
         }
         else if( dynamic_cast<ContinuousControl*>(control.get()) != nullptr )
         {
            return AddControlToMap( static_cast<ContinuousControl&>(*control), mContinuousTypes );
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool ControlStateActor::AddControl( std::shared_ptr<DiscreteControl>& control )
      {
         if( ! control.valid() )
            return false;

         return AddControlToMap( *control, mDiscreteTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      bool ControlStateActor::AddControl( std::shared_ptr<ContinuousControl>& control )
      {
         if( ! control.valid() )
            return false;

         return AddControlToMap( *control, mContinuousTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      bool ControlStateActor::RemoveControl( BaseControl* control )
      {
         if( control == nullptr )
            return false;

         if( dynamic_cast<DiscreteControl*>(control) != nullptr )
         {
            return RemoveControlFromMap( control->GetEncodableName(), mDiscreteTypes );
         }
         else if( dynamic_cast<ContinuousControl*>(control) != nullptr )
         {
            return RemoveControlFromMap( control->GetEncodableName(), mContinuousTypes );
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool ControlStateActor::RemoveDiscreteControl( const std::string& controlName )
      {
         return RemoveControlFromMap( controlName, mDiscreteTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      bool ControlStateActor::RemoveContinuousControl( const std::string& controlName )
      {
         return RemoveControlFromMap( controlName, mContinuousTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      DiscreteControl* ControlStateActor::GetDiscreteControl( const std::string& controlName )
      {
         return GetControlFromMap( controlName, mDiscreteTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      const DiscreteControl* ControlStateActor::GetDiscreteControl( const std::string& controlName ) const
      {
         return GetControlFromMap( controlName, mDiscreteTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      ContinuousControl* ControlStateActor::GetContinuousControl( const std::string& controlName )
      {
         return GetControlFromMap( controlName, mContinuousTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      const ContinuousControl* ControlStateActor::GetContinuousControl( const std::string& controlName ) const
      {
         return GetControlFromMap( controlName, mContinuousTypes );
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::Clear()
      {
         mEntity = nullptr;
         mStationType = 0;
         mContinuousTypes.clear();
         mDiscreteTypes.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::SetNumDiscreteControls( int numControls )
      {
         mChanged = mNumDiscreteControls != numControls;
         mNumDiscreteControls = numControls;
      }

      //////////////////////////////////////////////////////////////////////////
      int ControlStateActor::GetNumDiscreteControls() const
      {
         return mNumDiscreteControls;
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::SetNumContinuousControls( int numControls )
      {
         mChanged = mNumContinuousControls != numControls;
         mNumContinuousControls = numControls;
      }

      //////////////////////////////////////////////////////////////////////////
      int ControlStateActor::GetNumContinuousControls() const
      {
         return mNumContinuousControls;
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::SetDiscreteControlsByGroupParameter( const dtDAL::NamedGroupParameter& groupParam )
      {
         SetMapByGroupParameter( mDiscreteTypes, groupParam );
      }

      //////////////////////////////////////////////////////////////////////////
      std::shared_ptr<dtDAL::NamedGroupParameter> ControlStateActor::GetDiscreteControlsAsGroupParameter() const
      {
         return GetMapAsGroupParameter( mDiscreteTypes, PARAM_NAME_ARRAY_DISCRETE_CONTROLS );
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::SetContinuousControlsByGroupParameter( const dtDAL::NamedGroupParameter& groupParam )
      {
         SetMapByGroupParameter( mContinuousTypes, groupParam );
      }

      //////////////////////////////////////////////////////////////////////////
      std::shared_ptr<dtDAL::NamedGroupParameter> ControlStateActor::GetContinuousControlsAsGroupParameter() const
      {
         return GetMapAsGroupParameter( mContinuousTypes, PARAM_NAME_ARRAY_CONTINUOUS_CONTROLS );
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         mTimeUntilNextUpdate -= tickMessage.GetDeltaSimTime();

         // Periodically send out an update for our control states so new people on the network will get them.
         if (mTimeUntilNextUpdate < 0.0)
         {
            GetGameActorProxy().NotifyFullActorUpdate();
            mTimeUntilNextUpdate = TIME_BETWEEN_UPDATES;
         }
      }


      //////////////////////////////////////////////////////////////////////////
      // ENTITY CONTROL STATE PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString ControlStateProxy::CLASS_NAME("SimCore::Actors::ControlStateActor");

      //////////////////////////////////////////////////////////////////////////
      ControlStateProxy::ControlStateProxy()
      {
         SetClassName( ControlStateProxy::CLASS_NAME );
         SetHideDTCorePhysicsProps(true);
      }

      //////////////////////////////////////////////////////////////////////////
      ControlStateProxy::~ControlStateProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateProxy::OnEnteredWorld()
      {
         if (!IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         }

      }

      //////////////////////////////////////////////////////////////////////////
      void ControlStateProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         ControlStateActor* actor = nullptr;
         GetActor(actor);

         AddProperty(new dtDAL::ActorActorProperty( *this, "EntityID", "EntityID",
            dtDAL::ActorActorProperty::SetFuncType( actor, &ControlStateActor::SetEntity ),
            dtDAL::ActorActorProperty::GetFuncType( ),
            "SimCore::Actors::Platform",
            "Entity to which the control state points and modifies"));

         AddProperty(new dtDAL::IntActorProperty( "StationType", "StationType",
            dtDAL::IntActorProperty::SetFuncType( actor, &ControlStateActor::SetStationType ),
            dtDAL::IntActorProperty::GetFuncType( actor, &ControlStateActor::GetStationType ),
            "Station type of the controls state"));

         AddProperty(new dtDAL::IntActorProperty( "NumDiscreteControls", "NumDiscreteControls",
            dtDAL::IntActorProperty::SetFuncType( actor, &ControlStateActor::SetNumDiscreteControls ),
            dtDAL::IntActorProperty::GetFuncType( actor, &ControlStateActor::GetNumDiscreteControls ),
            "The expected number of elements in the discrete controls array"));

         AddProperty(new dtDAL::GroupActorProperty( "DiscreteControlsArray", "DiscreteControlsArray",
            dtDAL::GroupActorProperty::SetFuncType( actor, &ControlStateActor::SetDiscreteControlsByGroupParameter ),
            dtDAL::GroupActorProperty::GetFuncType( actor, &ControlStateActor::GetDiscreteControlsAsGroupParameter ),
            "", // Description - unknown use
            "Arrays"));

         AddProperty(new dtDAL::IntActorProperty( "NumContinuousControls", "NumContinuousControls",
            dtDAL::IntActorProperty::SetFuncType( actor, &ControlStateActor::SetNumContinuousControls ),
            dtDAL::IntActorProperty::GetFuncType( actor, &ControlStateActor::GetNumContinuousControls ),
            "The expected number of elements in the continuous controls array"));

         AddProperty(new dtDAL::GroupActorProperty( "ContinuousControlsArray", "ContinuousControlsArray",
            dtDAL::GroupActorProperty::SetFuncType( actor, &ControlStateActor::SetContinuousControlsByGroupParameter ),
            dtDAL::GroupActorProperty::GetFuncType( actor, &ControlStateActor::GetContinuousControlsAsGroupParameter ),
            "", // Description - unknown use
            "Arrays"));
      }

   }
}
