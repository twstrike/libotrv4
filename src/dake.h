#include "data_types.h"
#include "user_profile.h"

typedef struct {
  short protocol_version;
  uint8_t message_type;
  int sender_instance_tag;
  int receiver_instance_tag;
  user_profile_t *sender_profile;
  ed448_point_t *Y;
  uint8_t B[80];
} dake_pre_key_t;

dake_pre_key_t *
dake_pre_key_new();

void
dake_pre_key_free(dake_pre_key_t *pre_key);

dake_pre_key_t *
dake_compute_pre_key();
