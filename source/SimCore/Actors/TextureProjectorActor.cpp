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
*/
#include <prefix/SimCorePrefix.h>

#include <SimCore/Actors/TextureProjectorActor.h>
#include <SimCore/Components/TextureProjectorComponent.h>

#include <dtCore/resourcedescriptor.h>
#include <dtCore/transform.h>

#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>

#include <osg/Image>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

namespace SimCore
{
   namespace Actors
   {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TextureProjectorActor::TextureProjectorActor(dtGame::GameActorProxy &proxy) : dtGame::GameActor(proxy)
         , mCurrentTime(0)
         , mMaxTime(5)
         , mCurrentAlpha(0)
      {
         mImageProjectorFile = "";
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TextureProjectorActor::~TextureProjectorActor()
      {
         if(mProjector.valid())
            mProjector->off();
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         mCurrentTime +=
            tickMessage.GetDeltaSimTime();
      }
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorActor::OnEnteredWorld()
      {
         if(mEntityToAttachTo == NULL)
         {
            LOG_ERROR("mEntityToAttachTo was null when TextureProjectorActor was\
                      trying to OnEnteredWorld(), Bailing out");
            return;
         }

         Components::TextureProjectorComponent* tpComponent =
            dynamic_cast<Components::TextureProjectorComponent*>(
                     GetGameActorProxy().GetGameManager()->GetComponentByName(
                              Components::TextureProjectorComponent::DEFAULT_NAME));
         if(tpComponent == NULL)
         {
            LOG_ERROR("Couldnt find texture projector component bailing out,\
                      make sure it was initialized first!");
            return;
         }

         osg::ref_ptr<osg::Image> spotImage  = osgDB::readImageFile( mImageProjectorFile );
         if(spotImage == NULL)
         {
            LOG_ERROR("Couldnt find image file for the projectors image file.\
                      Bailing out of function OnEnteredWorld()!");
            return;
         }

         dtCore::Transform ourTransform;
         GetTransform(ourTransform);
         osg::Matrix ourMatrix;
         osg::Vec3 currentPos, lookAtPos;
         ourTransform.GetTranslation(lookAtPos);
         lookAtPos[2] -= 10;
         ourTransform.GetTranslation(currentPos);
         ourTransform.Set(currentPos, lookAtPos, osg::Vec3(0,0,1));
         ourTransform.Get(ourMatrix);

         osg::ref_ptr<osg::Texture2D> tex    = new osg::Texture2D;
         tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP );
         tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP );
         tex->setImage( spotImage.get() );

         //tx1->setMatrix(ourMatrix);

         //osg::ref_ptr<osg::MatrixTransform> tx2 = new osg::MatrixTransform;
         //osg::ref_ptr<osg::Node> projectorModel = osgDB::readNodeFile("C:/Code/light.osg" );
         //tx2->setMatrix(ourMatrix);
         //tx2->addChild(projectorModel.get());

         //osg::Quat quaterion;
         //quaterion.set(ourMatrix);

         mProjector = new Projector(tex.get(), 2);
         mProjector->setPositionAndAttitude(ourMatrix);
         //mProjector->setPositionAndAttitude(ourTransform.GetTranslation(), quaterion);
         mProjector->setFOV(75 * (M_PI/180));
         mProjector->on();

         //mProjector->setPositionAndAttitude(ourMatrix);
         mEntityToAttachTo->setStateSet(mProjector.get());
         //mEntityToAttachTo->getParent(0)->addChild(tx2.get());


         //mProjector = new Projector(2, osg::DegreesToRadians(90.0f) );
         //osg::ref_ptr<osg::MatrixTransform> sourceMatrix = new osg::MatrixTransform;//ourMatrix;
         //sourceMatrix->setMatrix(ourMatrix);
         //sourceMatrix->addChild(mProjector.get());
         //osg::StateSet *sset  = mEntityToAttachTo->getOrCreateStateSet();//->setStateSet(mProjector.get());
         //sset->setTextureAttributeAndModes(1, tex.get() );
         //sset->setTextureMode( 1, GL_TEXTURE_2D,    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
         //sset->setTextureMode( 1, GL_TEXTURE_GEN_S, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
         //sset->setTextureMode( 1, GL_TEXTURE_GEN_T, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
         //sset->setTextureMode( 1, GL_TEXTURE_GEN_R, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
         //sset->setTextureMode( 1, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
         //mEntityToAttachTo->getParent(0)->addChild(sourceMatrix.get());

         tpComponent->AddTextureProjectorActorToComponent(*this);
      }

      ////////////////////////////////////////////////////////////////////
      // Actor Proxy Below here
      ////////////////////////////////////////////////////////////////////
      TextureProjectorActorProxy::TextureProjectorActorProxy()
      {
         SetClassName("TextureProjectorActor");
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorActorProxy::BuildPropertyMap()
      {
         const std::string GROUP = "TextureProjectorActor";
         dtGame::GameActorProxy::BuildPropertyMap();
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TextureProjectorActorProxy::~TextureProjectorActorProxy()
      {}

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorActorProxy::CreateDrawable()
      {
         SetDrawable(*new TextureProjectorActor(*this));
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void TextureProjectorActorProxy::OnEnteredWorld()
      {
         dtGame::GameActorProxy::OnEnteredWorld();
         RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

         if (IsRemote())
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         else
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
      }
   }
} // dvte
