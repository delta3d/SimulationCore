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
#include <CEGUIcolour.h>
#include <dtABC/application.h>
#include <dtUtil/stringutils.h>
#include <dtCore/camera.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gamemanager.inl>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/Components/LabelManager.h>

// TEMP:
#include <dtCore/system.h>
#include <dtCore/transform.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtUtil/boundingshapeutils.h>
#include <dtUtil/exception.h>
#include <dtUtil/log.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/threadpool.h>

#include <osg/FrameStamp>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osgUtil/SceneView>
#include <osgDB/WriteFile>
// TEMP:
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Math>

#include <OpenThreads/ScopedLock>

#include <iostream>
#include <queue>
#include <sstream>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // LABEL CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string HUDLabel::PROPERTY_COLOR("IconColour");
      const std::string HUDLabel::PROPERTY_TEXT("CallSign");
      const std::string HUDLabel::PROPERTY_LINE2("Line2Text");

      static const float TIME_BETWEEN_DEPTH_SORTS = 2.00f;

      //////////////////////////////////////////////////////////////////////////
      HUDLabel::HUDLabel( const std::string& name, const std::string& type )
      : SimCore::Components::HUDElement(name,type)
      , mZDepth(0.0f)
      , mSized(false)
      {
         SetDeleteWindowOnDestruct(true); // else labels never go away 
      }

      //////////////////////////////////////////////////////////////////////////
      HUDLabel::HUDLabel( CEGUI::Window& window )
      : SimCore::Components::HUDElement(window)
      , mZDepth(0.0f)
      , mSized(false)
      {
         SetDeleteWindowOnDestruct(true); // else labels never go away 
      }

      //////////////////////////////////////////////////////////////////////////
      HUDLabel::~HUDLabel()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::UpdateSize()
      {
         if (!mSized)
         {
            // Change the size base on the size of the font with text.
            CEGUI::Font* font = GetCEGUIWindow()->getFont();
            float width = font->getTextExtent(GetText());
            SetSize( width, font->getLineSpacing() );
            mSized = true;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::SetText(const std::string& text)
      {
         SetProperty(PROPERTY_TEXT, text);
         mSized = false;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string HUDLabel::GetText() const
      {
         return GetProperty(PROPERTY_TEXT);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::SetLine2(const std::string& line2Text)
      {
         SetProperty(PROPERTY_LINE2, line2Text);
         mSized = false;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string HUDLabel::GetLine2() const
      {
         return GetProperty(PROPERTY_LINE2);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::SetColor(const osg::Vec3& color)
      {
         float alpha = GetAlpha();
         CEGUI::colour ceguiColor( color.x(), color.y(), color.z(), alpha );
         std::string colorString( CEGUI::PropertyHelper::colourToString( ceguiColor ).c_str() );
         SetProperty( PROPERTY_COLOR, colorString );
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::SetColor(const osg::Vec4& color)
      {
         CEGUI::colour ceguiColor(color.x(), color.y(), color.z(), color.w());
         std::string colorString(CEGUI::PropertyHelper::colourToString(ceguiColor).c_str());
         SetProperty( PROPERTY_COLOR, colorString);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::GetColor(osg::Vec4& outColor) const
      {
         const std::string colorString( GetProperty( PROPERTY_COLOR ) );
         if( ! colorString.empty() )
         {
            CEGUI::colour cegguiColor( CEGUI::PropertyHelper::stringToColour( colorString ) );
            outColor.set( cegguiColor.getRed(), cegguiColor.getGreen(),
               cegguiColor.getBlue(), cegguiColor.getAlpha() );
         }
         else // Property not found
         {
            outColor.set( -1.0f, -1.0f, -1.0f, -1.0f );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::SetAlpha( float alpha )
      {
         osg::Vec4 color;
         GetColor( color );
         color.w() = alpha;
         SetColor( color );
      }

      //////////////////////////////////////////////////////////////////////////
      float HUDLabel::GetAlpha() const
      {
         osg::Vec4 color;
         GetColor( color );
         return color.w();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDLabel::SetZDepth( float depth )
      {
         mZDepth = depth;
      }

      //////////////////////////////////////////////////////////////////////////
      float HUDLabel::GetZDepth() const
      {
         return mZDepth;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDLabel::operator< ( const HUDLabel& other ) const
      {
         return mZDepth < other.mZDepth;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDLabel::operator> ( const HUDLabel& other ) const
      {
         return mZDepth > other.mZDepth;
      }

      //////////////////////////////////////////////////////////////////////////
      class CompareHUDLabelPointers
      {
      public:
         bool operator () (SimCore::Components::HUDLabel* one, SimCore::Components::HUDLabel* two)
         {
            return (*one) < (*two);
         }
      };



      //////////////////////////////////////////////////////////////////////////
      // LABEL OPTIONS CODE
      //////////////////////////////////////////////////////////////////////////
      LabelOptions::LabelOptions()
      : mMaxLabelDistance(500.0f)
      , mMaxLabelDistance2(mMaxLabelDistance * mMaxLabelDistance)
      , mShowDamageState(false)
      , mShowLabels(true)
      , mShowLabelsForEntities(true)
      , mShowLabelsForPositionReports(true)
      , mShowLabelsForBlips(true)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      LabelOptions::~LabelOptions()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::ShowLabels() const
      {
         return mShowLabels;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelOptions::SetShowLabels(bool show)
      {
         mShowLabels = show;
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::ShowDamageState() const
      {
         return mShowDamageState;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelOptions::SetShowDamageState(bool show)
      {
         mShowDamageState = show;
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::ShowLabelsForEntities() const
      {
         return mShowLabelsForEntities;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelOptions::SetShowLabelsForEntities(bool show)
      {
         mShowLabelsForEntities = show;
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::ShowLabelsForPositionReports() const
      {
         return mShowLabelsForPositionReports;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelOptions::SetShowLabelsForPositionReports(bool show)
      {
         mShowLabelsForPositionReports = show;
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::ShowLabelsForBlips() const
      {
         return mShowLabelsForBlips;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelOptions::SetShowLabelsForBlips(bool show)
      {
         mShowLabelsForBlips = show;
      }

      //////////////////////////////////////////////////////////////////////////
      float LabelOptions::GetMaxLabelDistance() const
      {
         return mMaxLabelDistance;
      }

      //////////////////////////////////////////////////////////////////////////
      float LabelOptions::GetMaxLabelDistance2() const
      {
         return mMaxLabelDistance2;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelOptions::SetMaxLabelDistance(float distance)
      {
         mMaxLabelDistance = distance;
         mMaxLabelDistance2 = mMaxLabelDistance * mMaxLabelDistance;
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::operator == (const LabelOptions& toCompare) const
      {
         /// using 0.5 for the epsilon because the values are really only ever set to whole numbers.
         return osg::equivalent(toCompare.mMaxLabelDistance, mMaxLabelDistance, 0.5f) &&
            toCompare.mShowLabels == mShowLabels &&
            toCompare.mShowLabelsForBlips == mShowLabelsForBlips &&
            toCompare.mShowLabelsForEntities == mShowLabelsForEntities &&
            toCompare.mShowLabelsForPositionReports == mShowLabelsForPositionReports &&
            toCompare.mShowDamageState == mShowDamageState;
      }

      //////////////////////////////////////////////////////////////////////////
      bool LabelOptions::operator() (dtDAL::BaseActorObject& proxy)
      {
         if (!mShowLabels)
         {
            return false;
         }

         bool result = false;

         if (!proxy.GetActorType().InstanceOf(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE))
         {
            if (proxy.GetActorType().InstanceOf(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE)
               || proxy.GetActorType().InstanceOf(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE))
            {
               result = mShowLabelsForEntities;
            }
            else if (proxy.GetActorType().InstanceOf(*SimCore::Actors::EntityActorRegistry::POSITION_MARKER_ACTOR_TYPE))
            {
               result = mShowLabelsForPositionReports;
            }
            else if (proxy.GetActorType().InstanceOf(*SimCore::Actors::EntityActorRegistry::BLIP_ACTOR_TYPE))
            {
               result = mShowLabelsForBlips;
            }
         }
         if (result)
         {
            SimCore::Actors::BaseEntity* entity = dynamic_cast<SimCore::Actors::BaseEntity*>(proxy.GetActor());
            // We don't want a label for something that isn't visible.
            if (entity == NULL || !entity->IsVisible())
            {
               result = false;
            }
         }
         return result;
      }

      //////////////////////////////////////////////////////////////////////////
      LabelManager::LabelManager()
      : mTimeUntilSort(0.0f)
      {
         AddSender( &dtCore::System::GetInstance() );
      }

      //////////////////////////////////////////////////////////////////////////
      LabelManager::~LabelManager()
      {
         RemoveSender( &dtCore::System::GetInstance() );
         ClearLabelsFromGUILayer();
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::SetGameManager( dtGame::GameManager* gm )
      {
         mGM = gm;
      }

      //////////////////////////////////////////////////////////////////////////
      dtGame::GameManager* LabelManager::GetGameManager()
      {
         return mGM.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const dtGame::GameManager* LabelManager::GetGameManager() const
      {
         return mGM.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::SetOptions(const LabelOptions& options )
      {
         mOptions = options;
      }

      //////////////////////////////////////////////////////////////////////////
      const LabelOptions& LabelManager::GetOptions() const
      {
         return mOptions;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::SetGUILayer( SimCore::Components::HUDElement* guiLayer )
      {
         // If the reference to the GUI layer is going to be lost...
         if( mGUILayer.valid() && guiLayer == NULL )
         {
            // ...remove all allocated labels from it.
            ClearLabelsFromGUILayer();
         }

         mGUILayer = guiLayer;
         //must enable z-ordering
         mGUILayer->GetCEGUIWindow()->setZOrderingEnabled(true);
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Components::HUDElement* LabelManager::GetGUILayer()
      {
         return mGUILayer.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Components::HUDElement* LabelManager::GetGUILayer() const
      {
         return mGUILayer.get();
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<SimCore::Components::HUDLabel> LabelManager::GetOrCreateLabel(dtDAL::BaseActorObject& actor)
      {
         // safely push all the received messages onto the GameManager message queue
         dtCore::RefPtr<SimCore::Components::HUDLabel> label;

         LabelMap::iterator i = mLastLabels.find(actor.GetId());

         if (i != mLastLabels.end())
         {
            label = i->second.get();
         }
         else
         {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mTaskMutex);

            SimCore::Actors::BaseEntity* entity = dynamic_cast<SimCore::Actors::BaseEntity*>(actor.GetActor());
            if (entity != NULL)
            {
               label = new SimCore::Components::HUDLabel(
                        actor.GetName() + dtCore::UniqueId().ToString(), "Label/EntityLabel");
               osg::Vec2 size;
               label->GetSize(size);
               //This label has two lines.
               label->SetSize(size.x(), size.y() * 2);
            }
            else
            {
               label = new SimCore::Components::HUDLabel(
                        actor.GetName() + dtCore::UniqueId().ToString(), "Label/ActorLabel");
            }

            // Insert label into list (Z-sorted insert).
            AddLabel( *label );
         }

         return label;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::AddLabel(SimCore::Components::HUDLabel& label )
      {
         if (!mGUILayer.valid())
         {
            throw dtUtil::Exception("Labels may not be added unless the gui layer has been set.",
                     __FUNCTION__, __LINE__);
         }

         // Attach the label CEGUI window to the main label layer's CEGUI window
         // so that it can be visible.
         mGUILayer->GetCEGUIWindow()->addChildWindow(label.GetCEGUIWindow());
      }

      class ApplyLabelTask : public dtUtil::ThreadPoolTask
      {
      public:
         ApplyLabelTask(LabelManager& labelManager, std::vector<dtDAL::BaseActorObject*>& actors, dtCore::Camera& camera, LabelManager::LabelMap& newLabels,
                  unsigned low, unsigned high)
         : mLabelManager(labelManager)
         , mActors(actors)
         , mCamera(&camera)
         , mNewLabels(newLabels)
         , mLow(low)
         , mHigh(high)
         {
         }

         virtual void operator () ()
         {
            for (unsigned i = mLow; i < mHigh; ++i)
            {
               mLabelManager.ApplyLabelToActor(*mActors[i], *mCamera, mNewLabels, nameBuffer);
            }
         }

         LabelManager& mLabelManager;
         std::vector<dtDAL::BaseActorObject*>& mActors;
         dtCore::RefPtr<dtCore::Camera> mCamera;
         LabelManager::LabelMap& mNewLabels;
         std::string nameBuffer;
         unsigned mLow, mHigh;
      };

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::Update(float dt)
      {
         // Get all entities from the game manager.
         typedef std::vector<dtDAL::BaseActorObject*> ProxyList;
         ProxyList proxies;

         // No need to do a find if when labels are off.
         if (GetOptions().ShowLabels())
         {
            LabelOptions options = GetOptions();
            mGM->FindActorsIf(options, proxies);
         }

         // Get the MAIN camera.
         dtCore::Camera* deltaCamera = mGM->GetApplication().GetCamera();

         mCEGUISortList.clear(); // This should always happen between frames.

         LabelMap newLabels;


         unsigned numTasks = (dtUtil::ThreadPool::GetNumImmediateWorkerThreads());
         unsigned  proxiesPerTask = (proxies.size() / numTasks) + 1;

         for (unsigned i = 0; i < numTasks; ++i)
         {
            unsigned low, high;
            low = i * proxiesPerTask;
            high = low + proxiesPerTask;
            // The last task won't have an many unless the number of actors is evenly divisible by the number of actors.
            high = dtUtil::Min(high, unsigned(proxies.size()));
            if (high > low)
            {
               dtUtil::ThreadPool::AddTask(*new ApplyLabelTask(*this, proxies, *deltaCamera, newLabels, low, high));
            }
         }

         dtUtil::ThreadPool::ExecuteTasks();

         //We stored add the labels we're using now in the new map, so we swap with the last set
         //for the next frame.
         mLastLabels.swap(newLabels);

         // We have to set the text and size of the labels in the main thread since
         // it has a valid glContext and the other threads don't
         LabelMap::iterator i = mLastLabels.begin();
         for (; i != mLastLabels.end(); ++i)
         {
            SimCore::Components::HUDLabel& label = *i->second;
            label.UpdateSize();
         }

         mTimeUntilSort -= dt;
         if (mTimeUntilSort < 0.0f)
         {
            mTimeUntilSort = TIME_BETWEEN_DEPTH_SORTS;

            CompareHUDLabelPointers compLabels;
            std::sort(mCEGUISortList.begin(), mCEGUISortList.end(), compLabels);

            CEGUISortList::iterator curWindow = mCEGUISortList.begin();
            CEGUISortList::iterator endWindowList = mCEGUISortList.end();
            unsigned int i = 0;
            for( ; curWindow != endWindowList; ++curWindow, ++i )
            {
               CEGUI::Window* w = (*curWindow)->GetCEGUIWindow();
               w->moveToBack();
            }
         }
      }

      void LabelManager::ApplyLabelToActor(dtDAL::BaseActorObject& proxy, dtCore::Camera& deltaCamera,
               LabelMap& newLabels, std::string& nameBuffer)
      {
         // Declare variables to be used for each loop iteration.
         dtCore::Transformable* actor = NULL;
         dtCore::RefPtr<SimCore::Components::HUDLabel> label;
         dtCore::Transform xform;
         osg::Vec3d worldPos;
         osg::Vec3d screenPos;


         // Get the current entity actor.
         proxy.GetActor(actor);

         osg::Vec3 center;
         float radius = 0.0f;
         actor->GetBoundingSphere(center, radius);

         // if we have a valid bounding sphere, use the center point.
         if (radius > 0.0)
         {
            worldPos = center;
         }
         else
         {
            dtCore::Transform xform;
            actor->GetTransform(xform);
            xform.GetTranslation(worldPos);
         }

         // Determine if entity is in view.
         if (!deltaCamera.ConvertWorldCoordinateToScreenCoordinate(worldPos, screenPos))
         {
            // Entity not in view, avoid setting a label for it.
            return;
         }

         dtCore::Transform camXform;
         deltaCamera.GetTransform(camXform);
         osg::Vec3 camPos;
         camXform.GetTranslation(camPos);
         if (GetOptions().GetMaxLabelDistance() > 0.0 &&
                  (camPos - worldPos).length2() > GetOptions().GetMaxLabelDistance2())
         {
            return;
         }

         // Find a label that is not being used.
         label = GetOrCreateLabel(proxy);

         dtDAL::StringActorProperty* mappingTypeProp = NULL;
         proxy.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_MAPPING_NAME, mappingTypeProp);
         if (mappingTypeProp != NULL)
         {
            nameBuffer = proxy.GetName() + "  /  " + mappingTypeProp->GetValue();
         }
         else
         {
            nameBuffer = proxy.GetName();
         }

         if (nameBuffer != label->GetText())
         {
            label->SetText(nameBuffer);
         }

         osg::Vec2 labelSize;
         label->GetSize(labelSize);

//            dtDAL::Vec3ActorProperty* velProp = NULL;
//            proxy.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_VELOCITY_VECTOR, velProp);
//            if (velProp != NULL)
//            {
//               label->SetVelocity(velProp->GetValue());
//            }

         dtDAL::ActorProperty* damProp = NULL;
         proxy.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_DAMAGE_STATE, damProp);
         if (damProp != NULL)
         {
            label->SetLine2(damProp->ToString());
         }

         AssignLabelColor(proxy, *label);

         osg::Vec2 pos = CalculateLabelScreenPosition(radius, center, deltaCamera, screenPos, labelSize);

         //x / 2 is to center it. subtracting the y size puts it at the top.
         label->SetPosition(pos.x(), pos.y());

         // Set the label's depth so that it can be sorted in a label sorting loop.
         label->SetZDepth( screenPos.z() );

         {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mTaskMutex);
            mCEGUISortList.push_back(label.get());
            newLabels.insert(std::make_pair(actor->GetUniqueId(), label));
         }

      }

      //////////////////////////////////////////////////////////////////////////
      const std::string LabelManager::AssignLabelColor( const dtDAL::BaseActorObject& actor, HUDLabel& label)
      {
         const SimCore::Actors::BaseEntityActorProxy::ForceEnum* force =
            &SimCore::Actors::BaseEntityActorProxy::ForceEnum::OTHER;

         const SimCore::Actors::BaseEntity* entity = dynamic_cast<const SimCore::Actors::BaseEntity*>(actor.GetActor());

         if (entity != NULL)
         {
            force = &entity->GetForceAffiliation();
         }

         static const std::string COLOR("_COLOR");

         const std::string& colorProp = label.GetProperty(force->GetName()+COLOR);
         if (!colorProp.empty())
         {
            label.SetProperty(HUDLabel::PROPERTY_COLOR, colorProp);
            return colorProp;
         }

         LOGN_INFO("LabelManager.cpp", std::string("Couldn't find property for label color, or the value was set to empty: ")
                  + force->GetName()+COLOR);
         //This shouldn't happen unless one of the properties is missing.
         label.SetColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
         return "";
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::ClearLabelsFromGUILayer()
      {
         size_t labelCount = mLastLabels.size();
         size_t labelAttachedCount = 0;

         if (mGUILayer.valid())
         {
            CEGUI::Window* layerWindow = mGUILayer->GetCEGUIWindow();

            labelAttachedCount = layerWindow->getChildCount();

            // Remove all allocated labels from the GUI layer.
            CEGUISortList::iterator curWindow = mCEGUISortList.begin();
            CEGUISortList::iterator endWindowList = mCEGUISortList.end();
            for( ; curWindow != endWindowList; ++curWindow )
            {
               layerWindow->removeChildWindow( (*curWindow)->GetCEGUIWindow() );
            }

            // Determine if there has been a difference
            size_t removeCount = labelAttachedCount - layerWindow->getChildCount();
            if( removeCount != labelCount )
            {
               LOG_WARNING( "Labels removal count does not match the number of labels expected to be removed\n\tExpected: " + dtUtil::ToString(labelCount)
                  + "\n\tRemoved:  " + dtUtil::ToString(removeCount) );
            }
         }

         mLastLabels.clear();
         mCEGUISortList.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec2 LabelManager::CalculateLabelScreenPosition(float boundRadius, const osg::Vec3& boundCenter,
               dtCore::Camera& deltaCam, const osg::Vec3& screenPos, const osg::Vec2& labelSize)
      {
         dtCore::Transform xform;
         deltaCam.GetTransform(xform);
         osg::Vec3 camUpVector;
         xform.GetRow(2, camUpVector);

         osg::Vec3 radiusScaledUpVector = camUpVector * boundRadius;

         osg::Vec3 viewSphereTopWorld = radiusScaledUpVector + boundCenter;

         osg::Vec2 result;

         osg::Vec3d viewSphereTopScreen;
         if (deltaCam.ConvertWorldCoordinateToScreenCoordinate(viewSphereTopWorld, viewSphereTopScreen))
         {
            result.set(viewSphereTopScreen.x(), viewSphereTopScreen.y());
         }
         else
         {
            result.set(screenPos.x(), screenPos.y());
         }

         //CEGUI does -0.5 to 0.5 for center alignment.
         result.x() -= 0.5;
         //CEGUI is top to bottom, while the screen pos is bottom to top.
         result.y() = -result.y();
//         result.x() -= labelSize.x() / 2;
//         result.y() -= labelSize.y();

         return result;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::CalculateOnScreenBoundingBox(const osg::BoundingBox& bbox,
               const dtCore::Camera& cam, osg::Vec3d& outBottomLeft, osg::Vec3d& outTopRight)
      {
         osg::Vec3d min(1.0, 1.0, 1.0), max(0.0, 0.0, 0.0);
         for (unsigned i = 0; i < 7; ++i)
         {
            osg::Vec3d pos = bbox.corner(i);
            osg::Vec3d screen;
            cam.ConvertWorldCoordinateToScreenCoordinate(pos, screen);
            min.x() = dtUtil::Min(min.x(), screen.x());
            max.x() = dtUtil::Max(max.x(), screen.x());
            min.y() = dtUtil::Min(min.y(), screen.y());
            max.y() = dtUtil::Max(max.y(), screen.y());
            min.z() = dtUtil::Min(min.z(), screen.z());
            max.z() = dtUtil::Max(max.z(), screen.z());
         }

         outBottomLeft = min;
         outTopRight = max;

      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::CalculateBoundingBox(dtCore::Transformable& transformable, osg::BoundingBox& outBB)
      {
         osg::MatrixTransform* g = transformable.GetMatrixNode();
         for (unsigned i = 0; i != g->getNumChildren(); ++i)
         {
            if (dynamic_cast<osg::Geode*>(g->getChild(i)) != 0)
            {
               g->removeChild(i, 1);
               break;
            }
         }

         dtUtil::BoundingBoxVisitor bbv;
         osg::Node* node = transformable.GetOSGNode();
         node->accept(bbv);
         outBB = bbv.mBoundingBox;
      }

      //////////////////////////////////////////////////////////////////////////
      void LabelManager::OnMessage( MessageData* data )
      {
         // In frame sync, the cameras have already had their view matrices updated
         // which we need, so here is where the update occurs.
         if( data->message == dtCore::System::MESSAGE_FRAME_SYNCH)
         {
            try
            {
               Update(*static_cast<const double*>(data->userData));
            }
            catch (const dtUtil::Exception& ex)
            {
               ex.LogException(dtUtil::Log::LOG_ERROR);
               throw;
            }
         }
      }

   }
}
