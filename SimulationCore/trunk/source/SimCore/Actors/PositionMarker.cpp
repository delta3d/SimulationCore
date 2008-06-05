#include <SimCore/Actors/PositionMarker.h>
#include <dtDAL/enginepropertytypes.h>

namespace SimCore
{
   namespace Actors
   {

      ////////////////////////////////////////////////////////////////////////
      PositionMarker::PositionMarker(dtGame::GameActorProxy& proxy): 
         BaseClass(proxy),
         mReportTime(0.0),
         mAssociatedEntity(NULL),
         mSourceForce(&BaseEntityActorProxy::ForceEnum::OTHER),
         mSourceService(&BaseEntityActorProxy::ServiceEnum::OTHER)
      {
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
      ////////////////////////////////////////////////////////////////////////
      /////////////    Actor Proxy    ////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////

      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SOURCE_CALLSIGN("Source Callsign");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SOURCE_FORCE("Source Force");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_SOURCE_SERVICE("Source Service");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_REPORT_TIME("Report Time");
      const dtUtil::RefString PositionMarkerActorProxy::PROPERTY_ASSOCIATED_ENTITY("Associated Real Entity");

      ////////////////////////////////////////////////////////////////////////
      PositionMarkerActorProxy::PositionMarkerActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////
      PositionMarkerActorProxy::~PositionMarkerActorProxy()
      {
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
            dtDAL::ActorActorProperty::GetFuncType(this, &PositionMarkerActorProxy::GetAssociatedEntity),
            PROPERTY_ASSOCIATED_ENTITY_DESC, POSITION_MARKER_GROUP));
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
