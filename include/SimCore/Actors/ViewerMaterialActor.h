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
* @author Allen Danklefsen && The Delta3D Team.
*/
#ifndef _VIEWER_MATERIAL_ACTOR_H_
#define _VIEWER_MATERIAL_ACTOR_H_

#include <dtGame/gameactor.h>

#include <SimCore/Export.h>
#include <osg/Vec4>

namespace dtGame
{
   class Message;
}

/*      TYPES     */
/* ============== */
/* PHYS = Physics */
/* SHAD = Shader  */
/* PART = Particle*/
/* SFX  = Sound Effect*/
/* MUSC = Music Effect*/
/* NET  = Network Effect*/
/* VISL = Other Visual Effect*/
/* OTHR = Other or Multiple Effect*/

namespace SimCore
{
   namespace Actors
   {

      /**
       * Proxy for the ViewerMaterialActor.
       */
      class SIMCORE_EXPORT ViewerMaterialActor : public dtGame::GameActorProxy
      {
         public:
            ViewerMaterialActor();

            // Adds the properties associated with this actor
            virtual void BuildPropertyMap();

            /*override*/ void OnEnteredWorld();

            /**
            * The Material actor is global.
            */
            virtual bool IsPlaceable() const { return false;}

            // Creates the actor
            void CreateDrawable();
         public:

            ///@return The value of m_SFX_SmallHit
            std::string GetSmallHitSoundEffect()           {return m_SFX_SmallHit;}
            ///@return The value of m_SFX_MediumHit
            std::string GetMediumHitSoundEffect()          {return m_SFX_MediumHit;}
            ///@return The value of m_SFX_LargeHit
            std::string GetLargeHitSoundEffect()           {return m_SFX_LargeHit;}
            ///@return The value of m_SFX_AmbientRussel
            std::string GetAmbientSoundEffect()            {return m_SFX_AmbientRussel;}
            ///@return The value of m_MUSC_AmbientTrack
            std::string GetAmbientMusicEffect()            {return m_MUSC_AmbientTrack;}
            ///@return The value of m_VISL_DecalSmallHit
            std::string GetDecalSmallHit()                 {return m_VISL_DecalSmallHit;}
            ///@return The value of m_VISL_DecalMediumHit
            std::string GetDecalMediumHit()                {return m_VISL_DecalMediumHit;}
            ///@return The value of m_VISL_DecalLargeHit
            std::string GetDecalLargeHit()                 {return m_VISL_DecalLargeHit;}
            ///@return The value of m_VISL_BaseColor
            osg::Vec4 GetBaseColorvalue()                  {return m_VISL_BaseColor;}
            ///@return The value of m_VISL_HighlightColor
            osg::Vec4 GetHighlighteColorvalue()            {return m_VISL_HighlightColor;}
            ///@return The value of m_PART_DustTrail
            std::string GetDustTrailEffect()               {return m_PART_DustTrail;}
            ///@return The value of m_PART_SmallHit
            std::string GetSmallHitEffect()                {return m_PART_SmallHit;}
            ///@return The value of m_PART_MediumHit
            std::string GetMediumHitEffect()               {return m_PART_MediumHit;}
            ///@return The value of m_PART_LargeHit
            std::string GetLargeHitEffect()                {return m_PART_LargeHit;}
            ///@return The value of restituion [0, 1) (0.2 default)
            float    GetRestitution()                      {return m_PHYS_Restitution;}
            ///@return The value of static friction (0.5 default)
            float    GetStaticFriction()                    {return m_PHYS_StaticFriction;}
            ///@return The value of dynamic friction (0.5 default)
            float    GetDynamicFriction()                   {return m_PHYS_DynamicFriction;}
            /// @return The physics particle system to use
            std::string GetPhysicsParticleLookupStringOne() {return m_VISL_PhysicsParticleOne;}
            /// @return The physics particle system to use
            std::string GetPhysicsParticleLookupStringTwo() {return m_VISL_PhysicsParticleTwo;}
            /// @return The physics particle system to use
            std::string GetPhysicsParticleLookupStringThree() {return m_VISL_PhysicsParticleThree;}
            /// @return The physics particle system to use
            std::string GetPhysicsParticleLookupStringFour(){return m_VISL_PhysicsParticleFour;}
            /// @return The physics particle system to use
            std::string GetPhysicsParticleLookupStringFive(){return m_VISL_PhysicsParticleFive;}

            ///@param value The value for shinyness - shader / visual for an object
            ///@param value The value string the file is loaded from.
            void SetSmallHitSoundEffect(const std::string& value) {m_SFX_SmallHit = value;}
            ///@param value The value string the file is loaded from.
            void SetMediumHitSoundEffect(const std::string& value){m_SFX_MediumHit = value;}
            ///@param value The value string the file is loaded from.
            void SetLargeHitSoundEffect(const std::string& value) {m_SFX_LargeHit = value;}
            ///@param value The value string the file is loaded from.
            void SetAmbientSoundEffect(const std::string& value)  {m_SFX_AmbientRussel = value;}
            ///@param value The value string the file is loaded from.
            void SetAmbientMusicEffect(const std::string& value)  {m_MUSC_AmbientTrack = value;}
            ///@param value The value string the file is loaded from.
            void SetDecalSmallHit(const std::string& value)       {m_VISL_DecalSmallHit = value;}
            ///@param value The value string the file is loaded from.
            void SetDecalMediumHit(const std::string& value)      {m_VISL_DecalMediumHit = value;}
            ///@param value The value string the file is loaded from.
            void SetDecalLargeHit(const std::string& value)       {m_VISL_DecalLargeHit = value;}
            ///@param value The value for use with decals / particles
            void SetBaseColorvalue(const osg::Vec4& value)        {m_VISL_BaseColor = value;}
            ///@param value The value for use with decals / particles
            void SetHighlighteColorvalue(const osg::Vec4& value)  {m_VISL_HighlightColor = value;}
            ///@param value The value of the particle sys to load in for dust trails
            void SetDustTrailEffect(const std::string& value)     {m_PART_DustTrail = value;}
            ///@param value The value of the small hit particle effect to load in.
            void SetSmallHitEffect(const std::string& value)      {m_PART_SmallHit = value;}
            ///@param value The value of the medium hit particle effect to load in.
            void SetMediumHitEffect(const std::string& value)     {m_PART_MediumHit = value;}
            ///@param value The value of the large hit particle effect to load in.
            void SetLargeHitEffect(const std::string& value)      {m_PART_LargeHit = value;}
            ///@param value The value of restituion [0, 1) (0.2 default)
            void SetRestitution(float value)                      {m_PHYS_Restitution = value;}
            ///@param value The value of static friction (0.5 default)
            void SetStaticFriction(float value)                   {m_PHYS_StaticFriction = value;}
            ///@param value The value of dynamic friction (0.5 default)
            void SetDynamicFriction(float value)                  {m_PHYS_DynamicFriction = value;}
            /// @param value The name of the physics particle system prototype to look up in the map
            void SetPhysicsParticleLookupStringOne(const std::string& name) {m_VISL_PhysicsParticleOne = name;}
            /// @param value The name of the physics particle system prototype to look up in the map
            void SetPhysicsParticleLookupStringTwo(const std::string& name) {m_VISL_PhysicsParticleTwo = name;}
            /// @param value The name of the physics particle system prototype to look up in the map
            void SetPhysicsParticleLookupStringThree(const std::string& name) {m_VISL_PhysicsParticleThree = name;}
            /// @param value The name of the physics particle system prototype to look up in the map
            void SetPhysicsParticleLookupStringFour(const std::string& name) {m_VISL_PhysicsParticleFour = name;}
            /// @param value The name of the physics particle system prototype to look up in the map
            void SetPhysicsParticleLookupStringFive(const std::string& name) {m_VISL_PhysicsParticleFive = name;}

         protected:
            virtual ~ViewerMaterialActor();
         private:
            //////////////////////////////////////////////////////////////////////////////////////////////
            //                                  Physics Values                                          //
            //////////////////////////////////////////////////////////////////////////////////////////////

            /**
            * Dynamic Friction is a Physics material property for objects.  It defines
            * how easy or hard it is to keep something sliding. 0 means a frictionless
            * environment (space).
            */
            float m_PHYS_DynamicFriction;

            /**
            * Static Friction is a Physics material property for objects.  It defines
            * how easy or hard it is to get moving when it is stopped. 0 means
            * a frictionless environment (space).
            */
            float m_PHYS_StaticFriction;

            /**
            * Restitution is a Physics material property for objects. It defines the 'bouncy-ness' of
            * objects. It should be [0,1).  Values close to 0 mean almost NO bounce. Values close to 1
            * will retain all energy (possibly increase).
            */
            float m_PHYS_Restitution;

            /// The scene name which the physics material is in.
            std::string m_PHYS_SceneName;

            //////////////////////////////////////////////////////////////////////////////////////////////
            //                                  SND/MUSC Values                                         //
            //////////////////////////////////////////////////////////////////////////////////////////////
            std::string m_SFX_SmallHit;      /// SFX to be used for a hit with the object.
            std::string m_SFX_MediumHit;     /// SFX to be used for a hit with the object.
            std::string m_SFX_LargeHit;      /// SFX to be used for a hit with the object.
            std::string m_SFX_AmbientRussel; /// Does this have ambient russel noise (the dust storms in the desert)
            std::string m_MUSC_AmbientTrack; /// Does this have ambient long music streaming from it? Example : Radio
            float       m_SFX_DensityForSound; /// Density value for sound.

            //////////////////////////////////////////////////////////////////////////////////////////////
            //                                  Visual Values                                           //
            //////////////////////////////////////////////////////////////////////////////////////////////
            std::string m_VISL_DecalSmallHit;                     /// Decal Texture for use for hit with obj.
            std::string m_VISL_DecalMediumHit;                    /// Decal Texture for use for hit with obj.
            std::string m_VISL_DecalLargeHit;                     /// Decal Texture for use for hit with obj.
            osg::Vec4   m_VISL_BaseColor;                         /// Base color for decals / particles.
            osg::Vec4   m_VISL_HighlightColor;                    /// Hlight color for decals/particles.
            std::string m_VISL_PhysicsParticleOne;                /* name of the physics model that could be
                                                                     loaded from a template in the map    */
            std::string m_VISL_PhysicsParticleTwo;                /* name of the physics model that could be
                                                                     loaded from a template in the map    */
            std::string m_VISL_PhysicsParticleThree;                /* name of the physics model that could be
                                                                     loaded from a template in the map    */
            std::string m_VISL_PhysicsParticleFour;               /* name of the physics model that could be
                                                                     loaded from a template in the map    */
            std::string m_VISL_PhysicsParticleFive;               /* name of the physics model that could be
                                                                     loaded from a template in the map    */

            //////////////////////////////////////////////////////////////////////////////////////////////
            //                                  PARTICLE Values                                         //
            //////////////////////////////////////////////////////////////////////////////////////////////
            std::string m_PART_DustTrail;    /// DustTrail like particle system
            std::string m_PART_SmallHit;     /// Particle System for small hit.
            std::string m_PART_MediumHit;    /// Particle System for medium hit.
            std::string m_PART_LargeHit;     /// Particle System for large hit.

      };
   }
}

#endif
