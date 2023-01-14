#ifndef POLYMER_GAMESTATE_H_
#define POLYMER_GAMESTATE_H_

#include <polymer/asset/asset_system.h>
#include <polymer/camera.h>
#include <polymer/connection.h>
#include <polymer/render/block_mesher.h>
#include <polymer/render/chunk_renderer.h>
#include <polymer/render/font_renderer.h>
#include <polymer/types.h>
#include <polymer/world/block.h>
#include <polymer/world/dimension.h>
#include <polymer/world/world.h>

namespace polymer {

struct MemoryArena;

// TODO: Make this more advanced
struct InputState {
  bool forward;
  bool backward;
  bool left;
  bool right;
  bool climb;
  bool fall;
  bool sprint;
  bool display_players;
};

struct Player {
  char name[17];
  char uuid[16];

  u8 ping;
  u8 gamemode;
  bool listed;
};

struct PlayerManager {
  Player players[256];
  size_t player_count = 0;

  Player* client_player = nullptr;
  char client_name[16];

  void SetClientPlayer(Player* player) {
    client_player = player;
  }

  void AddPlayer(const String& name, const String& uuid, u8 ping, u8 gamemode);
  void RemovePlayer(const String& uuid);
  Player* GetPlayerByUuid(const String& uuid);

  void RenderPlayerList(render::FontRenderer& font_renderer);
};

// Temporary chat message display until a chat window is implemented.
// TODO: Remove
struct ChatMessagePopup {
  char message[1024];
  size_t message_size;
  float remaining_time;
};
struct ChatManager {
  ChatMessagePopup chat_message_queue[5];
  size_t chat_message_index;

  ChatManager() {
    for (size_t i = 0; i < polymer_array_count(chat_message_queue); ++i) {
      chat_message_queue[i].remaining_time = 0.0f;
    }
    chat_message_index = 0;
  }

  void Update(render::FontRenderer& font_renderer, float dt);
  void PushMessage(const char* mesg, size_t mesg_size, float display_time);
};

struct GameState {
  MemoryArena* perm_arena;
  MemoryArena* trans_arena;

  render::VulkanRenderer* renderer;
  render::FontRenderer font_renderer;
  render::ChunkRenderer chunk_renderer;

  render::RenderPass render_pass;
  // TODO: Clean this up
  VkCommandBuffer command_buffers[2];

  asset::AssetSystem assets;
  world::DimensionCodec dimension_codec;
  world::DimensionType dimension;

  Connection connection;
  Camera camera;
  world::World world;

  PlayerManager player_manager;
  ChatManager chat_manager;

  float position_sync_timer;
  float animation_accumulator;
  float time_accumulator;

  u32 world_tick;

  render::ChunkBuildQueue build_queue;
  render::BlockMesher block_mesher;

  world::BlockRegistry block_registry;

  GameState(render::VulkanRenderer* renderer, MemoryArena* perm_arena, MemoryArena* trans_arena);

  void OnBlockChange(s32 x, s32 y, s32 z, u32 new_bid);
  void OnChunkLoad(s32 chunk_x, s32 chunk_z);
  void OnChunkUnload(s32 chunk_x, s32 chunk_z);
  void OnPlayerPositionAndLook(const Vector3f& position, float yaw, float pitch);
  void OnDimensionChange();

  void OnWindowMouseMove(s32 dx, s32 dy);

  void BuildChunkMesh(render::ChunkBuildContext* ctx);
  void BuildChunkMesh(render::ChunkBuildContext* ctx, s32 chunk_x, s32 chunk_y, s32 chunk_z);

  void ImmediateRebuild(render::ChunkBuildContext* ctx, s32 chunk_y);

  void Update(float dt, InputState* input);
  void ProcessMovement(float dt, InputState* input);

  void SubmitFrame();

  void ProcessBuildQueue();

  void FreeMeshes();

  inline float GetCelestialAngle() {
    float result = (((s32)world_tick - 6000) % 24000) / 24000.0f;

    if (result < 0.0f) result += 1.0f;
    if (result > 1.0f) result -= 1.0f;

    return result;
  }

  inline float GetSunlight() {
    float angle = GetCelestialAngle();
    float sunlight = 1.0f - (cosf(angle * 3.1415f * 2.0f) * 2.0f + 1.0f);

    sunlight = 1.0f - Clamp(sunlight, 0.0f, 1.0f);

    return sunlight * 0.8f + 0.2f;
  }
};

} // namespace polymer

#endif
