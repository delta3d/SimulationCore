#include <SimCore/Actors/PositionMarker.h>
#include <dtDAL/enginepropertytypes.h>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

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
      PositionMarker::PositionMarker(dtGame::GameActorProxy& proxy): 
         BaseClass(proxy),
         mReportTime(0.0),
         mAssociatedEntity(NULL),
         mSourceForce(&BaseEntityActorProxy::ForceEnum::OTHER),
         mSourceService(&BaseEntityActorProxy::ServiceEnum::OTHER)
      {
         dtCore::RefPtr<osg::Sphere> sphere = new osg::Sphere(osg::Vec3(0.0, 0.0, 10.0), 5.0);
         dtCore::RefPtr<osg::Box> box = new osg::Box(osg::Vec3(0.0, 0.0, 2.5), 25.0, 15.0, 5.0);
         dtCore::RefPtr<osg::ShapeDrawable> sphereDrawable = new osg::ShapeDrawable(sphere.get());
         dtCore::RefPtr<osg::ShapeDrawable> boxDrawable = new osg::ShapeDrawable(box.get());
         dtCore::RefPtr<osg::Geode> geode = new osg::Geode();

         dtCore::RefPtr<osg::TessellationHints> hints = new osg::TessellationHints;
         hints->setCreateTextureCoords(true);
         sphereDrawable->setTessellationHints(hints.get());

         geode->addDrawable(sphereDrawable.get());
         geode->addDrawable(boxDrawable.get());
         mSphere = sphereDrawable;
         mBox = boxDrawable;
         SetSphereColor(osg::Vec4(0.5, 0.5, 1.0, 0.4));
         SetBoxColor(osg::Vec4(0.5, 1.0, 5.0, 0.4));
         osg::Group* g = GetOSGNode()->asGroup();
         g->addChild(geode.get());

         osg::StateSet* ss = g->getOrCreateStateSet();
         ss->setMode(GL_BLEND,osg::StateAttribute::ON);
         dtCore::RefPtr<osg::BlendFunc> trans = new osg::BlendFunc();
         trans->setFunction( osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
         ss->setAttributeAndModes( trans.get() );
         ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
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
      void PositionMarker::SetSphereColor(const osg::Vec4& vec)
      {
         mSphere->setColor(vec);
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetSphereColor()
      {
         return mSphere->getColor();
      }

      ////////////////////////////////////////////////////////////////////////
      void PositionMarker::SetBoxColor(const osg::Vec4& vec)
      {
         mBox->setColor(vec);
      }

      ////////////////////////////////////////////////////////////////////////
      const osg::Vec4& PositionMarker::GetBoxColor()
      {
         return mBox->getColor();
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
            osg::StateSet* ss = mSphere->getOrCreateStateSet();
            ss->setTextureAttributeAndModes(0, NULL, osg::StateAttribute::OFF);
            return;
         }

         dtCore::RefPtr<osg::Image> icon  = osgDB::readImageFile( theFile );
         if(icon == NULL)
         {
            LOG_ERROR(std::string("Couldn't find image file \"") + theFile + "\"");
            osg::StateSet* ss = mSphere->getOrCreateStateSet();
            ss->setTextureAttributeAndModes(0, NULL, osg::StateAttribute::OFF);
            return;
         }

         dtCore::RefPtr<osg::Texture2D> tex = new osg::Texture2D;
         tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP );
         tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP );
         tex->setImage( icon.get() );

         osg::StateSet* ss = mSphere->getOrCreateStateSet();
         ss->setTextureAttributeAndModes(0, tex.get(), osg::StateAttribute::ON);
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
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SPHERE_COLOR("Sphere Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_BOX_COLOR("Box Color");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_ICON_IMAGE("Icon Image");

      ////////////////////////////////////////////////////////////////////////
      PositionMarkerActorProxy::PositionMarkerActorProxy()
      {
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

         static const dtUtil::RefString PROPERTY_SPHERE_COLOR_DESC("The color of the sphere on the marker.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_SPHERE_COLOR, PROPERTY_SPHERE_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetSphereColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetSphereColor),
                  PROPERTY_SPHERE_COLOR_DESC, POSITION_MARKER_GROUP
         ));

         static const dtUtil::RefString PROPERTY_BOX_COLOR_DESC("The color of the box on the marker.");
         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_BOX_COLOR, PROPERTY_BOX_COLOR,
                  dtDAL::ColorRgbaActorProperty::SetFuncType(pm, &PositionMarker::SetBoxColor),
                  dtDAL::ColorRgbaActorProperty::GetFuncType(pm, &PositionMarker::GetBoxColor),
                  PROPERTY_BOX_COLOR_DESC, POSITION_MARKER_GROUP
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
