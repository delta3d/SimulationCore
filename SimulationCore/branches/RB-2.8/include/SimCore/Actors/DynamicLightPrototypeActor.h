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
* @author Bradley Anderegg
*/


#ifndef _DYNAMICLIGHTPROTOTYPE_ACTOR_H_
#define _DYNAMICLIGHTPROTOTYPE_ACTOR_H_

#include <SimCore/Actors/IGActor.h>
#include <dtUtil/enumeration.h>

namespace SimCore
{
   namespace Actors
   {
      /**
       * Type of light - identifies the general class of light. Useful for combining lights when we 
       * have too many dynamic lights. Although each instance of a type of light can be unique, they 
       * will generally have similar properties. For instance, all flare munition are bright and all tracer
       * rounds move fast and have a short life and all fires have a relatively short radius.
       */
      class SIMCORE_EXPORT DynamicLightType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(DynamicLightType);
      public:
         static DynamicLightType UNKNOWN; // Default. We don't know what it is.
         static DynamicLightType FIRE; // Most fires. Typically flicker and have relatively short range. May or may not move.
         static DynamicLightType TRACER; // Typically VERY fast, short range light. 
         static DynamicLightType HEADLIGHT; // Fairly bright but relatively short range.
         static DynamicLightType FLARE; // extremely bright, high intensity, huge range lights.
         static DynamicLightType WEAPONS_FLASH; // mildly bright, short radius, short lived lights.
         static DynamicLightType GENERIC_SMALL; // General case for low range/intensity lights
         static DynamicLightType GENERIC_MEDIUM; // General case for medium range/intensity lights
         static DynamicLightType GENERIC_LARGE; // General case for large range/intensity lights
         static DynamicLightType OTHER; // It's something else not yet defined, but not actually unknown

      private:
         DynamicLightType(const std::string &name) : dtUtil::Enumeration(name)
         {
            AddInstance(this);
         }
      };


      /* 
       * Actor - The DynamicLightPrototype actor is used to set properties for lights in the map. This base class
       * is not intended to become a real actor. Instead, when you create a dynamic light in the world (such as a flare, 
       * a fire, or flash effect, you look up one of these prototypes from the map. This allows the lights to be configured 
       * in the map instead of hard coded.
       */
      class SIMCORE_EXPORT DynamicLightPrototypeActor : public IGActor
      {
      public:

         /// Constructor
         DynamicLightPrototypeActor(dtGame::GameActorProxy &proxy);

         // Intensity - the intensity is a multiplier of the effect of the light, can be used to disable or enable a light, typically 1 or 0
         void SetIntensity( float intensity ) { mIntensity = intensity; }
         float GetIntensity() const { return mIntensity; }

         // Saturation Intensity - this is currently not used for performance
         //void SetSaturationIntensity( float saturationIntensity ) { mSaturationIntensity = saturationIntensity; }
         //float GetSaturationIntensity() const { return mSaturationIntensity; }

         // Attenuation - this controls how far the light is visible from the vec3 represents (constant, linear, quadratic) attentions
         void SetAttenuation(const osg::Vec3 &attenuation) { mAttenuation = attenuation; }
         osg::Vec3 GetAttenuation() const { return mAttenuation; }

         // Light Color - The base color of the light.
         void SetLightColor(const osg::Vec3 &lightColor) { mLightColor = lightColor; }
         osg::Vec3 GetLightColor() const { return mLightColor; }

         // Flicker - indicates if the light should flicker or not. Flicker uses the flicker scale to create a fantastic visual effect 
         //void SetFlickerEnabled(bool flickerEnabled) { mFlickerEnabled = flickerEnabled; }
         //bool IsFlicerEnabled() const { return mFlickerEnabled; }

         // Flicker Scale - the maximum increase or decrease in the light intensity (something like 0.1-0.4)
         void SetFlickerScale( float flickerScale ) { mFlickerScale = flickerScale; }
         float GetFlickerScale() const { return mFlickerScale; }

         // Max Time - the maximum time (in seconds) that the light should exist. Set to 0 to have NO maximum time.
         void SetMaxTime( float maxTime ) { mMaxTime = maxTime; }
         float GetMaxTime() const { return mMaxTime; }

         // Fade Out Time - how long the light should take to fade out (in seconds). This occurs AFTER Max Time. 0 means no fade out. 
         void SetFadeOutTime( float fadeOutTime ) { mFadeOutTime = fadeOutTime; }
         float GetFadeOutTime() const { return mFadeOutTime; }

         // Radius - The distance of the bounding sphere of the light. ie, in general how big is the light estimated to be. Used for light priority, not for rendering 
         void SetRadius( float radius ) { mRadius = radius; }
         float GetRadius() const { return mRadius; }

         
         // Auto Delete If Target Is Null - indicates to delete this light if the target ever becomes NULL.  
         void SetDeleteOnTargetIsNull(bool autoDeleteLightOnTargetNull) { mAutoDeleteLightOnTargetNull = autoDeleteLightOnTargetNull; }
         bool IsDeleteOnTargetIsNull() const { return mAutoDeleteLightOnTargetNull; }

         // Dynamic Light Type - Indicates the general type of this light. Used for combining lights and priority.
         virtual void SetDynamicLightType(DynamicLightType &lightType) { mDynamicLightType = &lightType; }
         DynamicLightType& GetDynamicLightType() const { return *mDynamicLightType; }

      protected:

         /// Destructor
         virtual ~DynamicLightPrototypeActor();

      private:
         //these variables effect the way the light appears..
         float mIntensity; //the intensity is a multiplier of the effect of the light, can be used to disable or enable a light, typically 1 or 0
         //float mSaturationIntensity; //this is currently not used for performance
         osg::Vec3 mLightColor;
         osg::Vec3 mAttenuation; //this controls how far the light is visible from the vec3 represents (constant, linear, quadratic) attentions

         //these variables effect the simulation of the light
         //]bool mFlickerEnabled; //a flickering light will automatically increase and decrease its intensity relative to the flicker scale
         float mFlickerScale;  //the flicker scale should be the maximum increase or decrease in the light intensity in a single frame
                              //and also the maximum it will vary from its original intensity

         //bool mAutoDeleteAfterMaxTime; //using this flag will set the light to be automatically removed after the number of seconds
         float mMaxTime;            //specified by mMaxTime, this can be used in conjunction with Fade Out
         //bool mFadeOut;      //if this is set to true we will gradually decrease our intensity over the time specified
         float mFadeOutTime; //NOTE: if used in accordance with mMaxTime OR mAutoDeleteLightOnTargetNull then the fade out will
         //  occur after the object is marked for deletion.  So if MaxTime = 1.0 and FadeOutTime = 0.5
         //  the light will be at 100% intensity for 1.0 seconds and then fade from 100% to 0% over 0.5 seconds

         float mRadius;      //this is used to determine how far away we are from the light, it pretty much makes the light into a bounding sphere

         bool mAutoDeleteLightOnTargetNull; //setting this flag will auto delete the light when the target becomes NULL, this
                                            //can be used in conjunction with Fade Out

         DynamicLightType* mDynamicLightType;
      };

      /* 
       * Proxy - The DynamicLightPrototype actor is used to set properties for lights in the map. This base class
       * is not intended to become a real actor. Instead, when you create a dynamic light in the world (such as a flare, 
       * a fire, or flash effect, you look up one of these prototypes from the map. This allows the lights to be configured 
       * in the map instead of hard coded.
       */
      class SIMCORE_EXPORT DynamicLightPrototypeProxy : public dtGame::GameActorProxy
      {
      public:

         /// Constructor
         DynamicLightPrototypeProxy();

         void CreateDrawable();

         /// Adds the properties associated with this actor
         virtual void BuildPropertyMap();

      protected:

         /// Destructor
         virtual ~DynamicLightPrototypeProxy();

      private:
      };

      class SIMCORE_EXPORT SpotLightPrototypeActor : public DynamicLightPrototypeActor
      {
      public:

         /// Constructor
         SpotLightPrototypeActor(dtGame::GameActorProxy &proxy);

      public:

         /// Destructor
         virtual ~SpotLightPrototypeActor();

         void SetUseAbsoluteDirection(bool b);
         bool GetUseAbsoluteDirection() const;

         void SetSpotExponent(float f);
         float GetSpotExponent() const;

         void SetSpotCosCutoff(float f); 
         float GetSpotCosCutoff() const;

         void SetSpotDirection(const osg::Vec3& v);
         osg::Vec3 GetSpotDirection() const;

      private:

         bool mUseAbsoluteDirection;   //use this flag if this light is attached to a transformable but you
         //do not want to accumulate its parents rotation

         //mSpotExponent is the spot rate of decay and controls how 
         //the lights intensity decays from the center of the cone it its borders. The larger the value the faster de decay, with zero meaning constant light within the light cone.
         float mSpotExponent;  

         //The cosine of the angle between the light to vertex vector and the spot direction must be larger than spotCosCutoff
         float mSpotCosCutoff;

         osg::Vec3 mDirection;         //The direction of the light              

      };

      class SIMCORE_EXPORT SpotLightPrototypeProxy : public DynamicLightPrototypeProxy
      {
      public:

         /// Constructor
         SpotLightPrototypeProxy();

         /// Creates the drawable
         void CreateDrawable();

         /// Adds the properties associated with this actor
         virtual void BuildPropertyMap();

      protected:

         /// Destructor
         virtual ~SpotLightPrototypeProxy();

      private:
      };
   }
}

#endif
