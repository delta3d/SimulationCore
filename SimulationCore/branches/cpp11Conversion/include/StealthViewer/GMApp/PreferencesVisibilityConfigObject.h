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
 */

#ifndef PREFERENCESVISIBILITYCONFIGOBJECT_H_
#define PREFERENCESVISIBILITYCONFIGOBJECT_H_

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>
#include <SimCore/Components/LabelManager.h>
#include <SimCore/VisibilityOptions.h>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT PreferencesVisibilityConfigObject: public StealthGM::ConfigurationObjectInterface
   {
   public:
      PreferencesVisibilityConfigObject();

      void SetLabelOptions(const SimCore::Components::LabelOptions& options);
      const SimCore::Components::LabelOptions& GetLabelOptions() const;
      SimCore::Components::LabelOptions& GetLabelOptions();

      void SetEntityOptions(SimCore::VisibilityOptions& options);
      const SimCore::VisibilityOptions& GetEntityOptions() const;
      SimCore::VisibilityOptions& GetEntityOptions();

      /// Close the tops on solid Battlefield graphics objects
      DT_DECLARE_ACCESSOR(bool, BFGCloseTops);

      /**
       * Overridden base class method to apply the changes made to this class to the
       * game manager
       */
      virtual void ApplyChanges(dtGame::GameManager& gameManager);

   protected:
      virtual ~PreferencesVisibilityConfigObject();
      SimCore::Components::LabelOptions mLabelOptions;
      std::shared_ptr<SimCore::VisibilityOptions> mEntityOptions;
   };
}

#endif /* PREFERENCESVISIBILITYCONFIGOBJECT_H_ */
