//
// Created by HongBo Zhang on 2026/5/8.
//

#include <Arduino.h>
#include "driver/twai.h"
#include <math.h>

// 硬件：Waveshare ESP32-S3-RS485-CAN
#define CAN_TX_PIN GPIO_NUM_15
#define CAN_RX_PIN GPIO_NUM_16

// ==========================================
// 1. 全局状态变量
// ==========================================
volatile int current_speedProfile = 1;
volatile bool current_ota_status = false;
volatile uint32_t current_bus_off = 0;
volatile bool is_fsd_engaged = false;
volatile char currentGear = 'N';
volatile uint8_t current_smart_offset_pct = 0;
volatile uint8_t current_detected_limit = 0;

// FSD 状态防闪烁心跳计时器
unsigned long last_fsd_active_time = 0;
// 用于监测 FSD 状态切换的上一次状态
bool last_fsd_engaged_state = false;

// ==========================================
// 将力矩状态机变量提升至全局，防止时间冻结
// ==========================================
unsigned long next_pulse_start_time = 0;
unsigned long current_pulse_phase_time = 0;
int pulse_state = 0;
int total_strokes = 2;
int current_stroke_count = 0;
int current_direction = 1;
uint8_t random_amplitude = 45;
uint16_t current_stroke_duration = 150;

// 特斯拉专用 Checksum 算法
uint8_t calculate_tesla_checksum(uint16_t msg_id, uint8_t* data, uint8_t len) {
    uint8_t checksum = (msg_id & 0xFF) + ((msg_id >> 8) & 0xFF);
    for (uint8_t i = 0; i < len - 1; i++) { checksum += data[i]; }
    return checksum;
}

inline void setBit(twai_message_t &msg, int bit, bool value) {
    int byteIndex = bit / 8;
    int bitIndex = bit % 8;
    if (value) msg.data[byteIndex] |= (1U << bitIndex);
    else msg.data[byteIndex] &= ~(1U << bitIndex);
}

// 混沌随机数生成器
uint32_t xorshift_state = 88888888;
uint32_t xorshift32() {
    xorshift_state ^= xorshift_state << 13;
    xorshift_state ^= xorshift_state >> 17;
    xorshift_state ^= xorshift_state << 5;
    return xorshift_state;
}

// ==========================================
// 2. 独立后台监控任务
// ==========================================
void debugPrintTask(void *pvParameters) {
    for (;;) {
        const char* profileStr = "未知";
        switch(current_speedProfile) {
            case 4: profileStr = "狂飙"; break;
            case 3: profileStr = "激进"; break;
            case 2: profileStr = "标准"; break;
            case 1: profileStr = "舒适"; break;
            case 0: profileStr = "佛系"; break;
        }
        Serial.printf("[安全融合版] 档位:%c | FSD:%s | 限速:%d | 偏移:+%d%% | 模式:%s\n",
                      currentGear, (is_fsd_engaged ? "是" : "否"),
                      current_detected_limit, current_smart_offset_pct, profileStr);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ==========================================
// 3. 系统初始化
// ==========================================
void setup() {
    Serial.begin(921600);
    delay(2000);

    xorshift_state += millis();

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 200;
    g_config.tx_queue_len = 20;
    g_config.alerts_enabled = TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK && twai_start() == ESP_OK) {
        Serial.println("--- HW4.0  (V-Chaos Pro 安全修复版) 启动 ---");
        xTaskCreatePinnedToCore(debugPrintTask, "DebugTask", 4096, NULL, 1, NULL, 0);
    }
}

// ==========================================
// 4. 核心 CAN 处理循环
// ==========================================
void loop() {
    uint32_t alerts;
    if (twai_read_alerts(&alerts, 0) == ESP_OK) {
        if (alerts & TWAI_ALERT_BUS_OFF) { current_bus_off++; twai_initiate_recovery(); }
        if (alerts & TWAI_ALERT_BUS_RECOVERED) twai_start();
    }

    twai_message_t msg;
    if (twai_receive(&msg, pdMS_TO_TICKS(1)) == ESP_OK) {

        // 【1】 OTA 保护 (0x318)
        if (msg.identifier == 0x318) { current_ota_status = (msg.data[0] & 0x01); }
        if (current_ota_status) return;

        // 【2】 档位识别 (0x118)
        if (msg.identifier == 0x118) {
            uint8_t gearRaw = (msg.data[2] >> 4) & 0x0F;
            if (gearRaw == 0x3) currentGear = 'P';
            else if (gearRaw == 0x5) currentGear = 'R';
            else if (gearRaw == 0x9) currentGear = 'D';
            else currentGear = 'N';
        }

        // ====================================================
        //  修复：带有宽容度的心跳保活状态机
        // 解决严格掩码锁导致的心跳饿死问题
        // ====================================================
        if (msg.identifier == 0x389) {
            // 严格掩码锁：绝不让脏数据混入
            if (msg.data[0] == 0xFF && (msg.data[1] == 0x03 || msg.data[1] == 0x04)) {
                last_fsd_active_time = millis();
            }
        }

        // 核心判定：HW4.0 的真实 FSD 状态包发送频率很低，将超时放宽至 2000ms！
        // 可以稳定确认FSD开启状态，又能在关闭时迅速（2秒内）强制断开
        if (currentGear == 'D' && (millis() - last_fsd_active_time < 2000)) {
            is_fsd_engaged = true;
        } else {
            is_fsd_engaged = false;
        }

        // ====================================================
        // 🌟 监听 FSD 状态变化，执行全局重置
        // ====================================================
        if (is_fsd_engaged != last_fsd_engaged_state) {

            // 修改检测不准确问题
            if (is_fsd_engaged) {
                // 重置力矩状态机，防止历史时钟引发瞬间抽搐
                pulse_state = 0;
                // 给出 2.5 到 3.5 秒的起步缓冲期，再执行第一次肌肉拉拽
                next_pulse_start_time = millis() + 2500 + (xorshift32() % 1000);
            }

            // 立即重新计算限速偏移
            if (is_fsd_engaged && current_detected_limit > 0) {
                if (current_detected_limit <= 49) current_smart_offset_pct = 50;
                else if (current_detected_limit <= 69) current_smart_offset_pct = 30;
                else if (current_detected_limit <= 100) current_smart_offset_pct = 10;
                else current_smart_offset_pct = 0;
            } else {
                current_smart_offset_pct = 0;
            }

            last_fsd_engaged_state = is_fsd_engaged;
        }

        // ====================================================
        // 【4】 限速数据纯粹抓取 (0x399)
        // ====================================================
        if (msg.identifier == 0x399) {
            uint8_t raw_limit = 0;
            if (msg.data[1] > 0 && msg.data[1] <= 30) raw_limit = msg.data[1];

            if (raw_limit > 0) {
                current_detected_limit = raw_limit * 5;
                // 限速牌变化时，如果 FSD 开着，也同步更新偏移
                if (is_fsd_engaged) {
                    if (current_detected_limit <= 49) current_smart_offset_pct = 50;
                    else if (current_detected_limit <= 69) current_smart_offset_pct = 30;
                    else if (current_detected_limit <= 100) current_smart_offset_pct = 10;
                    else current_smart_offset_pct = 0;
                }
            }
            msg.data[0] &= ~0x10;
            twai_transmit(&msg, pdMS_TO_TICKS(1));
        }

        if (msg.identifier == 0x331) {
            setBit(msg, 12, true);
            twai_transmit(&msg, pdMS_TO_TICKS(1));
        }

        if (msg.identifier == 1016) {
            uint8_t fd = (msg.data[5] >> 5) & 0x07;
            switch(fd) {
                case 2: current_speedProfile = 4; break;
                case 3: current_speedProfile = 3; break;
                case 4: current_speedProfile = 2; break;
                case 5: current_speedProfile = 1; break;
                case 6: current_speedProfile = 0; break;
            }
        }

        // ====================================================
        // 【5】 防封禁架构：动态多态连招模拟 (0x370)
        // ====================================================
        if (msg.identifier == 0x370 && is_fsd_engaged) {
            msg.data[4] = (msg.data[4] & ~0xC0) | 0x40; // 声明手在方向盘上

            unsigned long currentMillis = millis();
            uint8_t final_torque = msg.data[3];

            if (pulse_state == 0) {
                if (currentMillis >= next_pulse_start_time) {
                    pulse_state = 1;
                    current_pulse_phase_time = currentMillis;

                    total_strokes = 1 + (xorshift32() % 3);
                    current_direction = (xorshift32() % 2 == 0) ? 1 : -1;
                    current_stroke_count = 0;

                    random_amplitude = 30 + (xorshift32() % 26);
                    current_stroke_duration = 80 + (xorshift32() % 120);
                } else {
                    final_torque = msg.data[3] + (sin(currentMillis / (300.0 + (xorshift32()%50))) * 1.5);
                }
            }

            if (pulse_state == 1) {
                if (currentMillis - current_pulse_phase_time < current_stroke_duration) {
                    int calc_torque = 0x80 + (current_direction * random_amplitude);
                    if (calc_torque < 0) calc_torque = 0;
                    if (calc_torque > 255) calc_torque = 255;
                    final_torque = calc_torque;
                } else {
                    current_stroke_count++;
                    if (current_stroke_count < total_strokes) {
                        current_direction *= -1;
                        current_pulse_phase_time = currentMillis;
                        random_amplitude = 25 + (xorshift32() % 26);
                        current_stroke_duration = 80 + (xorshift32() % 100);
                    } else {
                        pulse_state = 0;
                        next_pulse_start_time = currentMillis + 3500 + (xorshift32() % 2500);
                    }
                }
            }

            msg.data[3] = final_torque;

            msg.data[6] = (msg.data[6] & 0xF0) | ((msg.data[6] + 1) % 16);
            msg.data[7] = calculate_tesla_checksum(0x370, msg.data, 8);
            twai_transmit(&msg, pdMS_TO_TICKS(2));
        }

        // ====================================================
        // 【6】 FSD 核心配置强制注入 (1021)
        // ====================================================
        if (msg.identifier == 1021) {
            uint8_t index = msg.data[0] & 0x07;
            bool modified = false;

            if (index == 0) {
                setBit(msg, 46, true);
                setBit(msg, 60, true);
                modified = true;
            }
            if (index == 1) {
                setBit(msg, 19, false);
                setBit(msg, 47, true);
                modified = true;
            }
            if (index == 2) {
                msg.data[7] &= ~(0x07 << 4);
                msg.data[7] |= (current_speedProfile & 0x07) << 4;
                msg.data[1] = (msg.data[1] & ~0x3F) | (current_smart_offset_pct & 0x3F);
                modified = true;
            }

            if (modified) { twai_transmit(&msg, pdMS_TO_TICKS(2)); }
        }
    }
}