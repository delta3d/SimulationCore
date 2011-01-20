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

#ifndef _ENTITY_CONTROL_STATE_H_
#define _ENTITY_CONTROL_STATE_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtCore/base.h>
#include <dtCore/observerptr.h>
#include <dtCore/refptr.h>
#include <dtGame/gameactor.h>
#include <dtDAL/namedgroupparameter.h>
#include <SimCore/Actors/Platform.h>


////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class UniqueId;
   class DeltaDrawable;
}

namespace dtDAL
{
   class NamedGroupParameter;
}

namespace SimCore
{
   namespace Actors
   {
      class ControlStateProxy;

      //////////////////////////////////////////////////////////////////////////
      // BASE CONTROL TYPE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BaseControl : public dtCore::Base
      {
         public:
            static const unsigned NAME_LENGTH;

            BaseControl( const std::string& name = "" );

            // @return The name of this object clamped to the
            // name size limit, NAME_LENGTH.
            std::string GetEncodableName() const;

            void SetChanged( bool changed );
            bool HasChanged() const;

            virtual void Encode( char* buffer );
            virtual void Decode( const char* buffer );

            virtual void Clear();

            virtual void SetByGroupParameter( const dtDAL::NamedGroupParameter& groupParam );
            virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> GetAsGroupParameter() const;

         protected:
            virtual ~BaseControl();

         private:
            bool mChanged;
      };

      //////////////////////////////////////////////////////////////////////////
      // CONTINUOUS CONTROL STATE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ContinuousControl : public BaseControl
      {
         public:
            static const unsigned CONTROL_BYTE_SIZE;
            static const dtUtil::RefString PARAM_NAME_VALUE_MIN;
            static const dtUtil::RefString PARAM_NAME_VALUE_MAX;
            static const dtUtil::RefString PARAM_NAME_VALUE;

            ContinuousControl( const std::string& name = "",
               double minValue = 0.0, double maxValue = 0.0, double value = 0.0 );

            void SetMinValue( double minValue );
            double GetMinValue() const;

            void SetMaxValue( double maxValue );
            double GetMaxValue() const;

            void SetValue( double value );
            double GetValue() const;

            unsigned GetByteSize() const { return CONTROL_BYTE_SIZE; }

            virtual void Encode( char* buffer );
            virtual void Decode( const char* buffer );

            virtual void Clear();

            virtual void SetByGroupParameter( const dtDAL::NamedGroupParameter& groupParam );
            virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> GetAsGroupParameter() const;

         protected:
            virtual ~ContinuousControl();

         private:
            double mMinValue;
            double mMaxValue;
            double mValue;
      };



      //////////////////////////////////////////////////////////////////////////
      // DISCRETE CONTROL TYPE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DiscreteControl : public BaseControl
      {
         public:
            static const unsigned CONTROL_BYTE_SIZE;
            static const dtUtil::RefString PARAM_NAME_TOTAL_STATES;
            static const dtUtil::RefString PARAM_NAME_CURRENT_STATE;

            DiscreteControl( const std::string& name = "", unsigned long totalStates = 0, long currentState = 0 );

            void SetTotalStates( unsigned long totalStates );
            unsigned long GetTotalStates() const;

            void SetCurrentState( long state );
            long GetCurrentState() const;

            unsigned GetByteSize() const { return CONTROL_BYTE_SIZE; }

            virtual void Encode( char* buffer );
            virtual void Decode( const char* buffer );

            virtual void Clear();

            virtual void SetByGroupParameter( const dtDAL::NamedGroupParameter& groupParam );
            virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> GetAsGroupParameter() const;

         protected:
            virtual ~DiscreteControl();

         private:
            unsigned long mTotalStates;
            long          mCurrentState;
      };



      //////////////////////////////////////////////////////////////////////////
      // ENTITY CONTROL STATE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ControlStateActor : public dtGame::GameActor
      {
         public:
            static const dtUtil::RefString PARAM_NAME_ENTITY_ID;
            static const dtUtil::RefString PARAM_NAME_STATION_TYPE;
            static const dtUtil::RefString PARAM_NAME_NUM_DISCRETE_CONTROLS;
            static const dtUtil::RefString PARAM_NAME_NUM_CONTINUOUS_CONTROLS;
            static const dtUtil::RefString PARAM_NAME_ARRAY_DISCRETE_CONTROLS;
            static const dtUtil::RefString PARAM_NAME_ARRAY_CONTINUOUS_CONTROLS;

            static const float TIME_BETWEEN_UPDATES;//(10.0f);

            ControlStateActor( ControlStateProxy& proxy );

            const dtCore::UniqueId& GetEntityID() const;

            void SetEntity( dtDAL::ActorProxy* proxy );
            dtCore::DeltaDrawable* GetEntity();

            void SetStationType( int stationType );
            int GetStationType() const;

            const std::map<const std::string, dtCore::RefPtr<DiscreteControl> >&
               GetDiscreteControls() const;
            const std::map<const std::string, dtCore::RefPtr<ContinuousControl> >&
               GetContinuousControls() const;

            unsigned GetContinuousControlCount() const;
            unsigned GetDiscreteControlCount() const;

            bool AddControl( dtCore::RefPtr<BaseControl>& control );
            bool AddControl( dtCore::RefPtr<DiscreteControl>& control );
            bool AddControl( dtCore::RefPtr<ContinuousControl>& control );

            bool RemoveControl( BaseControl* control );
            bool RemoveDiscreteControl( const std::string& controlName );
            bool RemoveContinuousControl( const std::string& controlName );

            DiscreteControl* GetDiscreteControl( const std::string& controlName );
            const DiscreteControl* GetDiscreteControl( const std::string& controlName ) const;

            ContinuousControl* GetContinuousControl( const std::string& controlName );
            const ContinuousControl* GetContinuousControl( const std::string& controlName ) const;

            // Mutator for use by functor in a proxy property map.
            // Do NOT set this manually as it could affect out-going data integrity.
            void SetNumDiscreteControls( int numControls );
            int GetNumDiscreteControls() const;

            // Mutator for use by functor in a proxy property map.
            // Do NOT set this manually as it could affect out-going data integrity.
            void SetNumContinuousControls( int numControls );
            int GetNumContinuousControls() const;

            void SetDiscreteControlsByGroupParameter( const dtDAL::NamedGroupParameter& groupParam );
            dtCore::RefPtr<dtDAL::NamedGroupParameter> GetDiscreteControlsAsGroupParameter() const;

            void SetContinuousControlsByGroupParameter( const dtDAL::NamedGroupParameter& groupParam );
            dtCore::RefPtr<dtDAL::NamedGroupParameter> GetContinuousControlsAsGroupParameter() const;

            void Clear();

            // Periodically do a full actor publish
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

         protected:
            virtual ~ControlStateActor();

            // NOTE: The following functions are protected in case a subclass
            // needs to operate on new maps for new control types.
            template<class baseType>
            bool AddControlToMap( baseType& control,
               std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap );

            template<class baseType>
            bool RemoveControlFromMap( const std::string& controlName,
               std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap );

            template<class baseType>
            baseType* GetControlFromMap( const std::string& controlName,
               std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap );

            template<class baseType>
            const baseType* GetControlFromMap( const std::string& controlName,
               const std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap ) const;

            template<class baseType>
            dtCore::RefPtr<dtDAL::NamedGroupParameter> GetMapAsGroupParameter(
               const std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap,
               const std::string& groupName ) const;

            template<class baseType>
            void SetMapByGroupParameter(
               std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap,
               const dtDAL::NamedGroupParameter& groupParam );

         private:
            bool mChanged;
            int mNumDiscreteControls;   // used to verify the expected discrete control array length
            int mNumContinuousControls; // used to verify the expected continuous control array length
            int mStationType;
            dtCore::ObserverPtr<Platform> mEntity; // direct reference to the entity
            std::map<const std::string, dtCore::RefPtr<ContinuousControl> > mContinuousTypes;
            std::map<const std::string, dtCore::RefPtr<DiscreteControl> > mDiscreteTypes;
            float mTimeUntilNextUpdate;
      };



      //////////////////////////////////////////////////////////////////////////
      // ENTITY CONTROL STATE PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ControlStateProxy : public dtGame::GameActorProxy
      {
         public:
            static const dtUtil::RefString CLASS_NAME;

            ControlStateProxy();

            // Creates the associated actor
            virtual void CreateActor() { SetActor(*new ControlStateActor(*this)); }

            // Adds the properties associated with this actor
            virtual void BuildPropertyMap();

            // Register for messages
            virtual void OnEnteredWorld();


         protected:

            // Destructor
            virtual ~ControlStateProxy();
      };



      //////////////////////////////////////////////////////////////////////////
      // TEMPLATE FUNCTION DEFINITIONS
      //////////////////////////////////////////////////////////////////////////
      template<class baseType>
      bool ControlStateActor::AddControlToMap(
         baseType& control, std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap )
      {
         if (controlMap.insert( std::make_pair( control.GetEncodableName(), &control ) ).second)
         {
            mChanged = true;
            return true;
         }

         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      template<class baseType>
      bool ControlStateActor::RemoveControlFromMap( const std::string& controlName,
         std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap )
      {
         if (controlMap.erase( controlName ))
         {
            mChanged = true;
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      template<class baseType>
      baseType* ControlStateActor::GetControlFromMap(
         const std::string& controlName,
         std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap )
      {
         typedef typename std::map<const std::string, dtCore::RefPtr<baseType> >::iterator iterType;
         iterType iter = controlMap.find( controlName );

         return iter != controlMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      template<class baseType>
      const baseType* ControlStateActor::GetControlFromMap(
         const std::string& controlName,
         const std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap ) const
      {
         typedef typename std::map<const std::string, dtCore::RefPtr<baseType> >::const_iterator constIterType;
         constIterType iter = controlMap.find( controlName );

         return iter != controlMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      template<class baseType>
      dtCore::RefPtr<dtDAL::NamedGroupParameter> ControlStateActor::GetMapAsGroupParameter(
         const std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap,
         const std::string& groupName ) const
      {
         if( controlMap.empty() )
            return NULL;

         // Create the group parameter that will represent the control array (map)
         dtCore::RefPtr<dtDAL::NamedGroupParameter> controlArrayParams
            = new dtDAL::NamedGroupParameter( groupName );

         // Iterate through the control map and acquire a group parameter representation
         // of each control and add it to the new group parameter that represents the array.
         baseType* currentControl = NULL;
         typedef typename std::map<const std::string, dtCore::RefPtr<baseType> >::const_iterator constIterType;
         constIterType iter = controlMap.begin();
         for( ; iter != controlMap.end(); ++iter )
         {
            currentControl = iter->second.get();

            if( currentControl != NULL )
            {
               // Add the group parameter that represents the individual control
               // to the main array group parameter.
               dtCore::RefPtr<dtDAL::NamedGroupParameter> controlParam = currentControl->GetAsGroupParameter();
               if( controlParam.valid() )
                  controlArrayParams->AddParameter( *controlParam );
            }
         }

         return controlArrayParams;
      }

      //////////////////////////////////////////////////////////////////////////
      template<class baseType>
      void ControlStateActor::SetMapByGroupParameter(
         std::map<const std::string, dtCore::RefPtr<baseType> >& controlMap,
         const dtDAL::NamedGroupParameter& groupParam )
      {
         std::vector<const dtDAL::NamedParameter*> params;
         groupParam.GetParameters( params );

         // Visit each nested group parameter and set their values to a control
         // that shares the same name.
         const dtDAL::NamedGroupParameter* currentParam = NULL;
         const unsigned limit = params.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            currentParam = dynamic_cast<const dtDAL::NamedGroupParameter*>(params[i]);
            if( currentParam != NULL )
            {
               baseType* control = GetControlFromMap( currentParam->GetName(), controlMap );
               if( control != NULL )
               {
                  control->SetByGroupParameter( *currentParam );
               }
               else
               {
                  dtCore::RefPtr<BaseControl> newControl = new baseType;
                  newControl->SetByGroupParameter( *currentParam );
                  if ( ! AddControl( newControl ) )
                  {
                     std::string warningStr = "Unable to add control \"" + currentParam->GetName() + "\" to " + newControl->GetName();
                     LOG_WARNING( warningStr );
                  }
               }
            }

            currentParam = NULL;
         }
      }
   }
}

#endif
