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
 * @author Curtiss Murphy
 */

#include <prefix/SimCorePrefix.h>
#include <dtGame/gameactor.h>
#include <dtDAL/enginepropertytypes.h>
#include <SimCore/Actors/DynamicLightPrototypeActor.h>

#include <dtDAL/functor.h> // deprecated

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(DynamicLightType);
      DynamicLightType DynamicLightType::UNKNOWN("Unknown");
      DynamicLightType DynamicLightType::FIRE("Fire");
      DynamicLightType DynamicLightType::TRACER("Tracer");
      DynamicLightType DynamicLightType::HEADLIGHT("Headlights");
      DynamicLightType DynamicLightType::FLARE("Flare");
      DynamicLightType DynamicLightType::WEAPONS_FLASH("Weapons - Flash");
      DynamicLightType DynamicLightType::GENERIC_SMALL("Generic - Small");
      DynamicLightType DynamicLightType::GENERIC_MEDIUM("Generic - Medium");
      DynamicLightType DynamicLightType::GENERIC_LARGE("Generic - Large");
      DynamicLightType DynamicLightType::OTHER("Other");

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      DynamicLightPrototypeProxy::DynamicLightPrototypeProxy()
      {
         SetClassName("SimCore::Actors::DynamicLightPrototypeActor");
         SetInitialOwnership(dtGame::GameActorProxy::Ownership::PROTOTYPE);
      }

      //////////////////////////////////////////////////////////
      DynamicLightPrototypeProxy::~DynamicLightPrototypeProxy()
      {

      }

      void DynamicLightPrototypeProxy::CreateActor()
      {
         SetActor(*new DynamicLightPrototypeActor(*this)); 
      }

      //////////////////////////////////////////////////////////
      void DynamicLightPrototypeProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         DynamicLightPrototypeActor* actor = static_cast<DynamicLightPrototypeActor*>(GetActor());

         static const dtUtil::RefString LIGHT_GROUP("Dynamic Light");

         static const dtUtil::RefString PROPERTY_INTENSITY("Intensity");
         static const dtUtil::RefString PROPERTY_INTENSITY_DESC("A multiplier for the effect of the light. Can be used to disable or enable a light. Typically 0 or 1 (default). " \
            "Value also used for NVG effects - ex 1000 used for bright flares.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_INTENSITY, PROPERTY_INTENSITY, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetIntensity),
            dtDAL::FloatActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetIntensity),
            PROPERTY_INTENSITY_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_ATTENUATION("Attenuation");
         static const dtUtil::RefString PROPERTY_ATTENUATION_DESC("Controls how far the light is visible. The vec3 represents constant(X), linear(Y), quadratic(Z) attenuations");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_ATTENUATION, PROPERTY_ATTENUATION,
            dtDAL::Vec3ActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetAttenuation),
            dtDAL::Vec3ActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetAttenuation),
            PROPERTY_ATTENUATION_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_LIGHTCOLOR("Light Color");
         static const dtUtil::RefString PROPERTY_LIGHTCOLOR_DESC("The base color of this light");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_LIGHTCOLOR, PROPERTY_LIGHTCOLOR,
            dtDAL::Vec3ActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetLightColor),
            dtDAL::Vec3ActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetLightColor),
            PROPERTY_LIGHTCOLOR_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_FLICKERSCALE("Flicker Scale");
         static const dtUtil::RefString PROPERTY_FLICKERSCALE_DESC("Indicates if the light should flicker and how much. (value is the max range of variation - ex 0.1 to 0.4). 0.0 means no flicker.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_FLICKERSCALE, PROPERTY_FLICKERSCALE, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetFlickerScale),
            dtDAL::FloatActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetFlickerScale),
            PROPERTY_FLICKERSCALE_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_MAXIMUMTIME("Maximum Time");
         static const dtUtil::RefString PROPERTY_MAXIMUMTIME_DESC("The maximum time (in seconds) before this light will begin to fade out (see Fade Out Time). Use 0 to specify no maximum time");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_MAXIMUMTIME, PROPERTY_MAXIMUMTIME, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetMaxTime),
            dtDAL::FloatActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetMaxTime),
            PROPERTY_MAXIMUMTIME_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_FADEOUTTIME("Fade Out Time");
         static const dtUtil::RefString PROPERTY_FADEOUTTIME_DESC("How long the light should take to fade out (in seconds). This occurs AFTER Max Time. 0 means no fade out.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_FADEOUTTIME, PROPERTY_FADEOUTTIME, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetFadeOutTime),
            dtDAL::FloatActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetFadeOutTime),
            PROPERTY_FADEOUTTIME_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_RADIUS("Radius");
         static const dtUtil::RefString PROPERTY_RADIUS_DESC("The distance the light should affect (currently used only for light priority, not for rendering) ");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_RADIUS, PROPERTY_RADIUS, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetRadius),
            dtDAL::FloatActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::GetRadius),
            PROPERTY_RADIUS_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_DELETEONNULL("Delete When Target Is Null");
         static const dtUtil::RefString PROPERTY_DELETEONNULL_DESC("Indicates to delete the light if the target ever becomes NULL.");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_DELETEONNULL, PROPERTY_DELETEONNULL,
            dtDAL::BooleanActorProperty::SetFuncType(actor, &DynamicLightPrototypeActor::SetDeleteOnTargetIsNull),
            dtDAL::BooleanActorProperty::GetFuncType(actor, &DynamicLightPrototypeActor::IsDeleteOnTargetIsNull),
            PROPERTY_DELETEONNULL_DESC, LIGHT_GROUP));

         static const dtUtil::RefString PROPERTY_LIGHTTYPE("Dynamic Light Type");
         static const dtUtil::RefString PROPERTY_LIGHTTYPE_DESC("Indicates the general type of light. Used for priority and combining lights.");
         AddProperty(new dtDAL::EnumActorProperty<DynamicLightType>(PROPERTY_LIGHTTYPE, PROPERTY_LIGHTTYPE,
            dtDAL::EnumActorProperty<DynamicLightType>::SetFuncType(actor, &DynamicLightPrototypeActor::SetDynamicLightType),
            dtDAL::EnumActorProperty<DynamicLightType>::GetFuncType(actor, &DynamicLightPrototypeActor::GetDynamicLightType),
            PROPERTY_LIGHTTYPE_DESC, LIGHT_GROUP));
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      DynamicLightPrototypeActor::DynamicLightPrototypeActor( dtGame::GameActorProxy &proxy )
         : IGActor(proxy), 
         mIntensity(1.0f),
         mLightColor(osg::Vec3(1.0f, 1.0f, 1.0f)),
         mAttenuation(osg::Vec3(0.1f, 0.005f, 0.00002f)),
         mFlickerScale(0.0f),
         mMaxTime(0.0f),
         mFadeOutTime(1.0f),
         mRadius(100.0f),
         mAutoDeleteLightOnTargetNull(true),
         mDynamicLightType(&DynamicLightType::UNKNOWN)
      {
         SetName("DynamicLightPrototype");
      }

      //////////////////////////////////////////////////////////
      DynamicLightPrototypeActor::~DynamicLightPrototypeActor()
      {

      }




      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      SpotLightPrototypeProxy::SpotLightPrototypeProxy()
      {
         SetClassName("SimCore::Actors::SpotLightPrototypeActor");
         SetInitialOwnership(dtGame::GameActorProxy::Ownership::PROTOTYPE);
      }

      //////////////////////////////////////////////////////////
      SpotLightPrototypeProxy::~SpotLightPrototypeProxy()
      {

      }

      void SpotLightPrototypeProxy::CreateActor()
      {
         SetActor(*new SpotLightPrototypeActor(*this)); 
      }

      //////////////////////////////////////////////////////////
      void SpotLightPrototypeProxy::BuildPropertyMap()
      {
         DynamicLightPrototypeProxy::BuildPropertyMap();

         SpotLightPrototypeActor* actor = dynamic_cast<SpotLightPrototypeActor*>(GetActor());

         static const dtUtil::RefString PROPERTY_USE_ABSOLUTE_DIRECTION("UseAbsoluteDirection");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_USE_ABSOLUTE_DIRECTION, PROPERTY_USE_ABSOLUTE_DIRECTION,
            dtDAL::BooleanActorProperty::SetFuncType(actor, &SpotLightPrototypeActor::SetUseAbsoluteDirection),
            dtDAL::BooleanActorProperty::GetFuncType(actor, &SpotLightPrototypeActor::GetUseAbsoluteDirection),
            "use this flag if this light is attached to a transformable but you do not want to accumulate its parents rotation", "SpotLight"));

         static const dtUtil::RefString PROPERTY_SPOT_EXPONENT("SpotExponent");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_SPOT_EXPONENT, PROPERTY_SPOT_EXPONENT, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &SpotLightPrototypeActor::SetSpotExponent),
            dtDAL::FloatActorProperty::GetFuncType(actor, &SpotLightPrototypeActor::GetSpotExponent),
            "SpotExponent is the spot rate of decay and controls how the lights intensity decays from the center of the cone it its borders. The larger the value the faster de decay, with zero meaning constant light within the light cone.",
            "SpotLight"));

         static const dtUtil::RefString PROPERTY_SPOT_COS_CUTOFF("SpotCosCutoff");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_SPOT_COS_CUTOFF, PROPERTY_SPOT_COS_CUTOFF, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &SpotLightPrototypeActor::SetSpotCosCutoff),
            dtDAL::FloatActorProperty::GetFuncType(actor, &SpotLightPrototypeActor::GetSpotCosCutoff),
            "The cosine of the angle between the light to vertex vector and the spot direction must be larger than spotCosCutoff",
            "SpotLight"));


         static const dtUtil::RefString PROPERTY_SPOTDIRECTION("SpotDirection");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_SPOTDIRECTION, PROPERTY_SPOTDIRECTION,
            dtDAL::Vec3ActorProperty::SetFuncType(actor, &SpotLightPrototypeActor::SetSpotDirection),
            dtDAL::Vec3ActorProperty::GetFuncType(actor, &SpotLightPrototypeActor::GetSpotDirection),
            "The direction of the light", "SpotLight"));

      }



      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      SpotLightPrototypeActor::SpotLightPrototypeActor( dtGame::GameActorProxy &proxy )
         : DynamicLightPrototypeActor(proxy)
         , mUseAbsoluteDirection(false)
         , mSpotExponent(0.5f)
         , mSpotCosCutoff(0.75)
         , mDirection(0.0f, 1.0f, 0.0f)
      {
         SetName("SpotLightPrototype");
      }

      //////////////////////////////////////////////////////////
      SpotLightPrototypeActor::~SpotLightPrototypeActor()
      {

      }

      //////////////////////////////////////////////////////////
      void SpotLightPrototypeActor::SetUseAbsoluteDirection( bool b )
      {
         mUseAbsoluteDirection = b;
      }

      //////////////////////////////////////////////////////////
      bool SpotLightPrototypeActor::GetUseAbsoluteDirection() const
      {
         return mUseAbsoluteDirection;
      }

      //////////////////////////////////////////////////////////
      void SpotLightPrototypeActor::SetSpotExponent( float f )
      {
         mSpotExponent = f;
      }

      //////////////////////////////////////////////////////////
      float SpotLightPrototypeActor::GetSpotExponent() const
      {
         return mSpotExponent;
      }

      //////////////////////////////////////////////////////////
      void SpotLightPrototypeActor::SetSpotDirection( const osg::Vec3& v )
      {
         mDirection = v;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 SpotLightPrototypeActor::GetSpotDirection() const
      {
         return mDirection;
      }

      //////////////////////////////////////////////////////////
      void SpotLightPrototypeActor::SetSpotCosCutoff( float f )
      {
         mSpotCosCutoff = f;
      }

      //////////////////////////////////////////////////////////
      float SpotLightPrototypeActor::GetSpotCosCutoff() const
      {
         return mSpotCosCutoff;
      }
   }

}
