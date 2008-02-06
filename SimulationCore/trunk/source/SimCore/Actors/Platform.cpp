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
 * @author David Guthrie
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <string>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Components/DefaultArticulationHelper.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/invokable.h>
#include <dtGame/gameactor.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messageparameter.h>

#include <dtCore/tangentspacevisitor.h>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderparamfloat.h>
#include <dtCore/shadermanager.h>
#include <dtCore/particlesystem.h>
#include <dtCore/nodecollector.h>
#include <dtCore/camera.h>
#include <dtCore/boundingboxvisitor.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/groupactorproperty.h>
#include <dtDAL/actorproxyicon.h>
#include <dtDAL/actortype.h>
#include <dtDAL/namedparameter.h>

#include <dtUtil/log.h>
#include <dtUtil/mathdefines.h>

#include <dtAudio/sound.h>
#include <dtAudio/audiomanager.h>

#include <dtABC/application.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Switch>
#include <osg/MatrixTransform>
#include <osg/Point>
#include <osg/NodeVisitor>
#include <osg/Program>
#include <osgSim/DOFTransform>

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////////
      const std::string Platform::INVOKABLE_TICK_CONTROL_STATE("TickControlState");

      ////////////////////////////////////////////////////////////////////////////////////
      // Actor Proxy code
      ////////////////////////////////////////////////////////////////////////////////////
      const std::string PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED("Head Lights Enabled");

      ////////////////////////////////////////////////////////////////////////////////////
      PlatformActorProxy::PlatformActorProxy()
      {
         SetClassName("SimCore::Actors::Platform");
      }

      ////////////////////////////////////////////////////////////////////////////////////
      PlatformActorProxy::~PlatformActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::BuildPropertyMap()
      {
         Platform &e = static_cast<Platform&>(GetGameActor());

         BaseClass::BuildPropertyMap();

         static const std::string PROPERTY_MESH_NON_DAMAGED_ACTOR("Non-damaged actor");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            PROPERTY_MESH_NON_DAMAGED_ACTOR,
            PROPERTY_MESH_NON_DAMAGED_ACTOR,
            dtDAL::MakeFunctor(*this, &PlatformActorProxy::LoadNonDamagedFile),
            "This is the model for a non damaged actor"));

         static const std::string PROPERTY_MESH_DAMAGED_ACTOR("Damaged actor");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            PROPERTY_MESH_DAMAGED_ACTOR,
            PROPERTY_MESH_DAMAGED_ACTOR,
            dtDAL::MakeFunctor(*this, &PlatformActorProxy::LoadDamagedFile),
            "This is the model for a damaged actor"));

         static const std::string PROPERTY_MESH_DESTROYED_ACTOR("Destroyed actor");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            PROPERTY_MESH_DESTROYED_ACTOR,
            PROPERTY_MESH_DESTROYED_ACTOR,
            dtDAL::MakeFunctor(*this, &PlatformActorProxy::LoadDestroyedFile),
            "This is the model for a destroyed actor"));

         static const std::string PROPERTY_MUZZLE_FLASH_POSITION("Muzzle Flash Position");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_MUZZLE_FLASH_POSITION,
            PROPERTY_MUZZLE_FLASH_POSITION,
            dtDAL::MakeFunctor(e, &Platform::SetMuzzleFlashPosition),
            dtDAL::MakeFunctorRet(e, &Platform::GetMuzzleFlashPosition),
            "Sets the muzzle flash position on an entity"));
         
         static const std::string PROPERTY_ARTICULATION_PARAM_ARRAY("Articulated Parameters Array");
         AddProperty(new dtDAL::GroupActorProperty(PROPERTY_ARTICULATION_PARAM_ARRAY,
            PROPERTY_ARTICULATION_PARAM_ARRAY, 
            dtDAL::MakeFunctor(e, &Platform::SetArticulatedParametersArray), 
            dtDAL::MakeFunctorRet(e, &Platform::GetArticulatedParametersArray), 
            "The list of articulated parameters for modifying DOF's", ""));

         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_HEAD_LIGHTS_ENABLED,
            PROPERTY_HEAD_LIGHTS_ENABLED,
            dtDAL::MakeFunctor(e, &Platform::SetHeadLightsEnabled),
            dtDAL::MakeFunctorRet(e, &Platform::IsHeadLightsEnabled),
            "Determines if the entity has it head lights on or not."));

         static const std::string PROPERTY_SEAT_CONFIG_TABLE_NAME("VehiclesSeatConfigActorNameTable");
         AddProperty(new dtDAL::StringActorProperty(PROPERTY_SEAT_CONFIG_TABLE_NAME,
            PROPERTY_SEAT_CONFIG_TABLE_NAME,
            dtDAL::MakeFunctor(e, &Platform::SetVehiclesSeatConfigActorName),
            dtDAL::MakeFunctorRet(e, &Platform::GetVehiclesSeatConfigActorName),
            "The Vehicle seat config option to coincide with the use of portals.",""));

         static const std::string PROPERTY_ENTITY_TYPE("EntityType");
         AddProperty(new dtDAL::StringActorProperty(PROPERTY_ENTITY_TYPE,
            PROPERTY_ENTITY_TYPE,
            dtDAL::MakeFunctor(e, &Platform::SetEntityType),
            dtDAL::MakeFunctorRet(e, &Platform::GetEntityType),
            "The type of the entity, such as HMMWVDrivingSim. Used to determine what behaviors this entity can have at runtime, such as embark, gunner, commander, ...", ""));

         static const std::string SOUND_PROPERTY_TYPE("Sounds");

         AddProperty(new dtDAL::FloatActorProperty("MinDistanceIdleSound", "MinDistanceIdleSound",
            dtDAL::MakeFunctor(e, &Platform::SetMinDistanceIdleSound),
            dtDAL::MakeFunctorRet(e, &Platform::GetMinDistanceIdleSound),
            "Distance for the sound", SOUND_PROPERTY_TYPE));

         AddProperty(new dtDAL::FloatActorProperty("MaxDistanceIdleSound", "MaxDistanceIdleSound",
            dtDAL::MakeFunctor(e, &Platform::SetMaxDistanceIdleSound),
            dtDAL::MakeFunctorRet(e, &Platform::GetMaxDistanceIdleSound),
            "Distance for the sound", SOUND_PROPERTY_TYPE));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "mSFXSoundIdleEffect", "mSFXSoundIdleEffect", dtDAL::MakeFunctor(e, 
            &Platform::SetSFXEngineIdleLoop),
            "What is the filepath / string of the sound effect", SOUND_PROPERTY_TYPE));
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();
         
         Platform* actor = static_cast<Platform*>(GetActor());

         AddInvokable(*new dtGame::Invokable(Platform::INVOKABLE_TICK_CONTROL_STATE, 
            dtDAL::MakeFunctor(*actor, &Platform::TickControlState)));

         AddInvokable(*new dtGame::Invokable("TimeElapsedForSoundIdle",
            dtDAL::MakeFunctor(*actor, &Platform::TickTimerMessage)));
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::CreateActor()
      {
         Platform* pEntity = new Platform(*this);
         SetActor(*pEntity);

         //we made a virtual function to create our dead reckoning helper so the helper
         //could be changed by a subclass of entity.... bga
         pEntity->InitDeadReckoningHelper();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtDAL::ActorProxyIcon* PlatformActorProxy::GetBillboardIcon()
      {
         if(!mBillboardIcon.valid())
            mBillboardIcon = new dtDAL::ActorProxyIcon(dtDAL::ActorProxyIcon::IconType::STATICMESH);

         return mBillboardIcon.get();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::LoadNonDamagedFile(const std::string &fileName)
      {
         static_cast<Platform&>(GetGameActor()).LoadDamageableFile(fileName, BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::LoadDamagedFile(const std::string &fileName)
      {
         static_cast<Platform&>(GetGameActor()).LoadDamageableFile(fileName, BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::LoadDestroyedFile(const std::string &fileName)
      {
         static_cast<Platform&>(GetGameActor()).LoadDamageableFile(fileName, BaseEntityActorProxy::DamageStateEnum::DESTROYED);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      // Actor code
      ////////////////////////////////////////////////////////////////////////////////////
      const std::string Platform::DOF_NAME_HEAD_LIGHTS("headlight_01");

      ////////////////////////////////////////////////////////////////////////////////////
      Platform::Platform(dtGame::GameActorProxy &proxy) : BaseEntity(proxy),
         mTimeBetweenControlStateUpdates(0.3333f),
         mTimeUntilControlStateUpdate(0.0f),
         mNonDamagedFileNode(new osg::Group()),
         mDamagedFileNode(new osg::Group()),
         mDestroyedFileNode(new osg::Group()),
         mSwitchNode(new osg::Switch),
         mHeadLightsEnabled(false),
         mHeadLightID(0),
         mMinIdleSoundDistance(5.0f),
         mMaxIdleSoundDistance(30.0f)
      {
         mSwitchNode->insertChild(0, mNonDamagedFileNode.get());
         mSwitchNode->insertChild(1, mDamagedFileNode.get());
         mSwitchNode->insertChild(2, mDestroyedFileNode.get());

         GetScaleMatrixTransform().addChild(mSwitchNode.get());

         SetDrawingModel(true);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      Platform::~Platform()
      {
         if(mSndIdleLoop.valid())
         {
            mSndIdleLoop->Stop();
            RemoveChild(mSndIdleLoop.get());
            mSndIdleLoop.release();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetHeadLightsEnabled( bool enabled )
      {
         mHeadLightsEnabled = enabled;

         // Attempt to capture the headlight transformable that is tracked by the
         // light effect for world position information.
         dtCore::RefPtr<dtCore::Transformable> headLightPoint;
         dtCore::DeltaDrawable* curChild = NULL;
         unsigned limit = GetNumChildren();
         for( unsigned i = 0; i < limit; ++i )
         {
            curChild = GetChild( i );
            if( curChild != NULL && curChild->GetName() == DOF_NAME_HEAD_LIGHTS )
            {
               headLightPoint = dynamic_cast<dtCore::Transformable*>(curChild);
               break;
            }
         }

         // If the head light point was not found then try to create it.
         if( ! headLightPoint.valid() )
         {
            // If there is a node collector...
            dtCore::NodeCollector* nodeCollector = GetNodeCollector();

            // ...and there is a head light DOF...
            osgSim::DOFTransform* lightTrans = nodeCollector != NULL 
               ? nodeCollector->GetDOFTransform( DOF_NAME_HEAD_LIGHTS )
               : NULL;
            if( lightTrans == NULL )
            {
               return;
            }

            // ...add a Delta transformable at the same point as the DOF so that the
            // dynamic light effect can track the position; it does not track OSG DOF transforms.
            headLightPoint = new dtCore::Transformable( DOF_NAME_HEAD_LIGHTS );
            AddChild( headLightPoint.get() );
            dtCore::Transform xform;
            // NOTE: currently the DOF does not return a position for watever reason, so
            // the position is offset by a hard-coded offset temporarily.
            xform.SetTranslation( lightTrans->getCurrentTranslate()+osg::Vec3(0,20.0,0) );
            headLightPoint->SetTransform( xform, dtCore::Transformable::REL_CS );
         }

         // If the light point has been created or accessed...
         if( headLightPoint.valid() )
         {
            // ...get the game manager...
            dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
            if( gm == NULL )
            {
               return;
            }

            // ...so that the render support component can be accessed...
            SimCore::Components::RenderingSupportComponent* rsComp
               = dynamic_cast<SimCore::Components::RenderingSupportComponent*>
               (gm->GetComponentByName( SimCore::Components::RenderingSupportComponent::DEFAULT_NAME ));

            // ...so that the head light effect can be accessed...
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl = rsComp->GetDynamicLight( mHeadLightID );

            if( enabled )
            {
               // ...and if the light effect does not exist...
               if( dl == NULL )
               {
                  // ...create it and get its ID to be tracked.
                  //
                  // NOTE: The following string value for the light name should eventually be
                  //       property, but is not currently for the sake of time.
                  dl = rsComp->AddDynamicLightByPrototypeName( "Light-HMMWV-Headlight" );
                  dl->mTarget = headLightPoint.get();
                  mHeadLightID = dl->mID;
               }
            }
            else if( dl != NULL )
            {
               // ...turn it off by removing it.
               rsComp->RemoveDynamicLight( mHeadLightID );
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool Platform::IsHeadLightsEnabled() const
      {
         return mHeadLightsEnabled;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 ComputeDimensions( osg::Node& node )
      {
         dtCore::BoundingBoxVisitor bbv;
         node.accept(bbv);

         osg::Vec3 modelDimensions;
         modelDimensions.x() = bbv.mBoundingBox.xMax() - bbv.mBoundingBox.xMin();
         modelDimensions.y() = bbv.mBoundingBox.yMax() - bbv.mBoundingBox.yMin();
         modelDimensions.z() = bbv.mBoundingBox.zMax() - bbv.mBoundingBox.zMin();
         return modelDimensions;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::InternalSetDamageState(PlatformActorProxy::DamageStateEnum &damageState)
      {
         if(IsDrawingModel())
         {

            bool setUseDimensions = true;
            osg::Node* modelToCalcDims = NULL;

            if(damageState == PlatformActorProxy::DamageStateEnum::NO_DAMAGE)
            {
               mSwitchNode->setSingleChildOn(0);
               modelToCalcDims = mNonDamagedFileNode.get();
            }
            else if(damageState == PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE || damageState == PlatformActorProxy::DamageStateEnum::MODERATE_DAMAGE)
            {
               mSwitchNode->setSingleChildOn(1);
               modelToCalcDims = mDamagedFileNode.get();
            }
            else if(damageState == PlatformActorProxy::DamageStateEnum::DESTROYED)
            {
               mSwitchNode->setSingleChildOn(2);
               modelToCalcDims = mDestroyedFileNode.get();
            }
            else
            {
               mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,  
               "Damage state is not a valid state");

               setUseDimensions = false;
            }


            //compute the model dimensions for this damage state model, since the dimensions will differ from
            //the other damage states
            //NOTE: this fixes the flaming entity problem because the DeadReckoningComponent will
            //factor in any particle system attached to us with our bounding volume
            if(modelToCalcDims != NULL)
            {
               GetDeadReckoningHelper().SetModelDimensions(ComputeDimensions(*modelToCalcDims));               
            }

            GetDeadReckoningHelper().SetUseModelDimensions(setUseDimensions);

         }

         BaseClass::SetDamageState(damageState);

         // Update the articulation helper with DOFs of the current model.
         if( ! mArticHelper.valid() )
         {
            mArticHelper = new SimCore::Components::DefaultArticulationHelper;
         }

         mArticHelper->UpdateDOFReferences( mNodeCollector.get() );

         //The damage state can effect the dr algorithm.
         SetDeadReckoningAlgorithm(GetDeadReckoningAlgorithm());
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetDamageState(PlatformActorProxy::DamageStateEnum &damageState)
      {
         if (damageState != GetDamageState())
         {
            InternalSetDamageState(damageState);
         }
      }

      /// For the different physics models
      osg::Node* Platform::GetNonDamagedFileNode() 
      {
         return mNonDamagedFileNode.get()->getChild(0);
      }

      /// For the different physics models
      osg::Node* Platform::GetDamagedFileNode()
      {
         return mDamagedFileNode.get()->getChild(0);
      }

      /// For the different physics models
      osg::Node* Platform::GetDestroyedFileNode()
      {
         return mDestroyedFileNode.get()->getChild(0);
      }  

      ////////////////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<osg::Switch> Platform::GetSwitchNode() 
      { 
         return mSwitchNode; 
      }

      void Platform::HandleModelDrawToggle(bool draw)
      {
         if (!draw)
         {
            mSwitchNode->setAllChildrenOff();
            if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__, "Hiding Model on BaseEntity \"s\".", GetName().c_str());
         }
         else
         {
            if(!draw)
               return;

            BaseEntityActorProxy::DamageStateEnum& damageState = GetDamageState();
            if(damageState == BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
               mSwitchNode->setSingleChildOn(0);
            else if(damageState == BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE 
                  || damageState == BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE)
               mSwitchNode->setSingleChildOn(1);
            else if(damageState == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
               mSwitchNode->setSingleChildOn(2);
           // mNode->asGroup()->addChild(mSwitchNode.get());
            if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__, "Showing Model on Platform \"%s\" with damage state \"%s\".", 
                     GetName().c_str(), GetDamageState().GetName().c_str());
         }

      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::LoadDamageableFile(const std::string &fileName, PlatformActorProxy::DamageStateEnum &state)
      {
         if(!fileName.empty())
         {
            if(state == PlatformActorProxy::DamageStateEnum::NO_DAMAGE)
            {
               if (mNonDamagedFileNode->getNumChildren() > 0)
               {
                  std::ostringstream oss;
                  oss << "Platform forced a reload of model files: File [" << fileName      << 
                         "], Actor Type[" << GetGameActorProxy().GetActorType().GetName() <<
                         "], Name[" << GetName() << "], Id[" << GetUniqueId().ToString()  <<
                         "].";

                  mLogger->LogMessage(dtUtil::Log::LOG_WARNING, 
                                      __FUNCTION__, __LINE__, oss.str().c_str());

                  //std::cout << "Platform - Loading NO DAMAGE File [" << fileName  << "] for actor type[" << 
                  //   GetGameActorProxy().GetActorType().GetName() << "].  Previous Num Children was[" << 
                  //   mNonDamagedFileNode->getNumChildren() << "], name[" << GetName() << "], Id[" << 
                  //   GetUniqueId().ToString() << "]." << std::endl;
               }
               mNonDamagedFileNode->removeChild(0,mNonDamagedFileNode->getNumChildren());

               dtCore::RefPtr<osg::Node> cachedOriginalNode;
               dtCore::RefPtr<osg::Node> copiedNode;
               if (!LoadFile(fileName, cachedOriginalNode, copiedNode, true))
                  throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, 
                  std::string("Model file could not be loaded: ") + fileName, __FILE__, __LINE__);
               copiedNode->setName("No Damage");
               mNonDamagedFileNode->addChild(copiedNode.get());
               mNonDamagedFileNode->setUserData(cachedOriginalNode.get());
               LoadNodeCollector(copiedNode.get());

               //osg::Node *node = mLoader.LoadFile(fileName, true);
               //if(node == NULL)
               //   throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, std::string("Model file could not be loaded: ") + fileName);
              
               //node->setName("No Damage");
               //mNonDamagedFileNode->addChild(node);
               //LoadNodeCollector(node);
            }
            else if(state == PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
            {
               mDamagedFileNode->removeChild(0,mDamagedFileNode->getNumChildren());

               dtCore::RefPtr<osg::Node> cachedOriginalNode;
               dtCore::RefPtr<osg::Node> copiedNode;
               if (!LoadFile(fileName, cachedOriginalNode, copiedNode, true))
                  throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, 
                  std::string("Model file could not be loaded: ") + fileName, __FILE__, __LINE__);
               copiedNode->setName("Slight Damage");
               mDamagedFileNode->addChild(copiedNode.get());
               mDamagedFileNode->setUserData(cachedOriginalNode.get());

               //osg::Node *node = mLoader.LoadFile(fileName, true);
               //if(node == NULL)
               //   throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, std::string("Model file could not be loaded: ") + fileName);

               //node->setName("Slight Damage");
               //mDamagedFileNode->addChild(node);
            }
            else if(state == PlatformActorProxy::DamageStateEnum::MODERATE_DAMAGE)
            {
               mDamagedFileNode->removeChild(0,mDamagedFileNode->getNumChildren());

               dtCore::RefPtr<osg::Node> cachedOriginalNode;
               dtCore::RefPtr<osg::Node> copiedNode;
               if (!LoadFile(fileName, cachedOriginalNode, copiedNode, true))
                  throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
                  std::string("Model file could not be loaded: ") + fileName, __FILE__, __LINE__);
               copiedNode->setName("Moderate Damage");
               mDamagedFileNode->addChild(copiedNode.get());
               mDamagedFileNode->setUserData(cachedOriginalNode.get());

               //osg::Node *node = mLoader.LoadFile(fileName, true);
               //if(node == NULL)
               //   throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, std::string("Model file could not be loaded: ") + fileName);

               //node->setName("Moderate Damage");
               //mDamagedFileNode->addChild(node);
            }
            else if(state == PlatformActorProxy::DamageStateEnum::DESTROYED)
            {
               mDestroyedFileNode->removeChild(0,mDestroyedFileNode->getNumChildren());

               dtCore::RefPtr<osg::Node> cachedOriginalNode;
               dtCore::RefPtr<osg::Node> copiedNode;
               if (!LoadFile(fileName, cachedOriginalNode, copiedNode, true))
                  throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
                  std::string("Model file could not be loaded: ") + fileName, __FILE__, __LINE__);
               copiedNode->setName("Destroyed");
               mDestroyedFileNode->addChild(copiedNode.get());
               mDestroyedFileNode->setUserData(cachedOriginalNode.get());

               //osg::Node *node = mLoader.LoadFile(fileName, true);
               //if(node == NULL)
               //   throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, std::string("Model file could not be loaded: ") + fileName);

               //node->setName("Destroyed");
               //mDestroyedFileNode->addChild(node);
            }
            else
               throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
               "Damage state is not supported", __FILE__, __LINE__);

            /// this resets the model now that new model files have been loaded n' stuff.
            InternalSetDamageState(GetDamageState());
         }
         else
         {
            if(state == PlatformActorProxy::DamageStateEnum::NO_DAMAGE)
            {
               mNonDamagedFileNode->removeChild(0,mNonDamagedFileNode->getNumChildren());
               mNonDamagedFileNode->setUserData(NULL);
            }
            else if(state == PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
            {
               mDamagedFileNode->removeChild(0,mDamagedFileNode->getNumChildren());
               mDamagedFileNode->setUserData(NULL);
            }
            else if(state == PlatformActorProxy::DamageStateEnum::MODERATE_DAMAGE)
            {
               mDamagedFileNode->removeChild(0,mDamagedFileNode->getNumChildren());
               mDamagedFileNode->setUserData(NULL);
            }
            else if(state == PlatformActorProxy::DamageStateEnum::DESTROYED)
            {
               mDestroyedFileNode->removeChild(0,mDestroyedFileNode->getNumChildren());
               mDestroyedFileNode->setUserData(NULL);
            }
            else
               throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, 
               "Damage state is not supported", __FILE__, __LINE__);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::OnShaderGroupChanged()
      {
         if (GetShaderGroup().empty())
            return;
         
         const dtCore::ShaderGroup *shaderGroup =
            dtCore::ShaderManager::GetInstance().FindShaderGroupPrototype(GetShaderGroup());

         //First get the shader group assigned to this actor.
         if (shaderGroup == NULL)
         {
            //mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,  
            //   "Could not find shader group: [" + GetShaderGroup() + "].");
            return;
         }

         //All DVTE entities have at least three possible shaders, one for each damage
         //state.  These specific shaders are searched for first.  If one is not found,
         //then if a default shader is specified in the shader group it will be assigned
         //to that damage state.  If all three damage state shaders are undefined and
         //a default shader exists, that shader is applied to the root switch node of
         //the actor so it gets passed on to all damage states.
         const dtCore::ShaderProgram *defaultShader = shaderGroup->GetDefaultShader();
         const dtCore::ShaderProgram *noDamage = shaderGroup->FindShader("NoDamage");
         const dtCore::ShaderProgram *moderate = shaderGroup->FindShader("ModerateDamage");
         const dtCore::ShaderProgram *destroyed = shaderGroup->FindShader("Destroyed");

         try
         {
            //First if all three are not present check the default, assign it, or return.
            if (noDamage == NULL && moderate == NULL && destroyed == NULL)
            {
               if (defaultShader != NULL)
               {
                  dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *mSwitchNode);
               }
               else
               {
                  //mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,  
                  //   "Could not find any shaders in shader group: [" + GetShaderGroup() + "].");
                  return;
               }
            }

            //Check the non damaged shader...
            if (noDamage != NULL)
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*noDamage,*mNonDamagedFileNode);
            else if (defaultShader != NULL)
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader,*mNonDamagedFileNode);

            //Check the moderate damage shader...
            if (moderate != NULL)
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*moderate,*mDamagedFileNode);
            else if (defaultShader != NULL)
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader,*mDamagedFileNode);

            //Check the destroyed shader...
            if (destroyed != NULL)
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*destroyed,*mDestroyedFileNode);
            else if (defaultShader != NULL)
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader,*mDestroyedFileNode);
         }
         catch (const dtUtil::Exception &e)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,  
               "Caught Exception while assigning shader: " + e.ToString());
            return;
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 Platform::GetEngineTransform()
      {
         dtCore::Transform xform;
         osg::Vec3 vec;

         GetTransform(xform, dtCore::Transformable::REL_CS);
         xform.GetTranslation(vec);

         return vec;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         GetGameActorProxy().RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_TIMER_ELAPSED, "TimeElapsedForSoundIdle");

         if (IsRemote())
         {
            dtCore::NodeCollector* nodeCollector = GetNodeCollector();
            if(nodeCollector != NULL && !nodeCollector->GetTransformNodeMap().empty())
               GetDeadReckoningHelper().SetNodeCollector(*nodeCollector);
         }
         else
         {
            GetDeadReckoningHelper().SetUpdateMode(dtGame::DeadReckoningHelper::UpdateMode::CALCULATE_ONLY);
         }

         RegisterWithDeadReckoningComponent();

         //// Curt - bump mapping
         dtCore::ShaderProgram *defaultShader = dtCore::ShaderManager::GetInstance().
            GetShaderInstanceForNode(GetOSGNode());
         dtCore::ShaderParamFloat *useBumpmappingParam = NULL;
         if (defaultShader != NULL)
            useBumpmappingParam = dynamic_cast<dtCore::ShaderParamFloat *>
               (defaultShader->FindParameter("useBumpMap"));

         // if bump mapping is turned on, generate the tangents to be passed to the shader
         if (useBumpmappingParam != NULL && useBumpmappingParam->GetValue() == 1.0f)
         {
            dtCore::RefPtr<dtCore::TangentSpaceVisitor> visitor = new dtCore::TangentSpaceVisitor
               ("vTangent", (osg::Program*)defaultShader->GetShaderProgram(), 6);
            mNonDamagedFileNode->accept(*visitor.get());
            mDamagedFileNode->accept(*visitor.get());
            mDestroyedFileNode->accept(*visitor.get());
         }
         
         if(!mSFXSoundIdleEffect.empty() && GetGameActorProxy().IsInGM())
            LoadSFXEngineIdleLoop();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetSFXEngineIdleLoop(const std::string& soundFX)
      {
         mSFXSoundIdleEffect = soundFX;
         if(!mSFXSoundIdleEffect.empty() && GetGameActorProxy().IsInGM())
            LoadSFXEngineIdleLoop();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::LoadSFXEngineIdleLoop()
      {
         if(mSndIdleLoop.valid())
         {
            mSndIdleLoop->Stop();
            RemoveChild(mSndIdleLoop.get());
            mSndIdleLoop.release();
         }

         if(!mSFXSoundIdleEffect.empty())
         {
            GetGameActorProxy().GetGameManager()->SetTimer("TimeElapsedForSoundIdle", &GetGameActorProxy(), 1, true);
            mSndIdleLoop = dtAudio::AudioManager::GetInstance().NewSound();
            mSndIdleLoop->LoadFile(mSFXSoundIdleEffect.c_str());
            mSndIdleLoop->ListenerRelative(false);
            mSndIdleLoop->SetLooping(true);
            mSndIdleLoop->SetMinDistance(mMinIdleSoundDistance);
            mSndIdleLoop->SetMaxDistance(mMaxIdleSoundDistance);
            AddChild(mSndIdleLoop.get());
            dtCore::Transform xform;
            mSndIdleLoop->SetTransform(xform, dtCore::Transformable::REL_CS);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::TickTimerMessage(const dtGame::Message& tickMessage)
      {
         UpdateEngineIdleSoundEffect();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::UpdateEngineIdleSoundEffect()
      {
         if(mSndIdleLoop == NULL)
            return;

         if(GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {
            if(mSndIdleLoop->IsPlaying())
               mSndIdleLoop->Stop();
            return;
         }

         dtCore::Transform ourTransform;
         dtCore::Transform cameraTransform;

         GetTransform(ourTransform);
         GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->GetTransform(cameraTransform);
         
         osg::Vec3 ourXYZ, cameraXYZ;

         ourXYZ = ourTransform.GetTranslation();
         cameraXYZ = cameraTransform.GetTranslation();
         
         osg::Vec3 distanceVector = ourXYZ - cameraXYZ;
         float distanceFormula = distanceVector.length2();
         if( (mMaxIdleSoundDistance * mMaxIdleSoundDistance ) <  distanceFormula)
         {
            mSndIdleLoop->Stop();
         }
         else
         {
            mSndIdleLoop->Play();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetArticulatedParametersArray(const dtDAL::NamedGroupParameter& newValue)
      {
         if( mArticHelper.valid() && GetNodeCollector() != NULL)
         {
            mArticHelper->HandleArticulatedParametersArray(
               newValue, *GetNodeCollector(), GetDeadReckoningHelper() );
         }
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::NamedGroupParameter> Platform::GetArticulatedParametersArray()
      {
         if( mArticHelper.valid() )
         {
            return mArticHelper->BuildGroupProperty();
         }

         return new dtDAL::NamedGroupParameter(Components::ArticulationHelper::PROPERTY_NAME_ARTICULATED_ARRAY);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::LoadNodeCollector(osg::Node* dofModel)
      {
         mNodeCollector = new dtCore::NodeCollector(dofModel, dtCore::NodeCollector::AllNodeTypes);
         GetDeadReckoningHelper().SetNodeCollector(*mNodeCollector);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtCore::NodeCollector* Platform::GetNodeCollector() 
      {
         return mNodeCollector.get();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetArticulationHelper( SimCore::Components::ArticulationHelper* articHelper )
      {
         mArticHelper = articHelper;
         if( mArticHelper.valid() )
         {
            mArticHelper->UpdateDOFReferences( mNodeCollector.get() );
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::TickControlState( const dtGame::Message& tickMessage )
      {
         if( mTimeUntilControlStateUpdate >= 0.0f )
         {
            mTimeUntilControlStateUpdate -= 
               static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaRealTime();
         }

         // Send control state update if controlling articulations directly
         // on a remote entity.
         if( mTimeUntilControlStateUpdate < 0.0f && mArticHelper.valid() && mArticHelper->IsDirty() )
         {
            if( mArticHelper->GetControlState() != NULL )
            {
               mTimeUntilControlStateUpdate = mTimeBetweenControlStateUpdates;

               mArticHelper->NotifyControlStateUpdate();
               mArticHelper->SetDirty( false );
            }
         }
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      bool Platform::ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
      {
         bool forceUpdate = BaseClass::ShouldForceUpdate(pos, rot, fullUpdate)
                               || (mArticHelper.valid() && mArticHelper->IsDirty());

         return forceUpdate;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill)
      {
         BaseClass::FillPartialUpdatePropertyVector(propNamesToFill);

         if( mArticHelper.valid() && mArticHelper->IsDirty() )
         {
            propNamesToFill.push_back( mArticHelper->GetArticulationArrayPropertyName() );
            mArticHelper->SetDirty(false);
         }
      }
   }
}
