// Created by Stefan Bauwens 2026
#include "resender.h"

bool send_int(uint32_t key, int value) {
  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result == APP_MSG_OK) {
    dict_write_int(out_iter, key, &value, sizeof(int), true);

    // Send this message
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      // Error sending the outbox
      return false;
    } else {
      return true;
    }
  } else {
    // The outbox cannot be used right now
    return false;;
  }
}

void timer_callback_retrying_int(void *data)
{
    KeyIntPair* keyIntPair = (KeyIntPair *)data;
    send_retrying_int(keyIntPair);
}

void send_retrying_int(KeyIntPair * keyIntPair)
{
    bool success = send_int(keyIntPair->key, keyIntPair->value);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending: %d Value: %d Success? %d", keyIntPair->key, keyIntPair->value, success);
    if (!success)
    {
        app_timer_register(100, timer_callback_retrying_int, keyIntPair);
    }
}
