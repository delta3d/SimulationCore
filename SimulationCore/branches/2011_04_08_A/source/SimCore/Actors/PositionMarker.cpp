#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/PositionMarker.h>
#include <SimCore/VisibilityOptions.h>

#include <dtGame/basemessages.h>
#include <dtGame/invokable.h>
#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>


#include <dtDAL/enginepropertytypes.h>

#include <dtUtil/functor.h>
#include <dtUtil/log.h>
#include <dtUtil/datetime.h>
#include <dtUtil/mathdefines.h>

#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Uniform>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////
      const std::string PositionMarker::COLOR_UNIFORM("forceColor");

      ////////////////////////////////////////////////////////////////////////
      PositionMarker::PositionMarker(dtGame::GameActorProxy& proxy)
         : BaseClass(proxy)
         , mReportTime(0.0)
         , mStaleTime(0.0)
         , mFadeOutTime(0.0)
         , mInitialAlpha(1.0)
         , mDeleteOnFadeOut(false)
         , mAssociatedEntity(NULL)
         , mSourceForce(&BaseEntityActorProxy::ForceEnum::OTHER)
         , mSourceService(&BaseEntityActorProxy::ServiceEnum::OTHER)
         , mFriendlyColor(0.5, 0.5, 1.0, 1.0)
         , mNeutralColor(0.1, 1.0, 0.1, 1.0)
         , mOpposingColor(1.0, 0.1, 0.1, 1.0)
         , mOtherColor(0.5, 0.5, 0.5, 1.0)
         , mStaleColor(0.6, 0.6, 0.6, 1.0)
      {
         dtCore::RefPtr<osg::Node> original, copied;
         IGActor::LoadFileStatic("StaticMeshes/Hemisphere.ive", original, copied);
         //gotta hold onto the new one.
         copied->setUserData(original.get());
         dtCore::RefPtr<osg::MatrixTransform> mt = new osg::MatrixTransform();
         mt->addChild(copied.get());
         osg::Matrix m;
         m.setTrans(osg::Vec3(0.0, 0.0, 3.0));
         mt->setMatrix(m);
         osg::Group* g = GetOSGNode()->asGroup();
         g->addChild(mt.get());

         osg::StateSet* ss = g->getOrCreateStateSet();
         ss->setMode(GL_BLEND,osg::StateAttribute::ON);
         dtCore::RefPtr<osg::BlendFunc> trans = new osg::BlendFunc();
         trans->setFunction( osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
         ss->setAttributeAndModes(trans.get());
         ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

         //Need to call the current incarnation to make it set the color.
         SetForceAffiliation(GetForceAffiliation());
         mLogger = &dtUtil::Log::GetInstance("PositionMarker.cpp");
      }

      ////////////////////////////////////////////////////////////////////////
      PositionMarker::~PositionMarker()
      {
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetReportTime(double reportTime)
      {
         mReportTime = reportTime;

         //mReportTime = GetGameActorProxy().GetGameManager()->GetSimulationTime();
         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_INFO))
         {
            std::ostringstream ss;
            ss << GetMappingName() << ": " << reportTime << " " << GetGameActorProxy().GetGameManager()->GetSimulationTime();
            mLogger->LogMessage(dtUtil::Log::LOG_INFO, __FILE__, __LINE__,  ss.str());
         }
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetStaleTime(float timeToFadeOut)
      {
         mStaleTime = timeToFadeOut;
      }

      ////////////////////////////////////////////////////////////////////////
      float PositionMarker::GetStaleTime() const
      {
         return mStaleTime;
      }

      ////////////////////////////////////////////////////////////////////////
      double PositionMarker::GetReportTime() const
      {
         return mReportTime;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetFadeOutTime(float time)
      {
         mFadeOutTime = time;
      }

      ////////////////////////////////////////////////////////////////////////
      float PositionMarker::GetFadeOutTime() const
      {
         return mFadeOutTime;
      }


      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetDeleteOnFadeOut(bool deleteOnFadeOut)
      {
         mDeleteOnFadeOut = deleteOnFadeOut;
      }

      ////////////////////////////////////////////////////////////////////////
      bool PositionMarker::GetDeleteOnFadeOut() const
      {
         return mDeleteOnFadeOut;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::UpdateColorForForce()
      {
         osg::Vec4 newColor;
         BaseEntityActorProxy::ForceEnum& markerForce = GetForceAffiliation();
         if (markerForce == BaseEntityActorProxy::ForceEnum::FRIENDLY)
         {
            newColor = GetFriendlyColor();
         }
         else if (markerForce == BaseEntityActorProxy::ForceEnum::NEUTRAL)
         {
            newColor = GetNeutralColor();
         }
         else if (markerForce == BaseEntityActorProxy::ForceEnum::INSURGENT ||
                  markerForce == BaseEntityActorProxy::ForceEnum::OPPOSING)
         {
            newColor = GetOpposingColor();
         }
         else
         {
            newColor = GetOtherColor();
         }
         osg::Vec3 newColor3(newColor.x(), newColor.y(), newColor.z());
         SetInitialColor(newColor3);
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetForceAffiliation(BaseEntityActorProxy::ForceEnum& markerForce)
      {
         BaseClass::SetForceAffiliation(markerForce);
         UpdateColorForForce();
      }


      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetSourceForce(BaseEntityActorProxy::ForceEnum& sourceForce)
      {
         mSourceForce = &sourceForce;
      }

      ////////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::ForceEnum& PositionMarker::GetSourceForce() const
      {
         return *mSourceForce;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetSourceService(BaseEntityActorProxy::ServiceEnum& sourceService)
      {
         mSourceService = &sourceService;
      }

      ////////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::ServiceEnum& PositionMarker::GetSourceService() const
      {
         return *mSourceService;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetSourceCallsign(const std::string& sourceCallsign)
      {
         mSourceCallsign = sourceCallsign;
      }

      ////////////////////////////////////////////////////////////////////////
      const std::string& PositionMarker::GetSourceCallsign() const
      {
         return mSourceCallsign;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetAssociatedEntity(BaseEntity* assocEntity)
      {
         mAssociatedEntity = assocEntity;
      }

      ////////////////////////////////////////////////////////////////////////
      const BaseEntity* PositionMarker::GetAssociatedEntity() const
      {
         return mAssociatedEntity.get();
      }

      ////////////////////////////////////////////////////////////////////////
      BaseEntity* PositionMarker::GetAssociatedEntity()
      {
         return mAssociatedEntity.get();
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetInitialAlpha(float alpha)
      {
         dtUtil::Clamp(alpha, 0.0f, 1.0f);
         mInitialAlpha = alpha;
      }

      ////////////////////////////////////////////////////////////////////////
      float PositionMarker::GetInitialAlpha() const
      {
         return mInitialAlpha;
      }

      ////////////////////////////////////////////////////////////////////////
      float PositionMarker::CalculateCurrentAlpha() const
      {
         float fadeOutFraction = CalculateFadeOutFraction();
         float alpha = mInitialAlpha - (fadeOutFraction * fadeOutFraction);
         return alpha;
      }

      ////////////////////////////////////////////////////////////////////////
      osg::Vec3 PositionMarker::CalculateCurrentColor() const
      {
         osg::Vec3 staleVec3(mStaleColor.x(), mStaleColor.y(), mStaleColor.z());
         osg::Vec3 colorDiff = staleVec3 - mInitialColor;
         float fadeOutFraction = CalculateStaleFraction();
         colorDiff *= fadeOutFraction;
         return mInitialColor + colorDiff;
      }

      ////////////////////////////////////////////////////////////////////////
      float PositionMarker::CalculateFadeOutFraction() const
      {
         if (GetFadeOutTime() <= 0.0)
         {
            return 0.0;
         }
         double simTime = GetGameActorProxy().GetGameManager()->GetSimulationTime();
         float ageOfMarker = simTime - GetReportTime();
         float fadeOutFraction = ageOfMarker / (double(GetFadeOutTime()) * 60.0);
         dtUtil::Clamp(fadeOutFraction, 0.0f, 1.0f);
         return fadeOutFraction;
      }

      ////////////////////////////////////////////////////////////////////////
      float PositionMarker::CalculateStaleFraction() const
      {
         if (GetStaleTime() <= 0.0)
         {
            return 0.0;
         }
         double simTime = GetGameActorProxy().GetGameManager()->GetSimulationTime();
         float ageOfMarker = simTime - GetReportTime();
         float fadeOutFraction = ageOfMarker / (double(GetStaleTime()) * 60.0);;
         dtUtil::Clamp(fadeOutFraction, 0.0f, 1.0f);
         return fadeOutFraction;
      }


      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetInitialColor(const osg::Vec3& vec)
      {
         mInitialColor = vec;
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec3& PositionMarker::GetInitialColor() const
      {
         return mInitialColor;
      }

      ////////////////////////////////////////////////////////////////////////
      osg::Vec4 PositionMarker::GetInitialColorWithAlpha() const
      {
         return osg::Vec4(mInitialColor, mInitialAlpha);
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetCurrentColorUniform(const osg::Vec4& vec)
      {
         osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
         osg::Uniform* uniform = ss->getOrCreateUniform(COLOR_UNIFORM, osg::Uniform::FLOAT_VEC4, 1);
         uniform->set(vec);
      }

      ////////////////////////////////////////////////////////////////////////
      osg::Vec4 PositionMarker::GetCurrentColorUniform()
      {
         osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
         osg::Uniform* uniform = ss->getOrCreateUniform(COLOR_UNIFORM, osg::Uniform::FLOAT_VEC4, 1);
         osg::Vec4 returnVal;
         uniform->get(returnVal);
         return returnVal;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetFriendlyColor(const osg::Vec4& vec)
      {
         mFriendlyColor = vec;
         UpdateColorForForce();
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetFriendlyColor() const
      {
         return mFriendlyColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetNeutralColor(const osg::Vec4& vec)
      {
         mNeutralColor = vec;
         UpdateColorForForce();
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetNeutralColor() const
      {
         return mNeutralColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetOpposingColor(const osg::Vec4& vec)
      {
         mOpposingColor = vec;
         UpdateColorForForce();
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetOpposingColor() const
      {
         return mOpposingColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetOtherColor(const osg::Vec4& vec)
      {
         mOtherColor = vec;
         UpdateColorForForce();
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetOtherColor() const
      {
         return mOtherColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetStaleColor(const osg::Vec4& vec)
      {
         mStaleColor = vec;
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetStaleColor() const
      {
         return mStaleColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::LoadImage(const std::string& theFile)
      {
         if (theFile.empty())
         {
            osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
            ss->setTextureAttributeAndModes(0, NULL, osg::StateAttribute::OFF);
            return;
         }

         dtCore::RefPtr<osg::Image> icon  = osgDB::readImageFile( theFile );
         if(icon == NULL)
         {
            LOG_ERROR(std::string("Couldn't find image file \"") + theFile + "\"");
            osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
            ss->setTextureAttributeAndModes(0, NULL, osg::StateAttribute::OFF);
            return;
         }

         dtCore::RefPtr<osg::Texture2D> tex = new osg::Texture2D;
         tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP );
         tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP );
         tex->setImage( icon.get() );

         osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
         ss->setTextureAttributeAndModes(0, tex.get(), osg::StateAttribute::ON);
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         //RegisterWithDeadReckoningComponent(); // moved to base class
         // Register always.  this could be revisited, but the logic is somewhat complex;
         GetGameActorProxy().RegisterForMessagesAboutSelf(
                  dtGame::MessageType::INFO_TIMER_ELAPSED,
                  PositionMarkerActorProxy::INVOKABLE_TIME_ELAPSED);

         GetGameActorProxy().GetGameManager()->SetTimer(__FILE__, &GetGameActorProxy(), 8.0, true, true);

         SetCurrentColorUniform(osg::Vec4(CalculateCurrentColor(), CalculateCurrentAlpha()));
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::OnTimer(const dtGame::TimerElapsedMessage& timerElapsed)
      {
         if (CalculateFadeOutFraction() >= 1.0 && GetDeleteOnFadeOut())
         {
            //Bug, this won't record properly.  The actor is remote, but it must time itself out.
            // I could probably send myself a delete message :-).
            GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
         }
         else
         {
            SetCurrentColorUniform(osg::Vec4(CalculateCurrentColor(), CalculateCurrentAlpha()));
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      class HideNodeCallback : public osg::NodeCallback
      {
         public:

            /**
             * Constructor.
             *
             * @param terrain the owning InfiniteTerrain object
             */
            HideNodeCallback()
            {}

            virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
            {
               // We're hiding the node.  The point is to NOT traverse.
               // traverse(node, nv);
            }
      };

      ////////////////////////////////////////////////////////////////////////////////////
      bool PositionMarker::ShouldBeVisible(const SimCore::VisibilityOptions& options)
      {
         const BasicVisibilityOptions& basicOptions = options.GetBasicOptions();
         // this is kind of Dr. Seuss-ish, but if it's not a blip it's a track, but seeing as tracks look
         // exactly like blips, it's mighty hard telling the tracks from the blips except that blips
         // get the entity type "Blip" set.
         return (basicOptions.mSensorBlips && GetMappingName() == "Blip")
         || (basicOptions.mTracks && GetMappingName() != "Blip");
      }

      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      /////////////    Actor Proxy    ////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////

      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SOURCE_CALLSIGN("Source Callsign");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SOURCE_FORCE("Source Force");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SOURCE_SERVICE("Source Service");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_REPORT_TIME("Report Time");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_FADE_OUT_TIME("Fade Out Time");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_STALE_TIME("Stale Time");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_DELETE_ON_FADE_OUT("Delete After Fade Out");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_ASSOCIATED_ENTITY("Associated Real Entity");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_MARKER_COLOR("Marker Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_FRIENDLY_COLOR("Friendly Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_NEUTRAL_COLOR("Neutral Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_OPPOSING_COLOR("Opposing Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_OTHER_COLOR("Other Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_STALE_COLOR("Stale Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_ICON_IMAGE("Icon Image");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_INITIAL_ALPHA("Initial Alpha");

      const dtUtil::RefString PositionMarkerActorProxy::INVOKABLE_TIME_ELAPSED("Time elapsed PM");

      ////////////////////////////////////////////////////////////////////////
      PositionMarkerActorProxy::PositionMarkerActorProxy()
      {
         SetClassName("SimCore::Actors::PositionMarker");
      }

      ////////////////////////////////////////////////////////////////////////
      PositionMarkerActorProxy::~PositionMarkerActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      void PositionMarkerActorProxy::CreateActor()
      {
         PositionMarker* marker = new PositionMarker(*this);
         SetActor(*marker);
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarkerActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         static const dtUtil::RefString POSITION_MARKER_GROUP("Position Marker");

         PositionMarker* pm = NULL;
         GetActor(pm);

         static const dtUtil::RefString PROPERTY_SOURCE_FORCE_DESC("The force of the entity that reported this.");
         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>(
            PROPERTY_SOURCE_FORCE, PROPERTY_SOURCE_FORCE,
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>::SetFuncType(pm, &PositionMarker::SetSourceForce),
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>::GetFuncType(pm, &PositionMarker::GetSourceForce),
            PROPERTY_SOURCE_FORCE_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_SOURCE_SERVICE_DESC("The service of the entity that reported this.");
         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>(
            PROPERTY_SOURCE_SERVICE, PROPERTY_SOURCE_SERVICE,
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>::SetFuncType(pm, &PositionMarker::SetSourceService),
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>::GetFuncType(pm, &PositionMarker::GetSourceService),
            PROPERTY_SOURCE_SERVICE_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_SOURCE_CALLSIGN_DESC("The callsign of the entity that reported this.");
         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_SOURCE_CALLSIGN, PROPERTY_SOURCE_CALLSIGN,
            dtDAL::StringActorProperty::SetFuncType(pm, &PositionMarker::SetSourceCallsign),
            dtDAL::StringActorProperty::GetFuncType(pm, &PositionMarker::GetSourceCallsign),
            PROPERTY_SOURCE_CALLSIGN_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_REPORT_TIME_DESC("The time the entity reported this.");
         AddProperty(new dtDAL::DoubleActorProperty(
            PROPERTY_REPORT_TIME, PROPERTY_REPORT_TIME,
            dtDAL::DoubleActorProperty::SetFuncType(pm, &PositionMarker::SetReportTime),
            dtDAL::DoubleActorProperty::GetFuncType(pm, &PositionMarker::GetReportTime),
            PROPERTY_REPORT_TIME_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_FADE_OUT_TIME_DESC("The time in minutes it takes for the marker to fade out.");
         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_FADE_OUT_TIME, PROPERTY_FADE_OUT_TIME,
            dtDAL::FloatActorProperty::SetFuncType(pm, &PositionMarker::SetFadeOutTime),
            dtDAL::FloatActorProperty::GetFuncType(pm, &PositionMarker::GetFadeOutTime),
            PROPERTY_FADE_OUT_TIME_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_STALE_TIME_DESC("The time in minutes it takes for the marker to become stale/old.");
         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_STALE_TIME, PROPERTY_STALE_TIME,
            dtDAL::FloatActorProperty::SetFuncType(pm, &PositionMarker::SetStaleTime),
            dtDAL::FloatActorProperty::GetFuncType(pm, &PositionMarker::GetStaleTime),
            PROPERTY_STALE_TIME_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_DELETE_ON_FADE_OUT_DESC("Delete this position marker when actor fades out.");
         AddProperty(new dtDAL::BooleanActorProperty(
            PROPERTY_DELETE_ON_FADE_OUT, PROPERTY_DELETE_ON_FADE_OUT,
            dtDAL::BooleanActorProperty::SetFuncType(pm, &PositionMarker::SetDeleteOnFadeOut),
            dtDAL::BooleanActorProperty::GetFuncType(pm, &PositionMarker::GetDeleteOnFadeOut),
            PROPERTY_DELETE_ON_FADE_OUT_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_ASSOCIATED_ENTITY_DESC("The entity this position marker represents.");
         AddProperty(new dtDAL::ActorActorProperty(*this,
            PROPERTY_ASSOCIATED_ENTITY, PROPERTY_ASSOCIATED_ENTITY,
            dtDAL::ActorActorProperty::SetFuncType(this, &PositionMarkerActorProxy::SetAssociatedEntity),
//          Why doesn't this work?
//            dtDAL::ActorActorProperty::GetFuncType(pm, &PositionMarker::GetAssociatedEntity),
            dtDAL::ActorActorProperty::GetFuncType(this, &PositionMarkerActorProxy::GetAssociatedEntity),
            PROPERTY_ASSOCIATED_ENTITY_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_MARKER_COLOR_DESC("The color of the marker.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_MARKER_COLOR, PROPERTY_MARKER_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetInitialColorWithAlpha),
                  PROPERTY_MARKER_COLOR_DESC, POSITION_MARKER_GROUP
         ));
         GetProperty(PROPERTY_MARKER_COLOR)->SetReadOnly(true);

         static const dtUtil::RefString PROPERTY_FRIENDLY_COLOR_DESC("The color if the force is friendly.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_FRIENDLY_COLOR, PROPERTY_FRIENDLY_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetFriendlyColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetFriendlyColor),
                  PROPERTY_FRIENDLY_COLOR_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_NEUTRAL_COLOR_DESC("The color if the force is neutral.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_NEUTRAL_COLOR, PROPERTY_NEUTRAL_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetNeutralColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetNeutralColor),
                  PROPERTY_NEUTRAL_COLOR_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_OPPOSING_COLOR_DESC("The color if the force is opposing or insurgent.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_OPPOSING_COLOR, PROPERTY_OPPOSING_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetOpposingColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetOpposingColor),
                  PROPERTY_OPPOSING_COLOR_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_OTHER_COLOR_DESC("The color if the force is other.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_OTHER_COLOR, PROPERTY_OTHER_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetOtherColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetOtherColor),
                  PROPERTY_OTHER_COLOR_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_STALE_COLOR_DESC("The color if the force is other.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_STALE_COLOR, PROPERTY_STALE_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetStaleColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetStaleColor),
                  PROPERTY_STALE_COLOR_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_ICON_IMAGE_DESC("This image represents the rough "
                  "type of the entity marked by this position.");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TEXTURE,
                  PROPERTY_ICON_IMAGE, PROPERTY_ICON_IMAGE,
                  dtDAL::ResourceActorProperty::SetFuncType(pm, &PositionMarker::LoadImage),
                  PROPERTY_ICON_IMAGE_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_INITIAL_ALPHA_DESC("The initial alpha, i.e. transparency, of the marker."
                  "Valid values are 0.0 - 1.0 where 0.0 is invisible.");
         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_INITIAL_ALPHA, PROPERTY_INITIAL_ALPHA,
            dtDAL::FloatActorProperty::SetFuncType(pm, &PositionMarker::SetInitialAlpha),
            dtDAL::FloatActorProperty::GetFuncType(pm, &PositionMarker::GetInitialAlpha),
            PROPERTY_INITIAL_ALPHA_DESC, POSITION_MARKER_GROUP));
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarkerActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();

         PositionMarker* pm = NULL;
         GetActor(pm);

         AddInvokable(*new dtGame::Invokable(INVOKABLE_TIME_ELAPSED,
                  dtUtil::MakeFunctor(&PositionMarker::OnTimer, *pm)));
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarkerActorProxy::SetAssociatedEntity(dtDAL::ActorProxy* assocEntity)
      {
         SetLinkedActor(PROPERTY_ASSOCIATED_ENTITY, assocEntity);
         PositionMarker* pm = NULL;
         GetActor(pm);

         ///I want it to crash if pm is NULL;
         if (assocEntity == NULL)
         {
            pm->SetAssociatedEntity(NULL);
         }
         else
         {
            BaseEntity* entity = NULL;;
            assocEntity->GetActor(entity);
            pm->SetAssociatedEntity(entity);
         }
      }

      ////////////////////////////////////////////////////////////////////////
      dtCore::DeltaDrawable* PositionMarkerActorProxy::GetAssociatedEntity()
      {
         PositionMarker* pm = NULL;
         GetActor(pm);
         return pm->GetAssociatedEntity();
      }

   }
}
