/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_sponge_lock

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>

#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct sponge_lock_continue_item {
    uint16_t page;
    uint32_t id;
    uint8_t implicit_modifiers;
};

struct behavior_sponge_lock_config {
    zmk_mod_flags_t mods;
    uint8_t index;
    uint8_t continuations_count;
    struct sponge_lock_continue_item continuations[];
};

struct behavior_sponge_lock_data {
    bool active;
    bool skip;
    uint8_t rand_state;
    //uint8_t rand_flag;
};

static void activate_sponge_lock(const struct device *dev) {
    struct behavior_sponge_lock_data *data = dev->data;

    data->active = true;
    data->skip = false;
}

static void deactivate_sponge_lock(const struct device *dev) {
    struct behavior_sponge_lock_data *data = dev->data;

    data->active = false;
    data->skip = true;
}

static int on_sponge_lock_binding_pressed(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_sponge_lock_data *data = dev->data;

    if (data->active) {
        deactivate_sponge_lock(dev);
    } else {
        activate_sponge_lock(dev);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_sponge_lock_binding_released(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_sponge_lock_driver_api = {
    .binding_pressed = on_sponge_lock_binding_pressed,
    .binding_released = on_sponge_lock_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

static int sponge_lock_keycode_state_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(behavior_sponge_lock, sponge_lock_keycode_state_changed_listener);
ZMK_SUBSCRIPTION(behavior_sponge_lock, zmk_keycode_state_changed);

static const struct device *devs[DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT)];

static bool sponge_lock_is_caps_includelist(const struct behavior_sponge_lock_config *config,
                                          uint16_t usage_page, uint8_t usage_id,
                                          uint8_t implicit_modifiers) {
    for (int i = 0; i < config->continuations_count; i++) {
        const struct sponge_lock_continue_item *continuation = &config->continuations[i];
        LOG_DBG("Comparing with 0x%02X - 0x%02X (with implicit mods: 0x%02X)", continuation->page,
                continuation->id, continuation->implicit_modifiers);

        if (continuation->page == usage_page && continuation->id == usage_id &&
            (continuation->implicit_modifiers &
             (implicit_modifiers | zmk_hid_get_explicit_mods())) ==
                continuation->implicit_modifiers) {
            LOG_DBG("Continuing spongelock, found included usage: 0x%02X - 0x%02X", usage_page,
                    usage_id);
            return true;
        }
    }

    return false;
}

static bool sponge_lock_is_alpha(uint8_t usage_id) {
    return (usage_id >= HID_USAGE_KEY_KEYBOARD_A && usage_id <= HID_USAGE_KEY_KEYBOARD_Z);
}

static bool sponge_lock_is_numeric(uint8_t usage_id) {
    return (usage_id >= HID_USAGE_KEY_KEYBOARD_1_AND_EXCLAMATION &&
            usage_id <= HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS);
}

static void sponge_lock_enhance_usage(const struct behavior_sponge_lock_config *config,
                                    struct zmk_keycode_state_changed *ev) {
    if (ev->usage_page != HID_USAGE_KEY || !sponge_lock_is_alpha(ev->keycode)) {
        return;
    }

    LOG_DBG("Enhancing usage 0x%02X with modifiers: 0x%02X", ev->keycode, config->mods);
    ev->implicit_modifiers |= config->mods;
}

static int sponge_lock_keycode_state_changed_listener(const zmk_event_t *eh) {
    struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev == NULL || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    for (int i = 0; i < DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT); i++) {
        const struct device *dev = devs[i];
        if (dev == NULL) {
            continue;
        }

        struct behavior_sponge_lock_data *data = dev->data;
        if (!data->active) {
            continue;
        }

        const struct behavior_sponge_lock_config *config = dev->config;

        //data->rand_flag = ((data->rand_state >> 0) ^ (data->rand_state >> 2) ^ (data->rand_state >> 5)) & 3;
        //data->rand_state = (data->rand_state >> 1) | (data->rand_flag << 7);
        //if (data->rand_flag < 3) {
          if (!data->skip) {
            data->skip = 1;
          } else {
            data->skip = 0;
          }
        //}
        if (!data->skip) {
          sponge_lock_enhance_usage(config, ev);
        }

        if (!sponge_lock_is_alpha(ev->keycode) && !sponge_lock_is_numeric(ev->keycode) &&
            !sponge_lock_is_caps_includelist(config, ev->usage_page, ev->keycode,
                                           ev->implicit_modifiers)) {
            LOG_DBG("Deactivating sponge_lock for 0x%02X - 0x%02X", ev->usage_page, ev->keycode);
            deactivate_sponge_lock(dev);
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

static int behavior_sponge_lock_init(const struct device *dev) {
    const struct behavior_sponge_lock_config *config = dev->config;
    devs[config->index] = dev;
    return 0;
}

#define SPONGE_LOCK_LABEL(i, _n) DT_INST_LABEL(i)

#define PARSE_BREAK(i)                                                                             \
    {                                                                                              \
        .page = ZMK_HID_USAGE_PAGE(i), .id = ZMK_HID_USAGE_ID(i),                                  \
        .implicit_modifiers = SELECT_MODS(i)                                                       \
    }

#define BREAK_ITEM(i, n) PARSE_BREAK(DT_INST_PROP_BY_IDX(n, continue_list, i))

#define KP_INST(n)                                                                                 \
    static struct behavior_sponge_lock_data behavior_sponge_lock_data_##n = {.active = false, .rand_state = 148};         \
    static struct behavior_sponge_lock_config behavior_sponge_lock_config_##n = {                      \
        .index = n,                                                                                \
        .mods = DT_INST_PROP_OR(n, mods, MOD_LSFT),                                                \
        .continuations = {LISTIFY(DT_INST_PROP_LEN(n, continue_list), BREAK_ITEM, (, ), n)},       \
        .continuations_count = DT_INST_PROP_LEN(n, continue_list),                                 \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_sponge_lock_init, NULL, &behavior_sponge_lock_data_##n,        \
                            &behavior_sponge_lock_config_##n, POST_KERNEL,                           \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_sponge_lock_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)

#endif
