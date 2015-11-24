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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>

#include <SimCore/Actors/MultipleDetonationActor.h>
#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/invokable.h>

#include <dtAudio/audiomanager.h>

#include <dtCore/camera.h>
#include <dtCore/transform.h>
#include <dtCore/batchisector.h>

#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/propertymacros.h>

#include <dtABC/application.h>

#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/TimedDeleterComponent.h>

#include <dtUtil/mathdefines.h>

#include <cmath>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////
      // Actor Proxy code
      //////////////////////////////////////////////////////////////
      const std::string MultipleDetonationActorProxy::CLASS_NAME("SimCore::Actors::MultipleDetonationActor");

      MultipleDetonationActorProxy::MultipleDetonationActorProxy()
      {
         SetClassName(CLASS_NAME);
      }

      ///////////////////////////////////////////////////////////////////////
      MultipleDetonationActorProxy::~MultipleDetonationActorProxy()
      {

      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActorProxy::BuildInvokables()
      {
         dtGame::GameActorProxy::BuildInvokables();
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         MultipleDetonationActor* da = NULL;
         GetDrawable(da);

         static const dtUtil::RefString groupImpactEffects("Multiple Detonation Effects");

         typedef dtCore::PropertyRegHelper<MultipleDetonationActorProxy&, MultipleDetonationActor> RegHelperType;
         RegHelperType propReg(*this, da, groupImpactEffects);

         DT_REGISTER_RESOURCE_PROPERTY(dtCore::DataType::PARTICLE_SYSTEM, MultipleImpactEffect, "Multiple Impact Effect", 
            "The particle system to spawn on impact.", RegHelperType, propReg);

         DT_REGISTER_PROPERTY(NumDetonations, "The number of individual detonations to spawn on impact.", RegHelperType, propReg);

         DT_REGISTER_PROPERTY(DetonationRadius, "The radius to randomly spawn the detonations.", RegHelperType, propReg);

      }

      //////////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////////

      MultipleDetonationActor::MultipleDetonationActor(dtGame::GameActorProxy& owner)
      : DetonationActor(owner)
      , mNumDetonations(24)
      , mDetonationRadius(200)
      , mDetonationOffsets()
      , mExplosionArray()
      , mSmokeArray()
      {
      }

      ///////////////////////////////////////////////////////////////////////
      MultipleDetonationActor::~MultipleDetonationActor()
      {

      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::OnEnteredWorld()
      {
         CreateRandomOffsets();
         CreateDetonationParticles();
         CreateSmokeParticles();
         CreateDynamicLights();
		
         BaseClass::OnEnteredWorld();
      }
      
      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::CreateRandomOffsets()
      {
         dtCore::RefPtr<dtCore::BatchIsector> isector = new dtCore::BatchIsector(&GetGameActorProxy().GetGameManager()->GetScene());

         osg::Vec3 down(0.0f, 0.0f, -1.0f);

	     dtCore::Transform trans;
         GetTransform(trans);
         osg::Vec3 detonationPos;
         trans.GetTranslation(detonationPos);

         for(int i = 0; i < mNumDetonations; ++i)
         {
			osg::Vec2 randVec((0.5f - dtUtil::RandPercent()) * mDetonationRadius, (0.5f - dtUtil::RandPercent()) * mDetonationRadius);
            
            //8848m is considered to be the highest point on earth
            osg::Vec3 pos(detonationPos[0] + randVec[0], detonationPos[1] + randVec[1], 8849.0f);

            dtCore::BatchIsector::SingleISector& singleISector = isector->EnableAndGetISector(i);
            singleISector.SetSectorAsRay(pos, down, 20000.0f);
         }

         isector->Update();

         //now go through and update our positions from the ground clamp
         for(int i = 0; i < mNumDetonations; ++i)
         {
            const dtCore::BatchIsector::SingleISector& singleIsector = isector->GetSingleISector(i);
            osg::Vec3 pos;
            singleIsector.GetHitPoint(pos);

            mDetonationOffsets.push_back(pos);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::PlaySound()
      {
         BaseClass::PlaySound();
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::RenderDetonation()
      {
         BaseClass::RenderDetonation();
         
         ParticleSystemArray::iterator iter = mExplosionArray.begin();
         ParticleSystemArray::iterator iterEnd = mExplosionArray.end();

         for(;iter != iterEnd; ++iter)
         {
            (*iter)->SetEnabled(true);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::RenderSmoke()
      {
         //BaseClass::RenderSmoke();

         ParticleSystemArray::iterator iter = mSmokeArray.begin();
         ParticleSystemArray::iterator iterEnd = mSmokeArray.end();
         
         for(;iter != iterEnd; ++iter)
         {
            StartSmokeEffect(**iter);
         }

      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::StopRenderingSmoke()
      {
         //BaseClass::StopRenderingSmoke();

         ParticleSystemArray::iterator iter = mSmokeArray.begin();
         ParticleSystemArray::iterator iterEnd = mSmokeArray.end();

         for(;iter != iterEnd; ++iter)
         {
            (*iter)->SetEnabled(false);
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::CreateDynamicLights()
      {
         if(!GetGroundImpactLight().empty())
         {
            for(int i = 0; i < mNumDetonations; ++i)
            {
               SimCore::Components::RenderingSupportComponent* renderComp;
               GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, renderComp);

               if(renderComp != NULL)
               {
                  SimCore::Components::RenderingSupportComponent::DynamicLight* dl = renderComp->AddDynamicLightByPrototypeName(GetGroundImpactLight());
                  if(dl != NULL)
                  {
                     dl->mTarget = mExplosionArray[i].get();
                  }
               }
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::CreateDetonationParticles()
      {
         if(!GetGroundImpactEffect().IsEmpty())
         {			 
			dtCore::Transform trans;
          
			for(int i = 0; i < mNumDetonations; ++i)
            {
               dtCore::RefPtr<dtCore::ParticleSystem> particleSys = new dtCore::ParticleSystem();
               AddChild(particleSys.get());

               Components::ParticleInfoAttributeFlags attrs = { true, false };
               RegisterParticleSystem(*particleSys, &attrs);

               trans.SetTranslation(mDetonationOffsets[i]);
               particleSys->SetTransform(trans);

               LoadParticleSystem(GetMultipleImpactEffect(), particleSys);

               mExplosionArray.push_back(particleSys);
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void MultipleDetonationActor::CreateSmokeParticles()
      {
         if(!GetSmokeEffect().IsEmpty())
         {
            dtCore::Transform trans;

            for(int i = 0; i < mNumDetonations; ++i)
            {
               dtCore::RefPtr<dtCore::ParticleSystem> particleSys = new dtCore::ParticleSystem();
               AddChild(particleSys.get());

               Components::ParticleInfoAttributeFlags attrs = { true, false };
               RegisterParticleSystem(*particleSys, &attrs);

               trans.SetTranslation(mDetonationOffsets[i]);
               particleSys->SetTransform(trans);
			   
               LoadParticleSystem(GetSmokeEffect(), particleSys);

               mSmokeArray.push_back(particleSys);
            }
         }
      }


   }
}
