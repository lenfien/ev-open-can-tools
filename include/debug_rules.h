#pragma once

#include "can_frame_types.h"
#include "drivers/can_driver.h"

#include <list>

#ifndef NATIVE_BUILD
#include <Arduino.h>
#include <SPIFFS.h>
#endif

#define DBG_RULES_MAX 32
#define DBG_LOG_MAX   16
#define DBG_RULES_FILE "/dbg_rules.txt"

struct DbgRule
{
    uint32_t canId = 0;
    int8_t mux = -1;    // -1 = match any mux (0-15)
    int8_t bit = 0;     // 0-63
    uint8_t bitVal = 0; // 0 or 1
    bool enabled = true;
    char name[24] = {};  // 用户自定义名称
};

// Per-(canId, actualMux) record of the last frame we sent
struct DbgLogEntry
{
    uint32_t canId = 0;
    int8_t mux = 0;
    uint8_t data[8] = {};
};

static bool dbgActive = false;

static std::list<DbgLogEntry> g_dbg_log_list;
static std::list<DbgRule> g_dbg_rule_list;

// ── Frame processing ──────────────────────────────────────────────

static bool dbgProcessFrame(CanFrame &frame, CanDriver &driver)
{
    bool should_send = false;
    if (!dbgActive || g_dbg_log_list.empty())
        return should_send;

    DbgRule *dbg_rule_ptr = nullptr;
    DbgRule *dbg_rule_for_log_ptr = nullptr;

    for (auto& r : g_dbg_rule_list)
    {
        if (r.canId != frame.id)
            continue;

        if (r.mux >= 0 && (frame.data[0] & 0x07) != (uint8_t)r.mux)
            continue;

        dbg_rule_for_log_ptr = &r;

        if (!r.enabled)
            continue;

        dbg_rule_ptr = &r;
    }

    if (dbg_rule_ptr != nullptr)
    {
        uint8_t byte_index = (uint8_t)dbg_rule_ptr->bit / 8;
        uint8_t bit_index = (uint8_t)dbg_rule_ptr->bit % 8;
        if (byte_index < 8)
        {
            if (dbg_rule_ptr->bitVal)
                frame.data[byte_index] |= (1u << bit_index);
            else
                frame.data[byte_index] &= ~(1u << bit_index);
        }

        should_send = true;
    }

    // 添加日志
    if (dbg_rule_for_log_ptr != nullptr)
    {
        // Record last sent frame per (canId, actualMux)
        int8_t mux_real = (int8_t)(frame.data[0] & 0x07);
        DbgLogEntry *log_entry_ptr = nullptr;

        for (auto& one : g_dbg_log_list)
        {
            if (one.canId == frame.id && one.mux == mux_real)
            {
                log_entry_ptr = &one;
                break;
            }
        }

        if (log_entry_ptr == nullptr)
        {
            g_dbg_log_list.emplace_back();
            log_entry_ptr = &g_dbg_log_list.back();
            log_entry_ptr->canId = frame.id;
            log_entry_ptr->mux = mux_real;
        }

        for (uint8_t b = 0; b < 8; b++)
            log_entry_ptr->data[b] = frame.data[b];
    }

    // 删除不需要的日志
    for (auto it = g_dbg_log_list.begin(); it != g_dbg_log_list.end(); )
    {
        bool should_care = false;
        for (auto& r : g_dbg_rule_list)
        {
            if (r.canId == it->canId && (r.mux < 0 || r.mux == it->mux))
            {
                should_care = true;
                break;
            }
        }

        if (should_care)
        {
            ++it;
            continue;
        }

        it = g_dbg_log_list.erase(it);
    }

    return should_send;
}

// ── SPIFFS persistence ────────────────────────────────────────────

static void dbgSaveRules()
{
    File f = SPIFFS.open(DBG_RULES_FILE, "w");
    if (!f)
        return;

    for (auto& r : g_dbg_rule_list)
    {
        // 格式：canId mux bit val en name（name 中空格替换为 \x01 以免分割出错）
        String safeName = String(r.name);
        safeName.replace(" ", "\x01");
        if (safeName.length() == 0) safeName = "-";
        f.printf("%lx %d %u %u %d %s\n",
                 (unsigned long)r.canId, (int)r.mux,
                 (unsigned)r.bit, (unsigned)r.bitVal,
                 r.enabled ? 1 : 0,
                 safeName.c_str());
    }

    f.close();
}

static void dbgLoadRules()
{
    if (!SPIFFS.exists(DBG_RULES_FILE))
        return;

    File f = SPIFFS.open(DBG_RULES_FILE, "r");
    if (!f)
        return;

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;
        unsigned long id;
        int mux, bit, val, en;
        char nameBuf[32] = {};
        int parsed = sscanf(line.c_str(), "%lx %d %d %d %d %31s", &id, &mux, &bit, &val, &en, nameBuf);
        if (parsed >= 5)
        {
            g_dbg_rule_list.emplace_back();
            DbgRule &r = g_dbg_rule_list.back();
            r.canId = (uint32_t)id;
            r.mux = (int8_t)mux;
            r.bit = (int8_t)bit;
            r.bitVal = (uint8_t)(val & 1);
            r.enabled = (en != 0);

            if (parsed >= 6 && strcmp(nameBuf, "-") != 0)
            {
                // 还原空格
                for (char *p = nameBuf; *p; p++)
                {
                    if (*p == '\x01')
                        *p = ' ';
                    strncpy(r.name, nameBuf, sizeof(r.name) - 1);
                }
            }
        }
    }

    f.close();
}

// ── JSON helpers ─────────────────────────────────────────────────

static String dbgRulesToJson()
{
    String j = "{\"active\":";
    j += dbgActive ? "true" : "false";
    j += ",\"rules\":[";

    bool should_comma = false;
    for (auto& r: g_dbg_rule_list)
    {
        if (should_comma)
            j += ",";

        should_comma = true;
        j += "{\"id\":" + String(r.canId);
        j += ",\"mux\":" + String((int)r.mux);
        j += ",\"bit\":" + String((int)r.bit);
        j += ",\"val\":" + String((unsigned)r.bitVal);
        j += ",\"en\":" + String(r.enabled ? 1 : 0);

        // name 字段：转义双引号和反斜杠
        j += ",\"name\":\"";
        for (const char *p = r.name; *p; p++)
        {
            if (*p == '"') j += "\\\"";
            else if (*p == '\\') j += "\\\\";
            else j += *p;
        }
        j += "\"";
        j += "}";
    }
    j += "]}";
    return j;
}

static String dbgLogToJson()
{
    String j = "[";
    bool first = true;

    for (auto& l : g_dbg_log_list)
    {
        if (!first) j += ",";
        first = false;
        j += "{\"id\":" + String(l.canId);
        j += ",\"mux\":" + String((int)l.mux);
        j += ",\"data\":[";
        for (uint8_t b = 0; b < 8; b++)
        {
            if (b) j += ",";
            j += String((unsigned)l.data[b]);
        }
        j += "]}";
    }
    j += "]";
    return j;
}

// Parse JSON array: [{"id":N,"mux":N,"bit":N,"val":N,"en":N}, ...]
static bool dbgParseRulesJson(const String &body)
{
    uint8_t count = 0;
    std::list<DbgRule> temp_rules_list;

    int pos = 0;
    int len = body.length();
    while (pos < len)
    {
        while (pos < len && body[pos] != '{')
            pos++;
        if (pos >= len)
            break;
        int end = body.indexOf('}', pos);
        if (end < 0)
            break;
        String obj = body.substring(pos + 1, end);
        pos = end + 1;

        auto getField = [&](const char *key) -> long {
            String k = String("\"") + key + "\":";
            int kp = obj.indexOf(k);
            if (kp < 0) return LONG_MIN;
            int vp = kp + k.length();
            while (vp < (int)obj.length() && obj[vp] == ' ') vp++;
            return obj.substring(vp).toInt();
        };

        long id  = getField("id");
        long mux = getField("mux");
        long bit = getField("bit");
        long val = getField("val");
        long en  = getField("en");

        if (id == LONG_MIN || mux == LONG_MIN || bit == LONG_MIN || val == LONG_MIN || en == LONG_MIN)
            continue;

        if (id < 1 || id > 0x7FF) continue;
        if (mux < -1 || mux > 15) continue;
        if (bit < -1 || bit > 63) continue;
        if (val < 0 || val > 1)   continue;

        temp_rules_list.emplace_back();
        auto& temp = temp_rules_list.back();
        temp.canId   = (uint32_t)id;
        temp.mux     = (int8_t)mux;
        temp.bit     = (uint8_t)bit;
        temp.bitVal  = (uint8_t)val;
        temp.enabled = (en != 0);

        // 解析 name 字段
        {
            String k = String("\"") + "name" + "\":";
            int kp = obj.indexOf(k);
            if (kp >= 0)
            {
                int vp = kp + k.length();
                while (vp < (int)obj.length() && obj[vp] == ' ') vp++;
                if (vp < (int)obj.length() && obj[vp] == '"')
                {
                    vp++;
                    String nm;
                    while (vp < (int)obj.length() && obj[vp] != '"')
                    {
                        if (obj[vp] == '\\' && vp + 1 < (int)obj.length()) { vp++; nm += obj[vp]; }
                        else nm += obj[vp];
                        vp++;
                    }

                    nm = nm.substring(0, 23);
                    strncpy(temp.name, nm.c_str(), sizeof(temp.name) - 1);
                }
            }
        }
        count++;
    }

    g_dbg_rule_list = std::move(temp_rules_list);
    return true;
}
