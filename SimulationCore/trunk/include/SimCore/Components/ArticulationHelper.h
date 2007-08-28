/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#ifndef _ARTICULATION_HELPER_H_
#define _ARTICULATION_HELPER_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>

#include <dtCore/refptr.h>

#include <dtUtil/enumeration.h>

#include <dtGame/messageparameter.h>

#include <osg/Referenced>
#include <osg/Vec3>

////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class NodeCollector;
}

namespace dtDAL
{
   class NamedGroupParameter;
}

namespace dtGame
{
   class DeadReckoningHelper;
}

namespace osgSim
{
   class DOFTransform;
}

namespace SimCore
{
   namespace Actors
   {
      class ControlStateActor;
   }

   namespace Components
   {

      //////////////////////////////////////////////////////////////////////////
      // ENUMERATION CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ArticulationMetricType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(ArticulationMetricType);

         public:
            static ArticulationMetricType ARTICULATE_UNKNOWN;
            static ArticulationMetricType ARTICULATE_POSITION;
            static ArticulationMetricType ARTICULATE_EXTENSION;
            static ArticulationMetricType ARTICULATE_X;
            static ArticulationMetricType ARTICULATE_Y;
            static ArticulationMetricType ARTICULATE_Z;
            static ArticulationMetricType ARTICULATE_AZIMUTH;
            static ArticulationMetricType ARTICULATE_ELEVATION;
            static ArticulationMetricType ARTICULATE_ROTATION;

            const std::string& GetRelatedRateMetricName() const { return mRelatedRateName; }

         private:
            ArticulationMetricType( const std::string &name, const std::string& relatedRateName );

            std::string mRelatedRateName;
      };



      //////////////////////////////////////////////////////////////////////////
      // ARTICULATION HELPER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ArticulationHelper : public osg::Referenced
      {
         public:
            static const std::string PROPERTY_NAME_ARTICULATED_ARRAY;
            static const std::string PARAM_NAME_SUFFIX_RATE;
            static const std::string PARAM_NAME_PREFIX_ARTICULATED;
            static const std::string PARAM_NAME_PREFIX_ATTACHED;
            static const std::string PARAM_NAME_CHANGE;
            static const std::string PARAM_NAME_DOF;
            static const std::string PARAM_NAME_DOF_PARENT;
            static const std::string PARAM_NAME_DOF_ROOT;

            ArticulationHelper();

            void SetDirty( bool dirty )
            {
               mIsDirty = dirty;
            }

            /**
             * Determine if this helper recommends sending an update about articulations.
             */
            bool IsDirty() const
            {
               return mIsDirty;
            }

            virtual void SetControlState( ControlStateActor* controlState ) {}
            virtual const ControlStateActor* GetControlState() const { return NULL; }

            /**
             * Set the name of the articulation array property as found on the
             * associated entity. This will be used as the name for the group
             * property that is generated when BuildGroupProperty is called.
             */
            void SetArticulationArrayPropertyName( const std::string& propertyName )
            {
               mArticArrayPropName = propertyName;
            }

            /**
             * Get the name that will be assigned to the group property returned
             * by BuildGroupProperty.
             */
            const std::string& GetArticulationArrayPropertyName() const
            {
               return mArticArrayPropName;
            }

            /**
             * Create a group parameter that represents the articulation
             * hierarchy referenced by this object.
             * @return group parameter representing the articulation hierarchy.
             *         The returned group property is intended to be used by update messages.
             */
            virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> BuildGroupProperty() = 0;

            /**
             * Capture references to specific DOFs in a model.
             * @param nodeCollector The node visitor used for traversing a model,
             *        looking for DOFs of interest.
             *        Specify NULL to dereference all DOFs.
             *
             * NOTE: This function is intended to be called when an entity's model changes.
             *       The model's DOFs are recommended to have the same names across
             *       different versions of the model.
             */
            virtual void UpdateDOFReferences( dtCore::NodeCollector* nodeCollector ) = 0;

            /**
             * Determine if the specified DOF is referenced by this object.
             * Sub-classes should compare the DOF against their own DOFs to
             * make the determination.
             * @param dof The DOF in question; ie. "Is this your dog?"
             * @return TRUE if the DOF is referenced by this object.
             */
            virtual bool HasDOF( osgSim::DOFTransform& dof ) const = 0;

            /**
             * Return all the metric types supported by this helper for a particular DOF.
             * @param dof DOF with metrics in question
             * @param metricName Name of the metric in question.
             *        The name must be one of those specified in ArticulationMetricTypes.
             * @return TRUE if the DOF is referenced by this helper AND if this
             *         helper claims control of the DOF's specified metric.
             */
            virtual bool HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const = 0;

            /**
             * Slot function for capturing change information about a DOF.
             * @param dof The DOF that was changed.
             * @param transChange The amount of positional change applied to the DOF
             * @param hprChange The degrees of rotational change applied to the DOF
             * @param hpdCurrent The absolute orientation in degrees.
             *
             * NOTE: This function calls the subsequent functions
             *       HandleUpdatedDOFTranslation and HandleUpdatedDOFOrientation.
             *
             *       Overriding this function is recommended.
             */
            virtual void HandleUpdatedDOF( const osgSim::DOFTransform& dof,
               const osg::Vec3& transChange, const osg::Vec3& hprChange, const osg::Vec3& hprCurrent );

            /**
             * Receive positional change information about a DOF.
             * @param dof The DOF that was changed.
             * @param transChange The amount of positional change applied to the DOF
             *
             * NOTE: Override this function if the helper only operates on translations.
             */
            virtual void HandleUpdatedDOFTranslation( const osgSim::DOFTransform& dof, const osg::Vec3& transChange ) {}

            /**
             * Receive rotational change information about a DOF.
             * @param dof The DOF that was changed.
             * @param hprChange The degrees of rotational change applied to the DOF
             * @param hpdCurrent The absolute orientation in degrees.
             *
             * NOTE: Override this function if the helper only operates on orientations.
             */
            virtual void HandleUpdatedDOFOrientation( const osgSim::DOFTransform& dof, 
               const osg::Vec3& hprChange, const osg::Vec3& hprCurrent ) {}

            /**
             * This function is intended to process incoming network messages
             * about update to a remote entity's articulations. The entity that
             * contains this helper will pass its node collector and dead reckoing
             * helper into this function so that its articulations can be physically
             * modified according to the values in the articArrayParam.
             * @param articArrayParam The updated values for a specific entity's articulations.
             * @param nodeCollector The object that will expose the entity's DOFs.
             * @param deadReckoningHelper The object that is responsible for dead reckoning
             *        and the physical movement of the entity's DOFs.
             */
            void HandleArticulatedParametersArray( const dtDAL::NamedGroupParameter& articArrayParam,
               dtCore::NodeCollector& nodeCollector, dtGame::DeadReckoningHelper& deadReckoningHelper );

            /**
             * Modify articulations of an entity by receiving data from the
             * network in the form of a control state. This is used in situations
             * where a remote simulation needs to take control of this application's
             * local vehicle simulator entity.
             * @param controlState Data object that should have related articulation updates
             *
             * NOTE: Override this function to implement specific behaviors between
             *       articulations and control states.
             *
             *       This function should physically change the associated DOFs
             *       of the vehicle and then call the appropriate version of the
             *       HandleUpdatedDOF function.
             */
            virtual void HandleUpdatedControlState( const SimCore::Actors::ControlStateActor& controlState ) {}

            /**
             * If this articulation helper references a control state, make it
             * send an update over the network so that it can update articulations
             * on a remote simulation.
             */
            virtual void NotifyControlStateUpdate() {}

         protected:
            virtual ~ArticulationHelper();

            /**
             * Utility function for adding an articulation parameter to a
             * provided group actor property that represents the articulations array.
             * @param outArticArrayProp Actor property that will represent the articulation array
             * @param articulationType The metric type of the articulated parameter
             * @param paramName Name of the new articulated parameter.
             * @param value Current value of the articulation.
             * @param valueChangeCount The running change count of the value.
             * @param rate Current rate of the articulation.
             * @param rateChangeCount The running change count of the rate.
             * @param dofName Name of the DOF that the articulation is affecting
             * @param dofParentName Name of the DOF that is the parent to the direct DOF, dofName
             */
            bool AddArticulatedParameter(
               dtDAL::NamedGroupParameter& outArticArrayProp,
               ArticulationMetricType& articulationType, 
               const std::string& paramName, 
               float value, unsigned valueChangeCount,
               float rate, unsigned rateChangeCount,
               const std::string& dofName, const std::string& dofParentName = PARAM_NAME_DOF_ROOT );

            /**
             * Utility function for pulling out the value contained a metric sub-parameter 
             * in an articulated parameter.
             * @param articParam Group parameter that represents a single articulation
             *        from an articulation array parameter.
             * @param metricTypeName Metric type the articParam may contain.
             * @param outValue Reference to a variable that captures metric value.
             * @return TRUE if the metric type was found and that a value was retrieved.
             */
            bool GetArticulation(const dtGame::GroupMessageParameter& articParam, const std::string &metricTypeName, float &outValue);

            /**
             * Utility function for pulling out the DOF name referenced by an articulated parameter.
             * @param articParam Group parameter that represents a single articulation
             *        from an articulation array parameter.
             * @param outName Reference to a variable that captures name of the DOF
             *        referenced by articParam.
             * @return TRUE if the DOF name was found and retrieved.
             */
            bool GetArticulationDOFName(const dtGame::GroupMessageParameter& articParam, std::string &outName);

         private:
            bool mIsDirty;
            std::string mArticArrayPropName;
      };
   }
}
#endif
