// Copyright 2014 Google Inc. All rights reserved.
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

#include "precompiled.h"
#include <math.h>

#include "character.h"
#include "character_state_machine.h"
#include "character_state_machine_def_generated.h"
#include "impel_flatbuffers.h"
#include "impel_init.h"
#include "impel_util.h"
#include "pie_noon_common_generated.h"
#include "scoring_rules_generated.h"
#include "timeline_generated.h"
#include "utilities.h"

using mathfu::vec2i;
using mathfu::vec2;
using mathfu::vec3;
using mathfu::vec4;
using mathfu::mat4;

namespace fpl {
namespace pie_noon {

Character::Character(
    CharacterId id, Controller* controller, const Config& config,
    const CharacterStateMachineDef* character_state_machine_def)
    : config_(&config),
      id_(id),
      target_(0),
      health_(0),
      pie_damage_(0),
      position_(mathfu::kZeros3f),
      controller_(controller),
      just_joined_game_(false),
      state_machine_(character_state_machine_def),
      victory_state_(kResultUnknown) {
  ResetStats();
}

void Character::Reset(CharacterId target, CharacterHealth health,
                      Angle face_angle, const vec3& position,
                      impel::ImpelEngine* impel_engine) {
  target_ = target;
  health_ = health;
  pie_damage_ = 0;
  position_ = position;
  state_machine_.Reset();
  victory_state_ = kResultUnknown;

  impel::OvershootImpelInit init;
  OvershootInitFromFlatBuffers(*config_->face_angle_def(), &init);

  impel::ImpelTarget1f t;
  t.SetValue(face_angle.ToRadians());
  face_angle_.InitializeWithTarget(init, impel_engine, t);
}

void Character::SetTarget(CharacterId target, Angle angle_to_target) {
  target_ = target;
  impel::ImpelTarget1f t;
  t.SetTargetValue(angle_to_target.ToRadians());
  face_angle_.SetTarget(t);
}

void Character::TwitchFaceAngle(impel::TwitchDirection twitch) {
  impel::Settled1f settled;
  impel::Settled1fFromFlatBuffers(*config_->face_angle_twitch(), &settled);
  const float velocity = config_->face_angle_twitch_velocity();
  impel::Twitch(twitch, velocity, settled, &face_angle_);
}

mat4 Character::CalculateMatrix(bool facing_camera) const {
  const Angle face_angle = FaceAngle();
  return mat4::FromTranslationVector(position_) *
         mat4::FromRotationMatrix(face_angle.ToXZRotationMatrix()) *
         mat4::FromScaleVector(vec3(1.0f, 1.0f, facing_camera ? 1.0f : -1.0f));
}

uint16_t Character::RenderableId(WorldTime anim_time) const {
  // The timeline for the current state has an array of renderable ids.
  const Timeline* timeline = state_machine_.current_state()->timeline();
  if (!timeline || !timeline->renderables())
    return RenderableId_Invalid;

  // Grab the TimelineRenderable for 'anim_time', from the timeline.
  const int renderable_index =
      TimelineIndexBeforeTime(timeline->renderables(), anim_time);
  const TimelineRenderable* renderable =
      timeline->renderables()->Get(renderable_index);
  if (!renderable)
    return RenderableId_Invalid;

  // Return the renderable id for 'anim_time'.
  return renderable->renderable();
}

mathfu::vec4 Character::Color() const {
  const bool ai = controller_->controller_type() == Controller::kTypeAI;
  const vec3 color = ai ? LoadVec3(config_->ai_color()) :
                     Lerp(mathfu::kOnes3f,
                          LoadVec3(config_->character_colors()->Get(id_)),
                          1.0f / config_->character_global_brightness_factor());
  return vec4(color, 1.0);
}

void Character::IncrementStat(PlayerStats stat) {
  player_stats_[stat]++;
}

void Character::ResetStats() {
  for (int i = 0; i < kMaxStats; i++) player_stats_[i] = 0;
}


// orientation_ and position_ are set each frame in GameState::Advance.
AirbornePie::AirbornePie(CharacterId original_source, CharacterId source,
                         CharacterId target, WorldTime start_time,
                         WorldTime flight_time, CharacterHealth original_damage,
                         CharacterHealth damage, float height, int rotations)
    : original_source_(original_source),
      source_(source),
      target_(target),
      start_time_(start_time),
      flight_time_(flight_time),
      original_damage_(original_damage),
      damage_(damage),
      height_(height),
      rotations_(rotations),
      orientation_(0.0f, 0.0f, 1.0f, 0.0f),
      position_(0.0f) {
}

mat4 AirbornePie::CalculateMatrix() const {
  return mat4::FromTranslationVector(position_) *
         mat4::FromRotationMatrix(orientation_.ToMatrix());
}

void ApplyScoringRule(const ScoringRules* scoring_rules,
                      ScoreEvent event,
                      unsigned int damage,
                      Character* character) {
  const auto* rule = scoring_rules->rules()->Get(event);
  switch (rule->reward_type()) {
    case RewardType_None: {
      break;
    }
    case RewardType_AddDamage: {
      character->set_score(character->score() + damage);
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                  "Player %i got %i %s!\n",
                  character->id(),
                  damage,
                  damage == 1 ? "point" : "points");
      break;
    }
    case RewardType_SubtractDamage: {
      character->set_score(character->score() - damage);
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                  "Player %i lost %i %s!\n",
                  character->id(),
                  damage,
                  damage == 1 ? "point" : "points");
      break;
    }
    case RewardType_AddPointValue: {
      character->set_score(character->score() + rule->point_value());
      if (rule->point_value()) {
        int points = rule->point_value();
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Player %i %s %i %s!\n", character->id(),
                    points > 0 ? "got" : "lost",
                    std::abs(points),
                    std::abs(points) == 1? "point" : "points");
      }
      break;
    }
  }

}

} //  namespace fpl
} //  namespace pie_noon
