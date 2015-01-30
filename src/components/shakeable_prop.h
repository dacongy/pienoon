// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COMPONENTS_SHAKEABLE_PROP_H_
#define COMPONENTS_SHAKEABLE_PROP_H_

#include <vector>
#include <memory>
#include "common.h"
#include "config_generated.h"
#include "entity/component.h"
#include "impel_flatbuffers.h"
#include "impel_init.h"
#include "impel_util.h"

namespace fpl {
namespace pie_noon {

struct ShakeablePropData {
  ShakeablePropData() {}
  float shake_scale;
  mathfu::vec3_packed shake_axis;
  impel::Impeller1f impeller;
};


class ShakeablePropComponent : public entity::Component<ShakeablePropData> {
 public:
  virtual void UpdateAllEntities(entity::WorldTime delta_time);
  virtual void AddFromRawData(entity::EntityRef& entity,
                              const void* data);
  virtual void InitEntity(entity::EntityRef& entity);
  virtual void CleanupEntity(entity::EntityRef& entity);

  void set_config(const Config* config) { config_ = config; }
  void set_impel_engine(impel::ImpelEngine* impel_engine) {
    impel_engine_ = impel_engine;
  }
  void LoadImpellerSpecs();
  void ShakeProps(float damage_percent, const mathfu::vec3& damage_position);

 private:
  const Config* config_;
  impel::ImpelEngine* impel_engine_;
  impel::OvershootImpelInit impeller_inits[ImpellerSpecification_Count];

};

}  // pie_noon
}  // fpl
#endif // COMPONENTS_SHAKEABLE_PROP_H_