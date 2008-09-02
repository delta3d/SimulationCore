#include <SimCore/Actors/PositionMarker.h>
#include <dtDAL/enginepropertytypes.h>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osg/Uniform>
#include <osg/MatrixTransform>

namespace SimCore
{
   namespace Actors
   {
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

      ////////////////////////////////////////////////////////////////////////
      const std::string PositionMarker::COLOR_UNIFORM("forceColor");

      ////////////////////////////////////////////////////////////////////////
      PositionMarker::PositionMarker(dtGame::GameActorProxy& proxy)
         : BaseClass(proxy)
         , mReportTime(0.0)
         , mAssociatedEntity(NULL)
         , mSourceForce(&BaseEntityActorProxy::ForceEnum::OTHER)
         , mSourceService(&BaseEntityActorProxy::ServiceEnum::OTHER)
         , mFriendlyColor(0.5, 0.5, 1.0, 1.0)
         , mNeutralColor(0.1, 1.0, 0.1, 1.0)
         , mOpposingColor(1.0, 0.1, 0.1, 1.0)
         , mOtherColor(0.5, 0.5, 0.5, 1.0)
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
         ss->setAttributeAndModes( trans.get() );
         ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

         //Need to call the current incarnation to make it set the color.
         SetForceAffiliation(GetForceAffiliation());
      }

      ////////////////////////////////////////////////////////////////////////
      PositionMarker::~PositionMarker()
      {
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetReportTime(double reportTime)
      {
         mReportTime = reportTime;
      }

      ////////////////////////////////////////////////////////////////////////
      double PositionMarker::GetReportTime() const
      {
         return mReportTime;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetForceAffiliation(BaseEntityActorProxy::ForceEnum& markerForce)
      {
         BaseClass::SetForceAffiliation(markerForce);
         osg::Vec4 newColor;
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
         newColor.a() = 0.8;
         SetColor(newColor);
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
      void PositionMarker::SetColor(const osg::Vec4& vec)
      {
         osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
         osg::Uniform* uniform = ss->getOrCreateUniform(COLOR_UNIFORM, osg::Uniform::FLOAT_VEC4, 1);
         uniform->set(vec);
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4 PositionMarker::GetColor()
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
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetFriendlyColor()
      {
         return mFriendlyColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetNeutralColor(const osg::Vec4& vec)
      {
         mNeutralColor = vec;
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetNeutralColor()
      {
         return mNeutralColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetOpposingColor(const osg::Vec4& vec)
      {
         mOpposingColor = vec;
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetOpposingColor()
      {
         return mOpposingColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetOtherColor(const osg::Vec4& vec)
      {
         mOtherColor = vec;
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetOtherColor()
      {
         return mOtherColor;
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::HandleModelDrawToggle(bool active)
      {
         if (active)
         {
            GetOSGNode()->setCullCallback(NULL);
         }
         else
         {
            GetOSGNode()->setCullCallback(new HideNodeCallback);
         }
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
         RegisterWithDeadReckoningComponent();
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
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_ASSOCIATED_ENTITY("Associated Real Entity");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_MARKER_COLOR("Marker Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_FRIENDLY_COLOR("Friendly Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_NEUTRAL_COLOR("Neutral Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_OPPOSING_COLOR("Opposing Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_OTHER_COLOR("Other Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_ICON_IMAGE("Icon Image");

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

         //we made a virtual function to create our dead reckoning helper so the helper
         //could be changed by a subclass of entity.... bga
         marker->InitDeadReckoningHelper();
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
            dtDAL::MakeFunctor(*pm, &PositionMarker::SetSourceForce),
            dtDAL::MakeFunctorRet(*pm, &PositionMarker::GetSourceForce),
            PROPERTY_SOURCE_FORCE_DESC, POSITION_MARKER_GROUP));

         static const dtUtil::RefString PROPERTY_SOURCE_SERVICE_DESC("The service of the entity that reported this.");
         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>(
            PROPERTY_SOURCE_SERVICE, PROPERTY_SOURCE_SERVICE,
            dtDAL::MakeFunctor(*pm, &PositionMarker::SetSourceService),
            dtDAL::MakeFunctorRet(*pm, &PositionMarker::GetSourceService),
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
            dtDAL::MakeFunctor(*pm, &PositionMarker::SetReportTime),
            dtDAL::MakeFunctorRet(*pm, &PositionMarker::GetReportTime),
            PROPERTY_REPORT_TIME_DESC, POSITION_MARKER_GROUP));

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
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetColor),
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

         static const dtUtil::RefString PROPERTY_ICON_IMAGE_DESC("This image represents the rough "
                  "type of the entity marked by this position.");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TEXTURE,
                  PROPERTY_ICON_IMAGE, PROPERTY_ICON_IMAGE,
                  dtDAL::ResourceActorProperty::SetFuncType(pm, &PositionMarker::LoadImage),
                  PROPERTY_ICON_IMAGE_DESC, POSITION_MARKER_GROUP
         ));
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
