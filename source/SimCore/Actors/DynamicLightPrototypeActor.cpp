/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Curtiss Murphy
 */

#include <prefix/SimCorePrefix-src.h>
#include <dtGame/gameactor.h>
#include <dtDAL/enginepropertytypes.h>
#include <SimCore/Actors/DynamicLightPrototypeActor.h>

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

      //////////////////////////////////////////////////////////
      void DynamicLightPrototypeProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         DynamicLightPrototypeActor& actor = static_cast<DynamicLightPrototypeActor&>(GetGameActor());

         static const std::string LIGHT_GROUP("Dynamic Light");

         static const std::string PROPERTY_INTENSITY("Intensity");
         static const std::string PROPERTY_INTENSITY_DESC("A multiplier for the effect of the light. Can be used to disable or enable a light. Typically 0 or 1 (default). " \
            "Value also used for NVG effects - ex 1000 used for bright flares.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_INTENSITY, PROPERTY_INTENSITY, 
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetIntensity), 
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetIntensity), 
            PROPERTY_INTENSITY_DESC, LIGHT_GROUP));

         static const std::string PROPERTY_ATTENUATION("Attenuation");
         static const std::string PROPERTY_ATTENUATION_DESC("Controls how far the light is visible. The vec3 represents constant(X), linear(Y), quadratic(Z) attenuations");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_ATTENUATION, PROPERTY_ATTENUATION,
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetAttenuation),
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetAttenuation),
            PROPERTY_ATTENUATION_DESC, LIGHT_GROUP));

         //AddProperty(new dtDAL::FloatActorProperty("SaturationIntensity", "Saturation Intensity", 
         //   dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetSaturationIntensity), 
         //   dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetSaturationIntensity), 
         //   "Unused", LIGHT_GROUP));

         static const std::string PROPERTY_LIGHTCOLOR("Light Color");
         static const std::string PROPERTY_LIGHTCOLOR_DESC("The base color of this light");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_LIGHTCOLOR, PROPERTY_LIGHTCOLOR,
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetLightColor),
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetLightColor),
            PROPERTY_LIGHTCOLOR_DESC, LIGHT_GROUP));

         //AddProperty(new dtDAL::BooleanActorProperty("FlickerEnabled", "Flicker Enabled",
         //   dtDAL::MakeFunctor(e, &DynamicLightPrototypeActor::SetFlickerEnabled),
         //   dtDAL::MakeFunctorRet(e, &DynamicLightPrototypeActor::IsFlickerEnabled),
         //   "Indicates if the light should flicker. If enabled, the flicker scale is used to determine how much flicker. ", LIGHT_GROUP));

         static const std::string PROPERTY_FLICKERSCALE("Flicker Scale");
         static const std::string PROPERTY_FLICKERSCALE_DESC("Indicates if the light should flicker and how much. (value is the max range of variation - ex 0.1 to 0.4). 0.0 means no flicker.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_FLICKERSCALE, PROPERTY_FLICKERSCALE, 
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetFlickerScale), 
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetFlickerScale), 
            PROPERTY_FLICKERSCALE_DESC, LIGHT_GROUP));

         static const std::string PROPERTY_MAXIMUMTIME("Maximum Time");
         static const std::string PROPERTY_MAXIMUMTIME_DESC("The maximum time (in seconds) before this light will begin to fade out (see Fade Out Time). Use 0 to specify no maximum time");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_MAXIMUMTIME, PROPERTY_MAXIMUMTIME, 
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetMaxTime), 
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetMaxTime), 
            PROPERTY_MAXIMUMTIME_DESC, LIGHT_GROUP));

         static const std::string PROPERTY_FADEOUTTIME("Fade Out Time");
         static const std::string PROPERTY_FADEOUTTIME_DESC("How long the light should take to fade out (in seconds). This occurs AFTER Max Time. 0 means no fade out.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_FADEOUTTIME, PROPERTY_FADEOUTTIME, 
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetFadeOutTime), 
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetFadeOutTime), 
            PROPERTY_FADEOUTTIME_DESC, LIGHT_GROUP));

         static const std::string PROPERTY_RADIUS("Radius");
         static const std::string PROPERTY_RADIUS_DESC("The distance the light should affect (currently used only for light priority, not for rendering) ");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_RADIUS, PROPERTY_RADIUS, 
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetRadius), 
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetRadius), 
            PROPERTY_RADIUS_DESC, LIGHT_GROUP));

         static const std::string PROPERTY_DELETEONNULL("Delete When Target Is Null");
         static const std::string PROPERTY_DELETEONNULL_DESC("Indicates to delete the light if the target ever becomes NULL.");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_DELETEONNULL, PROPERTY_DELETEONNULL,
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetDeleteOnTargetIsNull),
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::IsDeleteOnTargetIsNull),
            PROPERTY_DELETEONNULL_DESC, LIGHT_GROUP));

         static const std::string PROPERTY_LIGHTTYPE("Dynamic Light Type");
         static const std::string PROPERTY_LIGHTTYPE_DESC("Indicates the general type of light. Used for priority and combining lights.");
         AddProperty(new dtDAL::EnumActorProperty<DynamicLightType>(PROPERTY_LIGHTTYPE, PROPERTY_LIGHTTYPE,
            dtDAL::MakeFunctor(actor, &DynamicLightPrototypeActor::SetDynamicLightType),
            dtDAL::MakeFunctorRet(actor, &DynamicLightPrototypeActor::GetDynamicLightType),
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
      }

      //////////////////////////////////////////////////////////
      DynamicLightPrototypeActor::~DynamicLightPrototypeActor()
      {

      }

   }

}
